#include "shared_object.h"

#include <cassert>

#include "object.h"
#include "utils.h"

namespace op
{
    std::unordered_map<std::string, SharedObject*> SharedObject::m_resource;
    std::vector<SharedObject*> SharedObject::m_count;

    SharedObject::SharedObject()
    {
        filePath = "NotAFile";
        m_count.push_back(this);
    }

    SharedObject::~SharedObject()
    {
        m_count.erase(std::remove(m_count.begin(), m_count.end(), this), m_count.end());
    }

    void SharedObject::IncRef()
    {
        static std::string defaultKey = "DefaultKey";
        IncRef(defaultKey);
    }

    void SharedObject::IncRef(const std::string& key)
    {
        // auto obj = dynamic_cast<Object*>(this);
        // if (obj && obj->name == std::string("GroundGrid"))
        // {
        //     Utils::LogInfo("asdasd");
        // }
        
        if (m_reference.find(key) == m_reference.end())
        {
            m_reference[key] = 1;
        }
        else
        {
            m_reference[key]++;
        }
    }

    void SharedObject::DecRef()
    {
        static std::string defaultKey = "DefaultKey";
        DecRef(defaultKey);
    }

    void SharedObject::DecRef(const std::string& key)
    {
        // auto obj = dynamic_cast<Object*>(this);
        // if (obj && obj->name == std::string("GroundGrid"))
        // {
        //     Utils::LogInfo("asdasd");
        // }
        
        auto it = m_reference.find(key);
        if (it == m_reference.end())
        {
            return;
        }
        assert(it != m_reference.end() && Utils::FormatString("DecRef时，key不存在，物体 %s，类型 %s, key %s", filePath.c_str(), typeid(*this).name(), key.c_str()).c_str());
        
        auto val = it->second - 1;
        if (val <= 0)
        {
            m_reference.erase(key);
            if (m_reference.empty())
            {
                UnRegisterResource(filePath);
                delete this;
            }
        }
        else
        {
            m_reference[key] = val;
        }
    }

    void SharedObject::RegisterResource(const std::string& path, SharedObject* obj)
    {
        auto it = m_resource.find(path);
        if (it == m_resource.end())
        {
            m_resource[path] = obj;
        }
        else
        {
            throw std::runtime_error("Resource already exists");
        }
        obj->filePath = path;
    }

    void SharedObject::UnRegisterResource(const std::string& path)
    {
        m_resource.erase(path);
    }

    bool SharedObject::TryGetResource(const std::string& path, SharedObject*& obj)
    {
        auto it = m_resource.find(path);
        if (it == m_resource.end())
        {
            return false;
        }
        
        obj = it->second;
        return true;
    }
}
