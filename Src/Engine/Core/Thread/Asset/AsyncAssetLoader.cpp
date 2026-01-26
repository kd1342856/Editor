#include "AsyncAssetLoader.h"
#include "../ThreadManager.h" // スレッドマネージャ
#include "../Profiler/Profiler.h"
#include "../../../ImGui/Log/Logger.h"

void AsyncAssetLoader::Init()
{
}

void AsyncAssetLoader::Release()
{
	m_completionCallbacks.clear();
	m_textureCache.clear();
	m_modelCache.clear();
}

void AsyncAssetLoader::Update()
{
	// メインスレッドでのコールバック実行
	// (非同期ロードが完了したアセットの差し替えなど)
	std::vector<std::function<void()>> callbacks;

	{
		std::lock_guard<std::mutex> lock(m_callbackMutex);
		if (m_completionCallbacks.empty()) return;
		
		callbacks = std::move(m_completionCallbacks);
		m_completionCallbacks.clear();
	}

	for (auto& func : callbacks)
	{
		if (func) func();
	}
}

// 1x1 白テクスチャの管理
ID3D11ShaderResourceView* AsyncAssetLoader::GetWhiteTex()
{
    static ID3D11ShaderResourceView* s_whiteSRV = nullptr;
    
    if (!s_whiteSRV)
    {
        // 1x1の白テクスチャを作成
        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = 1;
        desc.Height = 1;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        uint32_t whitePixels = 0xFFFFFFFF; // RGBA all 1.0
        D3D11_SUBRESOURCE_DATA data = {};
        data.pSysMem = &whitePixels;
        data.SysMemPitch = 4;

        ID3D11Texture2D* tex = nullptr;
        if (SUCCEEDED(KdDirect3D::Instance().WorkDev()->CreateTexture2D(&desc, &data, &tex)))
        {
            // KdTextureのstatic関数ではなく、直接ビューを作成
            
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = desc.Format;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels = 1;
            
            KdDirect3D::Instance().WorkDev()->CreateShaderResourceView(tex, &srvDesc, &s_whiteSRV);
            
            tex->Release();
        }
    }
    return s_whiteSRV;
}

std::shared_ptr<KdTexture> AsyncAssetLoader::LoadTextureAsync(const std::string& filename, Job::Priority priority)
{
	if (filename.empty()) return nullptr;

	// キャッシュ確認
	auto it = m_textureCache.find(filename);
	if (it != m_textureCache.end())
	{
		auto ptr = it->second.lock();
		if (ptr) return ptr;
	}

	// 新規作成
	std::shared_ptr<KdTexture> newTex = std::make_shared<KdTexture>();

	// キャッシュ登録
	m_textureCache[filename] = newTex;

	// プレースホルダーセット
	ID3D11ShaderResourceView* white = GetWhiteTex();
	if (white)
	{
		newTex->SetSRView(white);
	}

	// パスをコピーしてスレッドに渡す
	std::string pathStr = filename;

	// newTexを weak_ptr で渡す
	std::weak_ptr<KdTexture> weakTex = newTex;

	ThreadManager::Instance().AddJobWithPriority(priority, [this, weakTex, pathStr]()
		{
			PROFILE_SCOPE("LoadTexture: " + pathStr);

			// --- ワーカースレッド内 ---

			// 開発用ログ
			Logger::Log("AsyncLoader", "Loading Texture: " + pathStr);

			// A. ファイル読み込み
			std::wstring wFilename = sjis_to_wide(pathStr);
			DirectX::TexMetadata meta;
			DirectX::ScratchImage image;
			bool bLoaded = false;

			if (SUCCEEDED(DirectX::LoadFromWICFile(wFilename.c_str(), DirectX::WIC_FLAGS_ALL_FRAMES, &meta, image))) bLoaded = true;
			if (!bLoaded && SUCCEEDED(DirectX::LoadFromDDSFile(wFilename.c_str(), DirectX::DDS_FLAGS_NONE, &meta, image))) bLoaded = true;
			if (!bLoaded && SUCCEEDED(DirectX::LoadFromTGAFile(wFilename.c_str(), &meta, image))) bLoaded = true;
			if (!bLoaded && SUCCEEDED(DirectX::LoadFromHDRFile(wFilename.c_str(), &meta, image))) bLoaded = true;

			if (!bLoaded)
			{
				// ログ出力
				Logger::Error("Failed to load texture: " + pathStr);
				return; // 失敗
			}

			// Mipmap
			if (meta.mipLevels == 1)
			{
				DirectX::ScratchImage mipChain;
				if (SUCCEEDED(DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0, mipChain)))
				{
					image = std::move(mipChain);
				}
			}

			// B. リソース作成
			ID3D11Texture2D* newTexRes = nullptr;
			if (FAILED(DirectX::CreateTextureEx(KdDirect3D::Instance().WorkDev(), image.GetImages(), image.GetImageCount(), image.GetMetadata(), D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, DirectX::CREATETEX_FLAGS::CREATETEX_DEFAULT, (ID3D11Resource**)&newTexRes)))
			{
				return;
			}

			// ビュー作成 (SRVのみ)
			ID3D11ShaderResourceView* newSRV = nullptr;

			D3D11_TEXTURE2D_DESC desc;
			newTexRes->GetDesc(&desc);

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = desc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = desc.MipLevels;

			if (FAILED(KdDirect3D::Instance().WorkDev()->CreateShaderResourceView(newTexRes, &srvDesc, &newSRV)))
			{
				newTexRes->Release();
				return;
			}

			// C. メインスレッドに適用依頼
			{
				std::lock_guard<std::mutex> lock(m_callbackMutex);
				m_completionCallbacks.emplace_back([weakTex, newSRV, newTexRes]()
					{
						// メインスレッドで実行
						auto ptr = weakTex.lock();
						if (ptr)
						{
							// 差し替え (KdTexture::SetSRView は Release() も呼んでくれる)
							ptr->SetSRView(newSRV);
						}

						// SetSRViewでAddRefされるので、ここのカウントは下げる
						newSRV->Release();
						newTexRes->Release();
					});
			}
		});

	return newTex;
}

std::shared_ptr<KdModelData> AsyncAssetLoader::LoadModelAsync(const std::string& filename, Job::Priority priority)
{
    if (filename.empty()) return nullptr;

    // キャッシュ確認
    auto it = m_modelCache.find(filename);
    if (it != m_modelCache.end())
    {
        auto ptr = it->second.lock();
        if (ptr) return ptr;
    }

    // 新規作成 (中身は空)
    std::shared_ptr<KdModelData> newModel = std::make_shared<KdModelData>();

    // キャッシュ登録
    m_modelCache[filename] = newModel;

    // パスをコピー
    std::string pathStr = filename;
    std::weak_ptr<KdModelData> weakModel = newModel;

    ThreadManager::Instance().AddJobWithPriority(priority, [this, weakModel, pathStr]()
    {
        PROFILE_SCOPE("LoadModel: " + pathStr);
        // 開発用ログ
        Logger::Log("AsyncLoader", "Loading Model: " + pathStr);

        // ワーカースレッド内でモデルロード
        auto loadedModel = std::make_shared<KdModelData>();
        
        if (loadedModel->Load(pathStr))
        {
            // 成功したらメインスレッドでスワップ
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            m_completionCallbacks.emplace_back([weakModel, loadedModel]()
            {
                auto ptr = weakModel.lock();
                if (ptr)
                {
                    ptr->Swap(*loadedModel);
                }
            });
        }
        else
        {
            #ifdef _DEBUG
            Logger::Error("Failed to load model: " + pathStr);
            #endif
        }

    });

    return newModel;
}
