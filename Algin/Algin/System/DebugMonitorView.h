#pragma once
#include "pch.h"
#include "../ImguiWindow/IImguiWindow.h"
#include "DebugMonitor.h" // Needs to know about the monitor to read its data

namespace AG
{
    namespace AGImGui
    {
        // This is purely for rendering. It holds a pointer to the
        // DebugMonitor to get the data it needs to draw.
        class DebugMonitorView : public IImguiWindow
        {
        public:
            DebugMonitorView(DebugMonitor* monitor)
                : IImguiWindow({ 300.f, 100.f }, "Debug Monitor"), m_monitor(monitor) {
            }

            ~DebugMonitorView() override = default;
            void Render() override;

        private:
            DebugMonitor* m_monitor; // A raw pointer is fine, as Application owns the monitor
        };
    }
}
