#pragma once
// ジョブ
struct Job
{
	enum class Priority
	{
		High,
		Normal,
		Low
	};

	// 実行する関数
	std::function<void()> m_func;

	// 優先度
	Priority m_priority = Priority::Normal;

	// コンストラクタ
	Job(std::function<void()> func, Priority prio = Priority::Normal)
		: m_func(func), m_priority(prio) {}
};

// スレッド
class ThreadManager
{
public:
	// 初期化 
	void Init();

	// 終了処理
	void Release();

	// ジョブ投入 
	template<typename Func, typename... Args>
	auto AddJob(Func&& func, Args&&... args) -> std::future<typename std::invoke_result<Func, Args...>::type>
	{
		using ReturnType = typename std::invoke_result<Func, Args...>::type;

		// タスクをパッケージ化
		auto task = std::make_shared<std::packaged_task<ReturnType()>>(
			std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
		);

		std::future<ReturnType> res = task->get_future();

		{
			std::unique_lock<std::mutex> lock(m_queueMutex);

			// ラムダ式でラップしてキューに入れる
			// (packaged_taskを実行するだけのジョブ)
			m_jobs.emplace([task]() { (*task)(); });
		}

		// 待機中のスレッドを1つ起こす
		m_condition.notify_one();

		return res;
	}

	// 優先度付きジョブ投入
	template<typename Func, typename... Args>
	auto AddJobWithPriority(Job::Priority priority, Func&& func, Args&&... args) -> std::future<typename std::invoke_result<Func, Args...>::type>
	{
		using ReturnType = typename std::invoke_result<Func, Args...>::type;

		// タスクをパッケージ化
		auto task = std::make_shared<std::packaged_task<ReturnType()>>(
			std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
		);

		std::future<ReturnType> res = task->get_future();

		{
			std::unique_lock<std::mutex> lock(m_queueMutex);

			// 優先度付きでキューに入れる
			m_jobs.emplace([task]() { (*task)(); }, priority);
		}

		// 待機中のスレッドを1つ起こす
		m_condition.notify_one();

		return res;
	}

private:
	ThreadManager() {}
	~ThreadManager() { Release(); }

	// ワーカースレッドのループ関数
	void WorkerThreadLoop();

	// ワーカースレッドリスト
	std::vector<std::thread> m_workers;

	// ジョブキュー
	std::queue<Job> m_jobs;

	// 排他制御用
	std::mutex m_queueMutex;
	std::condition_variable m_condition;

	// 終了フラグ
	std::atomic<bool> m_stop = false;

public:
	static ThreadManager& Instance()
	{
		static ThreadManager instance;
		return instance;
	}
};
