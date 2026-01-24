#pragma once

// ログウィンドウ
// ・複数スレッドからの書き込みに対応
// ・ImGuiで表示
class LogWindow
{
public:
	static LogWindow& Instance()
	{
		static LogWindow instance;
		return instance;
	}

	void Init();
	void Draw();
	
	// ログ追加 (printf形式)
	void AddLog(const char* fmt, ...);

	void Clear();

private:
	LogWindow() {}
	~LogWindow() {}

	ImGuiTextBuffer		m_Buf;
	ImGuiTextFilter		m_Filter;
	ImVector<int>		m_LineOffsets; // Index to lines offset.
	bool				m_AutoScroll = true;

	std::mutex			m_mutex;       // スレッドセーフ用
};
