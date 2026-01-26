#include "Logger.h"

static ImGuiTextBuffer     Buf;
static ImGuiTextFilter     Filter;
static std::vector<int>    LineOffsets;        
static bool                AutoScroll = true;  

void Logger::Log(const std::string& category, const std::string& msg)
{
	Add("[%s] %s\n", category.c_str(), msg.c_str());
}

void Logger::Error(const std::string& msg)
{
	Add("[Error] %s\n", msg.c_str());
}

void Logger::DrawImGui()
{
	if (ImGui::Begin("Log"))
	{
		DrawAddLog();
	}
	ImGui::End();
}

void Logger::DrawAddLog()
{
	if (ImGui::Button("Clear"))
	{ 
		Buf.clear(); LineOffsets.clear(); LineOffsets.push_back(0); 
	}
	ImGui::SameLine();
	bool copy = ImGui::Button("Copy");
	ImGui::SameLine();
	Filter.Draw("Filter", -100.0f);

	ImGui::Separator();
	ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

	if (copy) ImGui::LogToClipboard();

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	const char* buf = Buf.begin();
	const char* buf_end = Buf.end();
	if (Filter.IsActive())
	{
		for (int line_no = 0; line_no < LineOffsets.size(); line_no++)
		{
			const char* line_start = buf + LineOffsets[line_no];
			const char* line_end = (line_no + 1 < LineOffsets.size()) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
			if (Filter.PassFilter(line_start, line_end))
			{
				ImGui::TextUnformatted(line_start, line_end);
			}
		}
	}
	else
	{
		ImGui::TextUnformatted(buf, buf_end);
	}
	ImGui::PopStyleVar();

	if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
	{
		ImGui::SetScrollHereY(1.0f);
	}
	ImGui::EndChild();
}

void Logger::Add(const char* fmt, ...)
{
	std::lock_guard<std::mutex> lock(s_mutex);

	int old_size = Buf.size();
	va_list args;
	va_start(args, fmt);
	Buf.appendfv(fmt, args);
	va_end(args);
	for (int new_size = Buf.size(); old_size < new_size; old_size++)
	{
		if (Buf[old_size] == '\n')
		{
			LineOffsets.push_back(old_size + 1);
		}
	}
}
