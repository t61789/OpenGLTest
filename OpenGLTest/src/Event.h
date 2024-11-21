#pragma once
#include <functional>
#include <unordered_map>

template<typename ...T>
class Event
{
public:
    void Register(long long key, std::function<void(T...)> func);
    void UnRegister(long long key);
    void Dispatch(T... params);
    
private:
    std::unordered_map<long long, std::function<void(T...)>> _funcs;
};

template <typename ... T>
void Event<T...>::Register(long long key, std::function<void(T...)> func)
{
    _funcs[key] = func;
}

template <typename ... T>
void Event<T...>::UnRegister(long long key)
{
    _funcs.erase(key);
}

template <typename ... T>
void Event<T...>::Dispatch(T... params)
{
    for (auto& p : _funcs)
    {
        p.second(params...);
    }
}
