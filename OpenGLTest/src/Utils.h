﻿#pragma once

#include "glm.hpp"
#include <glad/glad.h>
#include <vec3.hpp>
#include <vec4.hpp>

#include "Event.h"
#include "../lib/json.hpp"
#include "glfw3.h"
#include "imgui.h"

enum LogType
{
    Info,
    Warning,
    Error,
};

class Bounds
{
public:
    glm::vec3 center;
    glm::vec3 extents;

    Bounds();
    Bounds(glm::vec3 center, glm::vec3 extents);
};

class Utils
{
public:
    static Event<GLFWwindow*, int, int> s_setFrameBufferSizeEvent;
    static std::vector<std::string> s_logs;
    
    static glm::vec3 ToVec3(nlohmann::json arr);
    static glm::vec4 ToVec4(nlohmann::json arr);

    static std::vector<std::string> ToDirectories(const std::string& path);
    
    static std::string GetRealAssetPath(const std::string& relativePath);
    static std::string GetRealAssetPath(const std::string& relativePath, const std::string& curPath);
    
    template<typename Base, typename T>
    static bool InstanceOf(const T* ptr)
    {
        return dynamic_cast<const Base*>(ptr) != nullptr;
    }

    static std::string GetCurrentTimeFormatted(); 

    static std::string FormatLog(const std::string& msg, LogType type = Info);
    static void Log(const std::string& msg, LogType type = Info);
    static void Log(const char* val, LogType type);
    static void Log(int val, LogType type = Info);
    static void Log(unsigned int val, LogType type = Info);
    static void Log(long val, LogType type = Info);
    static void Log(unsigned long val, LogType type = Info);
    static void Log(long long val, LogType type = Info);
    static void Log(unsigned long long val, LogType type = Info);
    static void Log(float val, LogType type = Info);
    static void Log(double val, LogType type = Info);
    static void Log(long double val, LogType type = Info);
    static void Log(char val, LogType type = Info);
    static void Log(unsigned char val, LogType type = Info);
    static void Log(wchar_t val, LogType type = Info);
    static void Log(char16_t val, LogType type = Info);
    static void Log(char32_t val, LogType type = Info);
    static void Log(glm::vec3 val, LogType type = Info);
    static void Log(glm::vec4 val, LogType type = Info);
    static void Log(glm::mat4 val, LogType type = Info);
    static std::string ToString(bool val);
    static std::string ToString(int val);
    static std::string ToString(unsigned int val);
    static std::string ToString(long val);
    static std::string ToString(unsigned long val);
    static std::string ToString(long long val);
    static std::string ToString(unsigned long long val);
    static std::string ToString(float val);
    static std::string ToString(double val);
    static std::string ToString(long double val);
    static std::string ToString(char val);
    static std::string ToString(unsigned char val);
    static std::string ToString(wchar_t val);
    static std::string ToString(char16_t val);
    static std::string ToString(char32_t val);
    static std::string ToString(float val, int fixed);
    static std::string ToString(const glm::vec3& val);
    static std::string ToString(const glm::vec4& val);
    static std::string ToString(const glm::mat4& val);

    static bool IsVec(const nlohmann::json& jsonValue, size_t components);
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
    static int ComputeRegionCode(float x, float y, float xmin, float ymin, float xmax, float ymax);
    static bool CohenSutherlandClip(float &x1, float &y1, float &x2, float &y2, float xmin, float ymin, float xmax, float ymax);
};
