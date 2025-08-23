#pragma once

#include <iostream>
#include <sstream>
#include <random>
#include <cstddef>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "nlohmann/json.hpp"
#include "imgui.h"

#include "event.h"
#include "const.h"

namespace op
{
    class Bounds;

    enum LogType
    {
        Info,
        Warning,
        Error,
    };

    class Utils
    {
    public:
        static std::vector<std::string> s_logs;
        
        static std::vector<std::string> ToDirectories(const std::string& path);
        static std::string GetAbsolutePath(const std::string& relativePath);
        static bool AssetExists(const std::string& path);

        static std::string GetCurrentTimeFormatted(); 

        static nlohmann::json LoadJson(const std::string& assetPath);
        static void MergeJson(nlohmann::json& json1, const nlohmann::json& json2, bool combineArray = false);

        static nlohmann::json GetResourceMeta(const std::string& assetPath);
        static std::vector<uint8_t> Base64ToBinary(const std::string& base64Str);
        static std::vector<uint32_t> Binary8To32(const std::vector<uint8_t>& data);
    };

    template <typename T>
    struct IndexObj
    {
        size_t index;
        T obj;
    };
    
    template <typename... Args>
    static std::string format_string(const std::string& format, Args... args)
    {
        int size = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // 计算最终格式化字符串所需空间
        if (size <= 0) {
            throw std::runtime_error("Error during formatting.");
        }
        std::unique_ptr<char[]> buf(new char[size]); // 分配临时缓冲区
        std::snprintf(buf.get(), size, format.c_str(), args...); // 格式化字符串
        return std::string(buf.get(), buf.get() + size - 1); // 返回 std::string（去掉末尾的 \0）
    }

    template <typename... Args>
    std::string format_log(const LogType type, const std::string& format, Args... args)
    {
        std::stringstream ss;
        ss << "[";
        ss << Utils::GetCurrentTimeFormatted();
        ss << "]";
        switch (type)
        {
            case Info:
                ss<<"[Info] ";
            break;
            case Warning:
                ss<<"[Warning] ";
            break;
            case Error:
                ss<<"[Error] ";
            break;
        }

        ss << format_string(format, args...);
        return ss.str();
    }

    template <typename... Args>
    static void log(const LogType logType, const std::string& msg, Args... args)
    {
        auto logStr = format_log(logType, msg, args...);
        Utils::s_logs.push_back(logStr);
        if (Utils::s_logs.size() > 50)
        {
            Utils::s_logs.erase(Utils::s_logs.begin());
        }
        std::cout << logStr << '\n';
    }
    
    template <typename... Args>
    static void log_info(const std::string& msg, Args... args)
    {
        return log(Info, msg, args...);
    }

    template <typename... Args>
    static void log_warning(const std::string& msg, Args... args)
    {
        return log(Warning, msg, args...);
    }

    template <typename... Args>
    static void log_error(const std::string& msg, Args... args)
    {
        return log(Error, msg, args...);
    }
    
    template <typename T>
    static T* find(std::vector<IndexObj<T>>& vec, const size_t key)
    {
        for (auto& v : vec)
        {
            if (v.index == key)
            {
                return &v.obj;
            }
        }

        return nullptr;
    }

    template <typename K, typename V>
    static V* find(std::vector<V>& vec, const K V::* keyField, const K& key)
    {
        for (auto& v : vec)
        {
            if (v.*keyField == key)
            {
                return &v;
            }
        }

        return nullptr;
    }
    
    template <typename V>
    static V* find(std::vector<V*>& vec, const StringHandle V::* keyField, const size_t key)
    {
        for (auto& v : vec)
        {
            if (v->*keyField == key)
            {
                return v;
            }
        }

        return nullptr;
    }
    
    template <typename K, typename V>
    static int find_index(std::vector<V>& vec, const K V::* keyField, const K& key)
    {
        int i = 0;
        for (auto& v : vec)
        {
            if (v.*keyField == key)
            {
                return i;
            }
            i++;
        }

        return -1;
    }
    
    template <typename K>
    static int find_index(std::vector<K>& vec, const K& key)
    {
        int i = 0;
        for (auto& v : vec)
        {
            if (v == key)
            {
                return i;
            }
            i++;
        }

        return -1;
    }
    
    template <typename T>
    static int find_index(std::vector<IndexObj<T>>& vec, const size_t key)
    {
        int i = 0;
        for (auto& v : vec)
        {
            if (v.index == key)
            {
                return i;
            }
            i++;
        }

        return i;
    }

