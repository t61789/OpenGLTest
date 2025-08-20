#pragma once

#include <unordered_map>
#include <string>

#include "const.h"

namespace op
{
    #define INCREF(obj) (obj)->IncRef(std::string(typeid(this).name()))
    #define INCREF_BY(obj, refObj) (obj)->IncRef(std::string(typeid(refObj).name()))
    #define DECREF(obj) (obj)->DecRef(std::string(typeid(this).name()))
    #define DECREF_BY(obj, refObj) (obj)->DecRef(std::string(typeid(refObj).name()))

    class SharedObject
    {
    public:
        StringHandle filePath = NOT_A_FILE;
        
        SharedObject();
        virtual ~SharedObject();

        void IncRef();
        void IncRef(const std::string& key);
        void DecRef();
        void DecRef(const std::string& key);

        static void RegisterResource(const StringHandle& path, SharedObject* obj);
        static void UnRegisterResource(const StringHandle& path);
        static bool TryGetResource(const StringHandle& path, SharedObject*& obj);
        
        static std::vector<SharedObject*> m_count;

    private:
        std::unordered_map<std::string, int> m_reference;
        static std::unordered_map<StringHandle, SharedObject*> m_resource;
    };
}
