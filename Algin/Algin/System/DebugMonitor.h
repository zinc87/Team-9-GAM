#pragma once
#include "pch.h"
#include "EventManager.h"

namespace AG
{
    // This class subscribes to events and holds state.
    // It has no knowledge of ImGui or rendering.
    class DebugMonitor
    {
    public:
        DebugMonitor();

        // Public data for the ImGui view to access
        int GetKeyPressCount() const { return m_keyPressCount; }
        bool IsMouseDown() const { return m_isMouseDown; }

    private:
        // The callback functions that the EventManager will call
        bool OnKeyPress(Event& e);
        bool OnMouseButtonPress(Event& e);
        bool OnMouseButtonRelease(Event& e);

        // The state that our callbacks will modify
        int m_keyPressCount = 0;
        bool m_isMouseDown = false;
    };
}