    template <typename T>
    static bool exists(std::vector<T>& vec, const T& obj)
    {
        return std::find(vec.begin(), vec.end(), obj) != vec.end();
    }
    
    #define GL_CHECK_ERROR(position) { auto errors = read_gl_error(); if (!errors.empty()) { throw_gl_error(errors, #position); }}
    #define GL_CLEAR_ERROR { GLuint error; while((error = glGetError()) != GL_NO_ERROR) {}}

    static const std::vector<int>& read_gl_error()
    {
        static std::vector<int> errors;
        errors.clear();
        GLenum error;
        while((error = glGetError()) != GL_NO_ERROR)
        {
            errors.push_back(static_cast<int>(error));
        }
        return errors;
    }

    static void throw_gl_error(const std::vector<int>& errors, const std::string& position)
    {
        static std::stringstream ss;
        ss.clear();
        ss << "在 [" << position << "] 位置检测到以下GL错误：";
        for (size_t i = 0; i < errors.size(); ++i)
        {
            if(i != 0)
            {
                ss << ", ";
            }
            ss << std::hex << errors[i];
        }
        auto s = ss.str();
        throw std::runtime_error(s);
    }

    static void begin_debug_group(const char* name)
    {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name);
    }

    static void end_debug_group()
    {
        glPopDebugGroup();
    }

    template<typename Base, typename T>
    static bool instance_of(const T* ptr)
    {
        return dynamic_cast<const Base*>(ptr) != nullptr;
    }

    static bool ends_with(const std::string& str, const std::string& suffix)
    {
        return str.size() >= suffix.size() && str.rfind(suffix) == str.size() - suffix.size();
    }
    
    static std::string join(const std::vector<std::string>& strings, const std::string& delimiter)
    {
        std::stringstream ss;
        for (size_t i = 0; i < strings.size(); ++i)
        {
            ss << strings[i];
            if (i != strings.size() - 1)
            {
                ss << delimiter;
            }
        }
        return ss.str();
    }

    static std::string to_string(const float val, const int fixed)
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(fixed) << val;
        return oss.str();
    }
    
    static bool is_vec(const nlohmann::json& jsonValue, const size_t components)
    {
        if (jsonValue.is_array() && jsonValue.size() == components)
        {
            bool allFloat = true;
            for (auto& e : jsonValue)
            {
                allFloat &= e.is_number();
            }
            return allFloat;
        }

        return false;
    }
    
    static bool is_vec3(const nlohmann::json& jsonValue)
    {
        return is_vec(jsonValue, 3);
    }
    
    static bool is_vec4(const nlohmann::json& jsonValue)
    {
        return is_vec(jsonValue, 4);
    }

    template <typename T>
    static T* aligned_malloc(const size_t size, const size_t alignment)
    {
        return static_cast<T*>(_aligned_malloc(size, alignment));
    }

    static void aligned_free(void* ptr)
    {
        _aligned_free(ptr);
    }

    static std::string replace(const std::string& original, const std::string& toReplace, const std::string& replacement)
    {
        std::string result = original;
        size_t pos = 0;
    
        // 循环查找并替换所有匹配的子字符串
        while ((pos = result.find(toReplace, pos)) != std::string::npos)
        {
            result.replace(pos, toReplace.length(), replacement);
            pos += replacement.length(); // 移动位置到替换后的字符串之后
        }
    
        return result;
    }

    template <typename T, typename K>
    static void sort(std::vector<T>& vec, K T::* key)
    {
        std::sort(vec.begin(), vec.end(), [key](const T& a, const T& b) { return a.*key < b.*key; });
    }

    template <typename T, typename K>
    static T& max_element(std::vector<T>& vec, K T::* key)
    {
        return *std::max_element(vec.begin(), vec.end(), [key](const T& a, const T& b) { return a.*key < b.*key; });
    }
    
    static size_t get_random_size_t()
    {
        static std::random_device rd;
        static std::mt19937_64 gen(rd()); // 使用64位引擎，因为size_t可能是64位
        static std::uniform_int_distribution<size_t> dis;
    
        return dis(gen);
    }

    template<typename T>
    class Singleton
    {
    public:
        ~Singleton() { m_instance = nullptr; }
        static T* Ins() { return m_instance; }
        
    private:
        friend T;
        
        Singleton()
        {
            if (m_instance)
            {
                THROW_ERROR("Singleton instance already exists! type ", std::string(typeid(T).name()).c_str())
            }

            m_instance = static_cast<T*>(this);
        }
        
    protected:
        inline static T* m_instance = nullptr;
    };

    class Time
    {
    public:
        int frame = -1;
        float time = 0;
        float deltaTime = 999;
    };
}
