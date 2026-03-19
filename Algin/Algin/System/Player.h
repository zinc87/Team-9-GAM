#pragma once
#include "pch.h"
#include "EventManager.h"

namespace AG
{
    class Player
    {
    public:
        Player();
        bool OnKeyEvent(Event& e);
    };
}
