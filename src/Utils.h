#pragma once

#include <iostream>

#include "glad.h"
#include "glfw3.h"
#include "glm/glm.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "json.hpp"
#include "imgui.h"

#include "Event.h"

namespace op
{
    class Bounds;

    enum LogType
    {
        Info,
        Warning,
        Error,
    };

    template<typename T>
    class Singleton
    {
    public:
        ~Singleton()
        {
            m_instance = nullptr;
        }
        
        static T* GetInstance()
        {
            return m_instance;
        }
        
    private:
        friend T;
        
        Singleton()
        {
            if (m_instance)
            {
                throw std::runtime_error("Singleton instance already exists! type " + std::string(typeid(T).name()));
            }

            m_instance = static_cast<T*>(this);
        }
        
    protected:
        inline static T* m_instance = nullptr;
    };

    class Time : public Singleton<Time>
    {
    public:
        int frame = -1;
        float time = 0;
        float deltaTime = 999;
    };

    class Utils
    {
    public:
        static Event<GLFWwindow*, int, int> s_setFrameBufferSizeEvent;
        static std::vector<std::string> s_logs;
        
        static glm::vec3 ToVec3(const nlohmann::json& arr);
        static glm::vec4 ToVec4(const nlohmann::json& arr);

        static std::vector<std::string> ToDirectories(const std::string& path);
        static std::string GetAbsolutePath(const std::string& relativePath);
        
        template<typename Base, typename T>
        static bool InstanceOf(const T* ptr)
        {
            return dynamic_cast<const Base*>(ptr) != nullptr;
        }

        static std::string GetCurrentTimeFormatted(); 

        template <typename... Args>
        static std::string FormatString(const std::string& format, Args... args)
        {
            int size = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // 计算最终格式化字符串所需空间
            if (size <= 0) {
                throw std::runtime_error("Error during formatting.");
            }
            std::unique_ptr<char[]> buf(new char[size]); // 分配临时缓冲区
            std::snprintf(buf.get(), size, format.c_str(), args...); // 格式化字符串
            return std::string(buf.get(), buf.get() + size - 1); // 返回 std::string（去掉末尾的 \0）
        }
        static std::string FormatLog(LogType type, const std::string& msg);
        template <typename... Args>
        static void Log(LogType logType, const std::string& msg, Args... args)
        {
            auto logStr = FormatLog(logType, FormatString(msg, args...));
            s_logs.push_back(logStr);
            if (s_logs.size() > 50)
            {
                s_logs.erase(s_logs.begin());
            }
            std::cout << logStr << '\n';
        }
        template <typename... Args>
        static void LogInfo(const std::string& msg, Args... args)
        {
            return Log(Info, msg, args...);
        }
        template <typename... Args>
        static void LogWarning(const std::string& msg, Args... args)
        {
            return Log(Warning, msg, args...);
        }
        template <typename... Args>
        static void LogError(const std::string& msg, Args... args)
        {
            return Log(Error, msg, args...);
        }
        
        static std::string ToString(float val, int fixed);
        static std::string ToString(const glm::vec3& val);
        static std::string ToString(const glm::vec4& val);
        static std::string ToString(const glm::mat4& val);

        static float* ToArr(const glm::vec3& val);
        static glm::vec3 FromArr(const float* arr);

        static bool IsVec(const nlohmann::json& jsonValue, int components);
        static bool IsVec3(const nlohmann::json& jsonValue);
        static bool IsVec4(const nlohmann::json& jsonValue);

        static void ClearGlError();
        static void CheckGlError(const std::string& position);

        static void BeginDebugGroup(const std::string& groupName);
        static void EndDebugGroup();

        static bool EndsWith(const std::string& str, const std::string& suffix);
        static std::string JoinStrings(const std::vector<std::string>& strings, std::string delimiter);

        static glm::vec3 WorldToScreen(const glm::vec3& worldPos, const glm::mat4& viewMatrix, const glm::mat4& projMatrix, const glm::vec2& screenSize);
        static void DebugDrawLine(const glm::vec3& worldStart, const glm::vec3& worldEnd, const glm::mat4& viewMatrix, const glm::mat4& projMatrix, const glm::vec2& screenSize, ImU32 color = IM_COL32(255, 255, 255, 255), float thickness = 1.0f);
        static void DebugDrawCube(const Bounds& bounds, const glm::mat4& viewMatrix, const glm::mat4& projMatrix, const glm::vec2& screenSize, ImU32 color = IM_COL32(255, 255, 255, 255), float thickness = 1.0f);
        static int ComputeRegionCode(float x, float y, float xmin, float ymin, float xmax, float ymax);
        static bool CohenSutherlandClip(float &x1, float &y1, float &x2, float &y2, float xmin, float ymin, float xmax, float ymax);

        static nlohmann::json LoadJson(const std::string& assetPath);
        static void MergeJson(nlohmann::json& json1, const nlohmann::json& json2);
    };
}
