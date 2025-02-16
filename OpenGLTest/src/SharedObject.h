#pragma once
#include <memory>
#include <unordered_map>
#include <string>

class SharedObject
{
public:
    std::unique_ptr<std::string> filePath;
    
    SharedObject();
    virtual ~SharedObject() = default;

    void IncRef();
    void IncRef(const std::string& key);
    void DecRef();
    void DecRef(const std::string& key);

    static void RegisterResource(const std::string& path, SharedObject* obj);
    static void UnRegisterResource(const std::string& path);
    static bool TryGetResource(const std::string& path, SharedObject*& obj);

private:
    std::unique_ptr<std::unordered_map<std::string, int>> m_reference;
    static std::unordered_map<std::string, SharedObject*> m_resource;
};
