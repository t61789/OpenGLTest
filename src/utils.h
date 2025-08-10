#pragma once

#include <iostream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "nlohmann/json.hpp"
#include "imgui.h"

#include "event.h"

#include "string_handle.h"

#include "math/math.h"

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

    class Time
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
        
        static std::vector<std::string> ToDirectories(const std::string& path);
        static std::string GetAbsolutePath(const std::string& relativePath);
        static bool AssetExists(const std::string& path);

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

        static bool IsVec(const nlohmann::json& jsonValue, int components);
        static bool IsVec3(const nlohmann::json& jsonValue);
        static bool IsVec4(const nlohmann::json& jsonValue);

        static void ClearGlError();
        static void CheckGlError(const std::string& position);

        static void BeginDebugGroup(const std::string& groupName);
        static void EndDebugGroup();

        static bool EndsWith(const std::string& str, const std::string& suffix);
        static std::string JoinStrings(const std::vector<std::string>& strings, std::string delimiter);

        static Matrix4x4 CreateProjection(float fov, float aspect, float near, float far);
        static Matrix4x4 CreateOrthoProjection(float r, float l, float t, float b, float f, float n);
        static Vec3 WorldToScreen(const Vec3& worldPos, const Matrix4x4& viewMatrix, const Matrix4x4& projMatrix, const Vec2& screenSize);
        static void DebugDrawLine(const Vec3& worldStart, const Vec3& worldEnd, const Matrix4x4& viewMatrix, const Matrix4x4& projMatrix, const Vec2& screenSize, ImU32 color = IM_COL32(255, 255, 255, 255), float thickness = 1.0f);
        static void DebugDrawCube(const Bounds& bounds, const Matrix4x4& viewMatrix, const Matrix4x4& projMatrix, const Vec2& screenSize, ImU32 color = IM_COL32(255, 255, 255, 255), float thickness = 1.0f);
        static int ComputeRegionCode(float x, float y, float xmin, float ymin, float xmax, float ymax);
        static bool CohenSutherlandClip(float &x1, float &y1, float &x2, float &y2, float xmin, float ymin, float xmax, float ymax);

        static nlohmann::json LoadJson(const std::string& assetPath);
        static void MergeJson(nlohmann::json& json1, const nlohmann::json& json2, bool combineArray = false);

        static nlohmann::json GetResourceMeta(const std::string& assetPath);
        static std::vector<uint8_t> Base64ToBinary(const std::string& base64Str);
        static std::vector<uint32_t> Binary8To32(const std::vector<uint8_t>& data);
    };
}
