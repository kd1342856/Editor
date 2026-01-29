#pragma once

class ImGuiFileBrowser
{
public:
	// ダイアログを開く
	// key: 識別子 (複数の場所で使うため。ImGuiFileDialogにおけるkey)
	// title: ウィンドウタイトル
	// filters: 拡張子フィルタ ({ ".json", ".txt" } など)
	// callback: 決定時のコールバック (フルパスが返る)
	void Open(const std::string& key, const std::string& title, const std::vector<std::string>& filters, std::function<void(const std::string&)> callback, const std::string& startPath = "Asset");

	void Draw();

private:
	ImGuiFileBrowser() = default;

	std::string m_dialogKey; // 現在開いているダイアログのキーを保持
	std::function<void(const std::string&)> m_onSelect;
public:
	static ImGuiFileBrowser& Instance()
	{
		static ImGuiFileBrowser instance;
		return instance;
	}
};
