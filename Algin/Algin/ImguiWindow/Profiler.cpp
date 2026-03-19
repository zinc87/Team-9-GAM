#include "pch.h"
#include "Profiler.h"

static ULARGE_INTEGER last_idle, last_kernel, last_user;
void AG::AGImGui::Profiler::Render()
{
    if (last_update + update_gaps < glfwGetTime() && !freeze)
    {
        imgui_buffer = Benchmarker::GetInstance().GetProfile();
        last_update = glfwGetTime();
        delta_time = Benchmarker::GetInstance().GetDeltaTime();
        fps = static_cast<int>(1.0f / delta_time);

        for (auto& session : imgui_buffer)
        {
            float duration = (static_cast<float>(session.second.second) - static_cast<float>(session.second.first)) * 1000.0f; // ms
            auto& hist = history_buffer[session.first];

            if (hist.size() >= max_history)
                hist.pop_front(); // remove oldest
            hist.push_back(duration);
        }
        if (cpu_usage.size() >= max_history)
            cpu_usage.pop_front();
        cpu_usage.push_back(GetCPUUsage());
    }

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;
	ImGui::Begin("Profiler", nullptr, windowFlags);

    ImGui::Text("FPS : %i", fps);
    ImGui::Text("Delta Time : %.5f ms", delta_time);

    /* --- block positioning --- */
    float block_height = 20.f;
    float block_gap = 2.f;
    ImVec2 block_position = ImGui::GetCursorScreenPos();
    float x_region = ImGui::GetContentRegionAvail().x;

    /*  --- update timing --- */
    auto update_pair = imgui_buffer["Update"];
    total_update_time = update_pair.second - update_pair.first;

    DrawPerformanceBlock("Update", block_position, ImVec2(x_region, block_height), ImColor(0.5f, 0.5f, 0.5f, 1.0f));
    block_position += ImVec2(0.f, block_height + block_gap);
    for (auto& session : imgui_buffer)
    {
        if (session.first == "Update") continue;

        float session_start = static_cast<float>(session.second.first);
        float session_end = static_cast<float>(session.second.second);

        float session_start_offset = static_cast<float>((session_start - update_pair.first) / total_update_time);
        float session_size = static_cast<float>((session_end - session_start) / total_update_time);

        ImVec2 subblock_pos = block_position + ImVec2(x_region * session_start_offset,0.f);
        ImVec2 subblock_size = ImVec2(x_region * session_size, block_height);
        DrawPerformanceBlock(session.first, subblock_pos, subblock_size , GetColor(session.first));

    }

    ImGui::SetCursorScreenPos(block_position + ImVec2(0.f, block_height * 1.5f));

    /* --- session table --- */
    if (ImGui::BeginTable("ProfilerTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Session");
        ImGui::TableSetupColumn("Duration (ms)");
        ImGui::TableSetupColumn("Percent of Update");
        ImGui::TableHeadersRow();

        for (auto& session : imgui_buffer)
        {
            ImGui::TableNextRow();

            float start = static_cast<float>(session.second.first);
            float end = static_cast<float>(session.second.second);
            float duration = static_cast<float>(end - start); // seconds
            float duration_ms = duration * 1000.0f;

            float percent = (total_update_time > 0.0)
                ? static_cast<float>((duration / total_update_time) * 100.0)
                : 0.0f;

            /* --- Session name column --- */
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(session.first.c_str());

            // Tooltip with history graph
            if (ImGui::IsItemHovered())
            {
                auto it = history_buffer.find(session.first);
                if (it != history_buffer.end() && !it->second.empty())
                {
                    std::vector<float> values(it->second.begin(), it->second.end());

                    float last = values.back();
                    float minVal = *std::min_element(values.begin(), values.end());
                    float maxVal = *std::max_element(values.begin(), values.end());
                    float avgVal = std::accumulate(values.begin(), values.end(), 0.0f) / values.size();

                    ImGui::BeginTooltip();

                    // Session name as a header
                    ImGui::Text("Session: %s", session.first.c_str());

                    // History graph
                    ImGui::PlotLines("##history",
                        values.data(),
                        (int)values.size(),
                        0,
                        NULL,
                        0.0f,
                        maxVal,
                        ImVec2(200, 80));

                    // Neat table for stats
                    if (ImGui::BeginTable("StatsTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
                    {
                        ImGui::TableSetupColumn("Metric");
                        ImGui::TableSetupColumn("Value (ms)");
                        ImGui::TableHeadersRow();

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Last");
                        ImGui::TableSetColumnIndex(1); ImGui::Text("%.3f", last);

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Min");
                        ImGui::TableSetColumnIndex(1); ImGui::Text("%.3f", minVal);

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Max");
                        ImGui::TableSetColumnIndex(1); ImGui::Text("%.3f", maxVal);

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Avg");
                        ImGui::TableSetColumnIndex(1); ImGui::Text("%.3f", avgVal);

                        ImGui::EndTable();
                    }

                    ImGui::EndTooltip();
                }
            }



            /* --- Duration column --- */
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%.3f", duration_ms);

            /* --- Percent column --- */
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%.2f %%", percent);
        }

        ImGui::EndTable();
    }


    ImGui::SeparatorText("CPU");
    { /* --- CPU USAGE --- */
        ImVec2 cpu_size = { ImGui::GetContentRegionAvail().x,
                            ImGui::GetContentRegionAvail().x * 0.3f };

        std::vector<float> cpu(cpu_usage.begin(), cpu_usage.end());

        float current = cpu.back();
        float avg = std::accumulate(cpu.begin(), cpu.end(), 0.0f) / cpu.size();
        float minVal = *std::min_element(cpu.begin(), cpu.end());
        float maxVal = *std::max_element(cpu.begin(), cpu.end());

        ImGui::PlotLines("##cpu_history",cpu.data(),(int)cpu.size(),0,NULL,0.0f,100.0f,cpu_size);

        ImVec2 pos = ImGui::GetItemRectMin();
        ImVec2 size = ImGui::GetItemRectSize();
        char overlay[64];
        snprintf(overlay, sizeof(overlay), "CPU Usage: %.1f%%", current);

        ImVec2 text_size = ImGui::CalcTextSize(overlay);
        ImGui::GetWindowDrawList()->AddText( ImVec2(pos.x + (size.x - text_size.x) * 0.5f, pos.y ), IM_COL32(255, 255, 255, 255), overlay);

        ImGui::Text("Min: %.1f%%   Max: %.1f%%   Avg: %.1f%%", minVal, maxVal, avg);
    }


    ImGui::SeparatorText("GPU");
    {
        ImGui::Text("Renderer:   %s", glGetString(GL_RENDERER));
        ImGui::Text("Vendor:     %s", glGetString(GL_VENDOR));
        ImGui::Text("Version:    %s", glGetString(GL_VERSION));
        ImGui::Text("Object Rendered: %d", Benchmarker::GetInstance().GetDrawCalls());
        ImGui::Text("Object Occluded: %d", Benchmarker::GetInstance().GetDrawCalls());
    }




    if (ImGui::Button(freeze ? "Unfreeze" : "Freeze"))
    {
        freeze = !freeze;
    }

    ImGui::Text("Frustum Checks: %u", BENCHMARKER.GetFrustumCheck());
    ImGui::Text("Frustum Culled: %d", BENCHMARKER.GetFrustumCulled());

	ImGui::End();
}

ImColor AG::AGImGui::Profiler::GetColor(const std::string& name)
{
    std::hash<std::string> hasher;
    size_t h = hasher(name);

    float r = ((h >> 0) & 0xFF) / 255.0f;
    float g = ((h >> 8) & 0xFF) / 255.0f;
    float b = ((h >> 16) & 0xFF) / 255.0f;

    // Pastel formula: blend with white
    r = (r + 1.0f) * 0.5f;
    g = (g + 1.0f) * 0.5f;
    b = (b + 1.0f) * 0.5f;

    return ImColor(r, g, b, 1.0f);
}

float AG::AGImGui::Profiler::GetCPUUsage()
{
    FILETIME idleTime, kernelTime, userTime;
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime))
        return -1.0f;

    ULARGE_INTEGER idle, kernel, user;
    idle.QuadPart = ((ULARGE_INTEGER&)idleTime).QuadPart;
    kernel.QuadPart = ((ULARGE_INTEGER&)kernelTime).QuadPart;
    user.QuadPart = ((ULARGE_INTEGER&)userTime).QuadPart;

    ULONGLONG sys = (kernel.QuadPart - last_kernel.QuadPart) + (user.QuadPart - last_user.QuadPart);
    ULONGLONG idleDiff = idle.QuadPart - last_idle.QuadPart;

    last_idle = idle; last_kernel = kernel; last_user = user;

    return sys > 0 ? (float)(sys - idleDiff) * 100.0f / sys : 0.0f;
}

