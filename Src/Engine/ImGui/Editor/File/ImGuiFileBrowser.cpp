#include "ImGuiFileBrowser.h"

void ImGuiFileBrowser::Open(const std::string& key, const std::string& title, const std::vector<std::string>& filters, std::function<void(const std::string&)> callback)
{
	m_dialogKey = key;
	m_onSelect = callback;

	// フィルタリストをカンマ区切りの文字列に変換 (例: ".cpp,.h")
	std::string filterStr;
	for (size_t i = 0; i < filters.size(); ++i)
	{
		filterStr += filters[i];
		if (i < filters.size() - 1)
			filterStr += ",";
	}
	// 空の場合はすべて
	if (filterStr.empty()) filterStr = ".*";

	IGFD::FileDialogConfig config;
	config.path = ".";
	ImGuiFileDialog::Instance()->OpenDialog(key, title.c_str(), filterStr.c_str(), config);
}

void ImGuiFileBrowser::Draw()
{
	// 指定したキーのダイアログが開かれているかチェック & 表示
	if (ImGuiFileDialog::Instance()->Display(m_dialogKey))
	{
		// ユーザーがOKなどを押して閉じた場合ここに来る
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
			
			// コールバック呼び出し
			if (m_onSelect)
			{
				m_onSelect(filePathName);
			}
		}

		// ダイアログ終了処理 (閉じる)
		ImGuiFileDialog::Instance()->Close();
	}
}
