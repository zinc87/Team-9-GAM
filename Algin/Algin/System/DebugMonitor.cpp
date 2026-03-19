#include "pch.h"
#include "DebugMonitor.h"
#include "../System/Key_Event.h"
#include "../System/Mouse_Event.h"

namespace AG
{
    DebugMonitor::DebugMonitor()
    {
        // In the constructor, we subscribe to the events we care about.
        EVENT_MANAGER.Subscribe(EventType::KeyPressed, std::bind(&DebugMonitor::OnKeyPress, this, std::placeholders::_1));
        EVENT_MANAGER.Subscribe(EventType::MouseButtonPressed, std::bind(&DebugMonitor::OnMouseButtonPress, this, std::placeholders::_1));
        EVENT_MANAGER.Subscribe(EventType::MouseButtonReleased, std::bind(&DebugMonitor::OnMouseButtonRelease, this, std::placeholders::_1));
    }

    // CORRECTED: Return bool
    bool DebugMonitor::OnKeyPress(Event& e)
    {
        (void)e;
        m_keyPressCount++;
        return false; // Not handled, just observing.
    }

    // CORRECTED: Return bool
    bool DebugMonitor::OnMouseButtonPress(Event& e)
    {
        (void)e;
        m_isMouseDown = true;
        return false; // Not handled, just observing.
    }

    // CORRECTED: Return bool
    bool DebugMonitor::OnMouseButtonRelease(Event& e)
    {
        (void)e;
        m_isMouseDown = false;
        return false; // Not handled, just observing.
    }
}
