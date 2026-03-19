#include "pch.h"
#include "Player.h"
#include "../System/Key_Event.h"
#include <iostream>

namespace AG
{
    Player::Player()
    {
        // This line will now compile correctly because the signatures match.
        EVENT_MANAGER.Subscribe(EventType::KeyPressed, std::bind(&Player::OnKeyEvent, this, std::placeholders::_1));
    }

    // CORRECTED: The function now returns bool.
    bool Player::OnKeyEvent(Event& e)
    {
        KeyPressedEvent& keyEvent = static_cast<KeyPressedEvent&>(e);

        // This will print to your Visual Studio Output window, not a separate console.
        // You can view it by going to View -> Output in Visual Studio.
        std::cout << "Player heard a key press: " << keyEvent.GetKeyCode() << std::endl;

        // Return false because the Player is just observing the event.
        // It isn't "handling" it to stop other systems from using it.
        return false;
    }
}
