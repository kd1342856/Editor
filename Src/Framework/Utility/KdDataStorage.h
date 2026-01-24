#pragma once


// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// アセットを取り出し可能な状態で保持するクラス
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// データの読み込み・保持・検索の機能を持っている
// 汎用性のため検索・読込命令は文字列を使用
// メモリ・処理効率を考えデザインパターンのFlyWeightパターンを利用
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
template<class DataType>
class KdDataStorage
{
public:
	KdDataStorage() {}
	~KdDataStorage() { ClearData(true); }

	// カスタムローダーの設定
	void SetCustomLoader(std::function<std::shared_ptr<DataType>(const std::string&)> loader)
	{
		m_customLoader = loader;
	}

	// 各アセットの読込・取得関数
	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// 強制的にデータを読み込ませて更新する
	std::shared_ptr<DataType> LoadData(std::string_view fileName)
	{
		// カスタムローダーがあればそれを使う
		if (m_customLoader)
		{
			std::shared_ptr<DataType> newData = m_customLoader(fileName.data());
			if (newData)
			{
				m_spDatas[fileName.data()] = newData;
				return newData;
			}
			// カスタムローダーがnullを返した場合は通常のロードを試みる、またはエラー
		}

		std::shared_ptr<DataType> newData = std::make_shared<DataType>();

		if (!newData->Load(fileName))
		{
			assert(0 && "KdDataStorage::LoadData ファイルが存在しません。ファイルパスを確認してください");

			return nullptr;
		}

		m_spDatas[fileName.data()] = newData;

		return newData;
	}

	// データの取得：リスト内に存在しない場合は新しくロードする
	std::shared_ptr<DataType> GetData(std::string_view fileName)
	{
		// リストの中に欲しいデータがあるか検索
		auto findData = m_spDatas.find(fileName.data());

		// データがあった場合
		if (findData != m_spDatas.end())
		{
			// そのままデータを共有
			return (*findData).second;
		}
		// データが無かった場合
		else
		{
			// 新たにデータをロードする
			return LoadData(fileName);
		}
	}

	// 保持しているデータの破棄
	void ClearData(bool force)
	{
		if (force)
		{
			// 強制的にすべてのデータを消去
			m_spDatas.clear();

			return;
		}

		// アプリ上で使用されておらず、Storageクラスが保持しているだけのデータを破棄
		for (auto dataIter = m_spDatas.begin(); dataIter != m_spDatas.end();)
		{
			if (dataIter->second.use_count() < 2)
			{
				dataIter = m_spDatas.erase(dataIter);

				continue;
			}

			++dataIter;
		}
	}

private:
	std::unordered_map<std::string, std::shared_ptr<DataType>> m_spDatas;
	std::function<std::shared_ptr<DataType>(const std::string&)> m_customLoader;
};

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// フレームワークのクラスで使用するアセットをまとめたクラス
// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
// 複数存在する事を許さないのでシングルトンパターンを使用
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
class KdAssets
{
public:

	// それぞれのアセット
	KdDataStorage<KdTexture>	m_textures;
	KdDataStorage<KdModelData>	m_modeldatas;

	static KdAssets& Instance()
	{
		static KdAssets instance;
		return instance;
	}

	void ClearData(bool force)
	{
		m_textures.ClearData(force);
		m_modeldatas.ClearData(force);
	}

private:

	void Release()
	{
		ClearData(true);
	}

	KdAssets() {}
	~KdAssets() { Release(); }
};
