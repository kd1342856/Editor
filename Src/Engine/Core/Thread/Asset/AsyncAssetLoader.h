#pragma once
#include "../ThreadManager.h"

// 非同期アセットローダー
// ・別スレッドでのアセット読み込みを管理する
class AsyncAssetLoader
{
public:

	// 初期化
	void Init();
	
	// 更新
	void Update();

	// 解放
	void Release();

	// テクスチャの非同期ロード
	std::shared_ptr<KdTexture> LoadTextureAsync(const std::string& filename, Job::Priority priority = Job::Priority::Normal);

	// モデルの非同期ロード
	std::shared_ptr<KdModelData> LoadModelAsync(const std::string& filename, Job::Priority priority = Job::Priority::Normal);

private:
	AsyncAssetLoader() {}
	~AsyncAssetLoader() { Release(); }

	// 白テクスチャ取得 (プレースホルダー用)
	ID3D11ShaderResourceView* GetWhiteTex();

	// テクスチャキャッシュ (ファイルパス -> テクスチャ)
	// weak_ptrにしておき、使われなくなったら自動解放されるようにする
	std::map<std::string, std::weak_ptr<KdTexture>> m_textureCache;
	std::map<std::string, std::weak_ptr<KdModelData>> m_modelCache;
	
	// コールバックリクエスト
	// スレッドからメインスレッドに処理を依頼するためのキュー
	// (テクスチャ差し替えなど)
	std::mutex m_callbackMutex;
	std::vector<std::function<void()>> m_completionCallbacks;
public:
	static AsyncAssetLoader& Instance()
	{
		static AsyncAssetLoader instance;
		return instance;
	}
};
