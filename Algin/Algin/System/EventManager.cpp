#include "pch.h"
#include "EventManager.h"
#include <algorithm>
namespace AG
{
    SubscriptionHandle EventManager::Subscribe(EventType eventType, const EventCallback& callback)
    {
        SubscriptionHandle handle = m_NextHandle++;
        m_Subscribers[eventType].push_back({ handle, callback });
        return handle;
    }

    void EventManager::Unsubscribe(EventType eventType, SubscriptionHandle handle)
    {
        if (m_Subscribers.find(eventType) == m_Subscribers.end())
        {
            return;
        }

        auto& subscribers = m_Subscribers[eventType];
        subscribers.erase(
            std::remove_if(subscribers.begin(), subscribers.end(),
                [handle](const Subscriber& sub) {
                    return sub.Handle == handle;
                }),
            subscribers.end()
        );
    }
    void EventManager::Publish(Event& e)
    {
        EventType type = e.GetEventType();

        if (m_Subscribers.find(type) != m_Subscribers.end())
        {
            // Note: We iterate over a copy in case a callback tries to unsubscribe,
            // which would invalidate the iterator of the original vector.
            auto subscribers = m_Subscribers.at(type);
            for (auto const& sub : subscribers)
            {
                e.Handled = sub.Callback(e);
                if (e.Handled)
                {
                    break;
                }
            }
        }
    }
}
