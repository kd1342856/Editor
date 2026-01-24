#include "ThreadManager.h"
#include "Profiler/Profiler.h"

void ThreadManager::Init()
{
	// すでに初期化済みなら何もしない
	if (!m_workers.empty()) return;

	// ハードウェアの並行実行可能なスレッド数を取得
	// (取得できない場合はとりあえず4スレッド)
	unsigned int numThreads = std::thread::hardware_concurrency();
	if (numThreads == 0) numThreads = 4;

	// メインスレッド分を引くかはお好みだが、
	// ここではフルパワーを使うためにそのまま作成する
    // (重い処理を投げる前提なので)
    
	m_stop = false;

	m_workers.reserve(numThreads);
	for (unsigned int i = 0; i < numThreads; ++i)
	{
		m_workers.emplace_back(std::bind(&ThreadManager::WorkerThreadLoop, this));
	}

	// Logger::Log("ThreadManager", KdFormat("Worker Threads Created: %d", numThreads));
}

void ThreadManager::Release()
{
	// 停止フラグを立てる
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		m_stop = true;
	}

	// 全スレッドを起こす
	m_condition.notify_all();

	// スレッドの終了を待機 (Join)
	for (std::thread& worker : m_workers)
	{
		if (worker.joinable())
		{
			worker.join();
		}
	}

	m_workers.clear();
}

void ThreadManager::WorkerThreadLoop()
{
	while (true)
	{
		Job job(nullptr);

		{
			std::unique_lock<std::mutex> lock(m_queueMutex);

			// ジョブが来るか、停止フラグが立つまで待機
			m_condition.wait(lock, [this] {
				return m_stop || !m_jobs.empty();
			});

			// 停止フラグかつジョブもなければ終了
			if (m_stop && m_jobs.empty())
			{
				return;
			}

			// ジョブを取り出す
			if (!m_jobs.empty())
			{
				job = std::move(m_jobs.front());
				m_jobs.pop();
			}
		}

		// ジョブ実行 (ロックの外で行う)
		if (job.m_func)
		{
			try
			{
				PROFILE_SCOPE("Job Execution");
				job.m_func();
			}
			catch (...)
			{
				// 例外が発生してもスレッドを止めない
				// Logger::Error("Exception in Worker Thread");
			}
		}
	}
}
