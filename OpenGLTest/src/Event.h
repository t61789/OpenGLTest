#pragma once

#include <functional>
#include <unordered_map>

template<typename... Args>
class Event
{
public:
    int addListener(std::function<void(Args...)> callback)
    {
        auto id = m_eventHandlerId ++;
        m_callbacks[id] = callback;
        return id;
    }

    void removeListener(int id)
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
    int m_eventHandlerId = 0;
    std::unordered_map<int, std::function<void(Args...)>> m_callbacks;
};
