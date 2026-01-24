#pragma once

// プロファイルデータ（1区間の計測結果）
struct ProfileResult
{
	std::string m_name;
	float m_duration; // ms
	std::thread::id m_threadID;
	float m_startOffset; // フレーム開始からの経過時間 (ms)
};

// プロファイラ
class Profiler
{
public:
	static Profiler& Instance()
	{
		static Profiler instance;
		return instance;
	}

	void ResetFrame();
	void WriteProfile(const std::string& name, float duration, float startOffset);

	// ImGui描画
	void DrawProfilerWindow();

	// フレームごとの計測結果を取得
	const std::vector<ProfileResult>& GetResults() const { return m_results; }

    // フレーム開始時間を取得
    std::chrono::high_resolution_clock::time_point GetFrameStartTime() const { return m_frameStartTime; }

private:
	Profiler() {}
	
    std::chrono::high_resolution_clock::time_point m_frameStartTime;
	std::vector<ProfileResult> m_results;
	std::vector<ProfileResult> m_historyResults;
	std::mutex m_mutex;

	// グラフ描画用の一時バッファなどはcpp側で
	bool m_isPaused = false;
	float m_timeScale = 1.0f;
};

// RAII計測用クラス
class ScopedProfile
{
public:
	ScopedProfile(const std::string& name);
	~ScopedProfile();

private:
	std::string m_name;
	std::chrono::high_resolution_clock::time_point m_startTime;
};

// マクロ定義 (リリースビルドで無効化できるように)
#ifdef _DEBUG
#define PROFILE_FUNCTION() ScopedProfile timer(__FUNCTION__)
#define PROFILE_SCOPE(name) ScopedProfile timer(name)
#else
#define PROFILE_FUNCTION()
#define PROFILE_SCOPE(name)
#endif
