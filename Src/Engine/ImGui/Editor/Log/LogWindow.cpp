#include "LogWindow.h"

void LogWindow::Init()
{
	Clear();
}

void LogWindow::Clear()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_Buf.clear();
	m_LineOffsets.clear();
	m_LineOffsets.push_back(0);
}

void LogWindow::AddLog(const char* fmt, ...)
{
	int old_size = m_Buf.size();
	va_list args;
	va_start(args, fmt);
	
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_Buf.appendfv(fmt, args);
		for (int new_size = m_Buf.size(); old_size < new_size; old_size++)
		{
			if (m_Buf[old_size] == '\n')
				m_LineOffsets.push_back(old_size + 1);
		}
	}
	
	va_end(args);
}

void LogWindow::Draw()
{
	if (!ImGui::Begin("Log"))
	{
		ImGui::End();
		return;
	}

	// Options menu
	if (ImGui::BeginPopup("Options"))
	{
		ImGui::Checkbox("Auto-scroll", &m_AutoScroll);
		ImGui::EndPopup();
	}

	// Main window
	if (ImGui::Button("Options"))
		ImGui::OpenPopup("Options");
	ImGui::SameLine();
	bool clear = ImGui::Button("Clear");
	ImGui::SameLine();
	bool copy = ImGui::Button("Copy");
	ImGui::SameLine();
	m_Filter.Draw("Filter", -100.0f);

	ImGui::Separator();

	if (ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
	{
		if (clear) Clear();
		if (copy) ImGui::LogToClipboard();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		const char* buf = m_Buf.begin();
		const char* buf_end = m_Buf.end();
		if (m_Filter.IsActive())
		{
			// In this example we don't use the clipper when Filter is enabled.
			// This is because we don't have random access to the result of the filter.
			// A real application processing warning/error/info entries might want to store them in a very different way.
			for (int line_no = 0; line_no < m_LineOffsets.Size; line_no++)
			{
				const char* line_start = buf + m_LineOffsets[line_no];
				const char* line_end = (line_no + 1 < m_LineOffsets.Size) ? (buf + m_LineOffsets[line_no + 1] - 1) : buf_end;
				if (m_Filter.PassFilter(line_start, line_end))
					ImGui::TextUnformatted(line_start, line_end);
			}
		}
		else
		{
			// The simplest and easy way to display the entire buffer:
			//   ImGui::TextUnformatted(buf_begin, buf_end);
			// And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
			// to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
			// within the visible area.
			// If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
			// on your side is recommended. Using ImGuiListClipper requires
			// - A) random access into your data
			// - B) items all being the  same height,
			// both of which we can do since we your data happens to be a single large buffer of same-height text.

			ImGuiListClipper clipper;
			clipper.Begin(m_LineOffsets.Size);
			while (clipper.Step())
			{
				for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
				{
					const char* line_start = buf + m_LineOffsets[line_no];
					const char* line_end = (line_no + 1 < m_LineOffsets.Size) ? (buf + m_LineOffsets[line_no + 1] - 1) : buf_end;
					ImGui::TextUnformatted(line_start, line_end);
				}
			}
			clipper.End();
		}
		ImGui::PopStyleVar();

		if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			ImGui::SetScrollHereY(1.0f);
	}
	ImGui::EndChild();
	ImGui::End();
}
