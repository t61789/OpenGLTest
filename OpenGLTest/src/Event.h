#pragma once

#include <functional>
#include <unordered_map>

template<typename... Args>
class Event
{
public:
    size_t addListener(std::function<void(Args...)> callback)
    {
        auto id = m_eventHandlerId ++;
        m_callbacks[id] = callback;
        return id;
    }

    void removeListener(size_t id)
    {
        m_callbacks.erase(id);
    }

    void invoke(Args... args)
    {
        for (auto& callback : m_callbacks)
        {
            callback.second(args...);
        }
    }

private:
    size_t m_eventHandlerId = 0;
    std::unordered_map<size_t, std::function<void(Args...)>> m_callbacks;
};