void AG::AGImGui::Profiler::DrawPerformanceBlock(std::string name, ImVec2 pos, ImVec2 size, ImColor color)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    draw_list->AddRectFilled( pos, pos + size, color );
    draw_list->AddRect( pos, pos + size, ImColor(0.f, 0.f, 0.f, 1.f));

    ImVec2 text_size = ImGui::CalcTextSize(name.c_str());

    ImVec2 text_pos;
    text_pos.x = pos.x + (size.x - text_size.x) * 0.5f;
    text_pos.y = pos.y + (size.y - text_size.y) * 0.5f;

    draw_list->PushClipRect(pos, pos + size, true);
    draw_list->AddText(text_pos, IM_COL32(0, 0, 0, 255), name.c_str());
    draw_list->PopClipRect();

    /* --- pop up upon hover --- */
    std::string identifier = "##performace_" + name;

    ImVec2 mouse = ImGui::GetIO().MousePos;
    if (mouse.x >= pos.x && mouse.x <= pos.x + size.x &&
        mouse.y >= pos.y && mouse.y <= pos.y + size.y)
    {
        auto& session = imgui_buffer[name];
        double duration = (session.second - session.first); // in s
        double percentage = (duration / total_update_time) * 100.0;
        duration *= 1000.0; // in ms

        ImGui::BeginTooltip();
        ImGui::Text("Session: %s", name.c_str());
        ImGui::Text("Duration: %.3f ms", duration);
        ImGui::Text("Percentage: %.1f %", percentage);
        ImGui::EndTooltip();
    }


}
