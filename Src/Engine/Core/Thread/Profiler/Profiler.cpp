#include "Profiler.h"

void Profiler::ResetFrame()
{
	// ポーズ中は履歴を更新しない（表示を固定）
	// ただし m_results はクリアしておかないとメモリが増え続けるのでクリアする
	if (!m_isPaused)
	{
		m_historyResults = std::move(m_results);
	}
	m_results.clear(); 
    m_frameStartTime = std::chrono::high_resolution_clock::now();
}

void Profiler::WriteProfile(const std::string& name, float duration, float startOffset)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	
	ProfileResult res;
	res.m_name = name;
	res.m_duration = duration;
	res.m_threadID = std::this_thread::get_id();
	res.m_startOffset = startOffset;
	
	m_results.push_back(res);
}

void Profiler::DrawProfilerWindow()
{
    if (ImGui::Begin("Profiler"))
    {
        // --- Control Panel ---
        ImGui::Checkbox("Pause", &m_isPaused);
        ImGui::SameLine();
        ImGui::SliderFloat("Scale", &m_timeScale, 0.1f, 10.0f, "x%.1f");
        
        ImGui::Text("Total Profiles: %d", m_historyResults.size());

        // --- Visualization ---
        // 各スレッドごとにレーンを分ける
        // threadID -> lane index のマップ
        // MainThread(今のスレッド)を0番にする
        std::map<std::thread::id, int> threadMap;
        threadMap[std::this_thread::get_id()] = 0;
        int laneCount = 1;

        // 描画領域など
        ImVec2 p = ImGui::GetCursorScreenPos();
        float width = ImGui::GetContentRegionAvail().x;
        float height = 300.0f; 
        float rowHeight = 30.0f;

        // 背景
        ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + width, p.y + height), IM_COL32(50, 50, 50, 255));

        // スケール計算
        // 16.6ms (60FPS) を基準に、Scale倍する
        // width = 33ms * scale くらいにする？
        // Scale 1.0 = 1画面 33ms (2フレーム分)
        float maxTime = 33.0f / m_timeScale; 
        float pxPerMs = width / maxTime;

        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& res : m_historyResults)
        {
            if (threadMap.find(res.m_threadID) == threadMap.end())
            {
                threadMap[res.m_threadID] = laneCount++;
            }
            int lane = threadMap[res.m_threadID];

            float x0 = p.x + res.m_startOffset * pxPerMs;
            float x1 = p.x + (res.m_startOffset + res.m_duration) * pxPerMs;
            float y0 = p.y + lane * rowHeight;
            float y1 = p.y + (lane + 1) * rowHeight - 2.0f; // 隙間

            // クリッピング
            if (x0 > p.x + width) continue;
            if (x1 < p.x) continue; // 左側もクリップ
            
            // 最小幅保証 (見えなくなるのを防ぐ)
            if (x1 - x0 < 1.0f) x1 = x0 + 1.0f;
            if (x1 > p.x + width) x1 = p.x + width;

            // 色分け (ハッシュ)
            std::hash<std::string> hasher;
            size_t h = hasher(res.m_name);
            ImU32 col = IM_COL32((h & 0xFF), ((h >> 8) & 0xFF) | 100, ((h >> 16) & 0xFF) | 100, 200);

            ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), col);
            
            // 文字は枠内に収まるときだけ
            if (x1 - x0 > 20.0f)
            {
                ImGui::GetWindowDrawList()->AddText(ImVec2(x0 + 2, y0 + 2), IM_COL32(255, 255, 255, 255), res.m_name.c_str());
            }

            // マウスホバーで詳細表示
            if (ImGui::IsMouseHoveringRect(ImVec2(x0, y0), ImVec2(x1, y1)))
            {
                ImGui::BeginTooltip();
                ImGui::Text("%s", res.m_name.c_str());
                ImGui::Text("Time: %.4f ms", res.m_duration);
                ImGui::Text("Offset: %.4f ms", res.m_startOffset);
                ImGui::EndTooltip();
                
                // ホバー時に枠を明るくする
                ImGui::GetWindowDrawList()->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), IM_COL32(255, 255, 255, 255));
            }
        }

        // 時間軸ガイド (16.6ms, 33.3ms)
        float guide16 = 16.66f * pxPerMs;
        if (guide16 < width) {
            ImGui::GetWindowDrawList()->AddLine(ImVec2(p.x + guide16, p.y), ImVec2(p.x + guide16, p.y + height), IM_COL32(100, 255, 100, 100));
        }

        ImGui::Dummy(ImVec2(width, height));
    }
    ImGui::End();
}

// --- ScopedProfile ---

ScopedProfile::ScopedProfile(const std::string& name)
    : m_name(name)
{
    m_startTime = std::chrono::high_resolution_clock::now();
}

ScopedProfile::~ScopedProfile()
{
    auto endTime = std::chrono::high_resolution_clock::now();
    
    // 経過時間 (ms)
    float duration = std::chrono::duration<float, std::milli>(endTime - m_startTime).count();
    
    // フレーム開始からのオフセット
    auto frameStart = Profiler::Instance().GetFrameStartTime();
    float startOffset = std::chrono::duration<float, std::milli>(m_startTime - frameStart).count();

    Profiler::Instance().WriteProfile(m_name, duration, startOffset);
}
