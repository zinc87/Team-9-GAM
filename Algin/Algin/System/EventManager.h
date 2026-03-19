#pragma once
#include "pch.h"
#include "../System/Event.h"
#include <functional>
#include <vector>
#include <unordered_map>

namespace AG
{
    // A delegate is a function pointer. This is the signature for any function
    // that wants to subscribe to an event. It takes a reference to an Event.
    using EventCallback = std::function<bool(Event&)>;
    using SubscriptionHandle = unsigned long long;
    class EventManager : public Pattern::ISingleton<EventManager>
    {
    public:
        // Subscribe now returns a handle
        SubscriptionHandle Subscribe(EventType eventType, const EventCallback& callback);

        // New function to unsubscribe
        void Unsubscribe(EventType eventType, SubscriptionHandle handle);


        // Publishes an event to all registered subscribers.
        void Publish(Event& e);

    private:

        struct Subscriber
        {
            SubscriptionHandle Handle;
            EventCallback Callback;
        };

        // A map where the key is the EventType and the value is a list of
        // all callback functions that are subscribed to that event type.
        std::unordered_map<EventType, std::vector<Subscriber>> m_Subscribers;

        std::atomic<SubscriptionHandle> m_NextHandle = 1; // Start handles at 1

    };
}
#define EVENT_MANAGER AG::EventManager::GetInstance()