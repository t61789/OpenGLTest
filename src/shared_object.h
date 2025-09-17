#pragma once

#include <unordered_map>
#include <string>

#include "const.h"

namespace op
{
    class SharedObject
    {
    public:
        StringHandle filePath = NOT_A_FILE;
        
        SharedObject();
        virtual ~SharedObject();

        SharedObject* IncRef();
        SharedObject* IncRef(const std::string& key);
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
