#pragma once
struct LogEntry
{
	std::string category;
	std::string message;
};
class Logger
{
public:
	static void Log(const std::string& category, const std::string& msg);
	static void Error(const std::string& msg);
	static std::string GetTimestamp()
	{
		auto now = std::chrono::system_clock::now();
		auto time = std::chrono::system_clock::to_time_t(now);
		std::tm tm;
		localtime_s(&tm, &time);
		std::ostringstream oss;
		oss << std::put_time(&tm, "[%H:%M:%S]");
		return oss.str();
	};

	static void DrawImGui();			//	カテゴリごとのログ表示
	static void DrawAddLog();			//	Add()で追加されたログ描画
	static void Add(const char* fmt, ...);
private:
	static inline std::vector<LogEntry> s_logs;
	static inline std::mutex s_mutex;
};