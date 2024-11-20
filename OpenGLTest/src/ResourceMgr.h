#pragma once
#include <unordered_map>
#include <vector>

typedef unsigned long long RESOURCE_ID;
#define UNDEFINED_RESOURCE ((RESOURCE_ID)(-1))

class ResourceInfo
{
public:
    RESOURCE_ID id = UNDEFINED_RESOURCE;
    std::string path;

    ResourceInfo() = default;

    ResourceInfo(const RESOURCE_ID id, const std::string& path)
    {
        this->id = id;
        this->path = path;
    }
};

class ResourceBase
{
public:
    RESOURCE_ID m_id = UNDEFINED_RESOURCE;

    ResourceBase();
    virtual ~ResourceBase();
};

class ResourceMgr
{
public:
    static RESOURCE_ID AddPtr(ResourceBase* ptr)
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

        return dynamic_cast<T*>(s_ptr[id]);
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
        if(!IsResourceRegistered(path))
        {
            return UNDEFINED_RESOURCE;
        }

        return s_pathMap[path].id;
    }

    static bool IsResourceRegistered(const std::string& path)
    {
        if(s_pathMap.find(path) == s_pathMap.end())
        {
            return false;
        }

        if(s_ptr[s_pathMap[path].id] == nullptr)
        {
            return false;
        }

        return true;
    }
    
    static bool IsResourceRegistered(const RESOURCE_ID id)
    {
        return s_resourceInfoMap.find(id) != s_resourceInfoMap.end();
    }

    static void RegisterResource(const std::string& path, const RESOURCE_ID id)
    {
        if(IsResourceRegistered(path))
        {
            return;
        }

        auto resourceInfo = ResourceInfo(id, path);
        s_pathMap[path] = resourceInfo;
        s_resourceInfoMap[id] = resourceInfo;
    }

    static void UnregisterResource(const RESOURCE_ID id)
    {
        if(!IsResourceRegistered(id))
        {
            return;
        }

        s_pathMap.erase(s_resourceInfoMap[id].path);
        s_resourceInfoMap.erase(id);
    }

private:
    static std::vector<ResourceBase*> s_ptr;
    static std::unordered_map<std::string, ResourceInfo> s_pathMap;
    static std::unordered_map<RESOURCE_ID, ResourceInfo> s_resourceInfoMap;
};

inline ResourceBase::ResourceBase()
{
    m_id = ResourceMgr::AddPtr(this);
}

inline ResourceBase::~ResourceBase()
{
    ResourceMgr::RemovePtr(m_id);
}

