#pragma once
#include <unordered_map>
#include <vector>

typedef unsigned long long RESOURCE_ID;
#define UNDEFINED_RESOURCE ((RESOURCE_ID)(-1))

class ResourceMgr
{
public:
    static RESOURCE_ID AddPtr(void* ptr)
    {
        RESOURCE_ID result = s_ptr.size();
        s_ptr.push_back(ptr);
        return result;
    }
    
    template <typename T>
    static T* GetPtr(const RESOURCE_ID id)
    {
        if(id == UNDEFINED_RESOURCE || id < 0 || id >= s_ptr.size())
        {
            return nullptr;
        }

        return dynamic_cast<T>(s_ptr[id]);
    }

    static void RemovePtr(const RESOURCE_ID id)
    {
        if(id == UNDEFINED_RESOURCE || id < 0 || id >= s_ptr.size())
        {
            return;
        }

        s_ptr[id] = nullptr;
    }

    static RESOURCE_ID GetRegisteredResource(const std::string& path)
    {
        if(!HasResourceRegistered(path))
        {
            return -1;
        }

        return s_pathMap[path];
    }

    static bool HasResourceRegistered(const std::string& path)
    {
        if(s_pathMap.find(path) == s_pathMap.end())
        {
            return false;
        }

        if(s_ptr[s_pathMap[path]] == nullptr)
        {
            return false;
        }

        return true;
    }

    static void RegisterResource(const std::string& path, RESOURCE_ID id)
    {
        if(HasResourceRegistered(path))
        {
            return;
        }
        
        s_pathMap[path] = id;
    }

private:
    static std::vector<void*> s_ptr;
    static std::unordered_map<std::string, RESOURCE_ID> s_pathMap;
};
