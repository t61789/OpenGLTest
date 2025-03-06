#pragma once
#include <memory>
#include <functional>
#include <vector>

template<typename... Args>
class Event
{
public:
    void AddCallBack(std::function<void(Args...)>* callback)
    {
        m_callbacks.push_back(callback);
    }

    void RemoveCallBack(std::function<void(Args...)>* callback)
    {
        m_callbacks.erase(std::remove(m_callbacks.begin(), m_callbacks.end(), callback), m_callbacks.end());
    }

    void Invoke(Args... args)
    {
        for (auto& callback : m_callbacks)
        {
            (*callback)(args...);
        }
    }

private:
    std::vector<std::function<void(Args ...)>*> m_callbacks;
};
