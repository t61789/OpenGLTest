#pragma once
#include <functional>
#include <vector>

namespace op
{
    template<typename Return, typename Pt0, typename Pt1>
    using EventCallback = std::function<Return(Pt0, Pt1)>*;

    template<typename Return, typename Pt0, typename Pt1, typename T>
    std::function<Return(Pt0, Pt1)>* CreateCallback(T* obj, Return (T::*func)(Pt0, Pt1)) {
        return new std::function<Return(Pt0, Pt1)>([obj, func](Pt0 p0, Pt1 p1) {
            return (obj->*func)(p0, p1);
        });
    }

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
}
