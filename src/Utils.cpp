#include "Utils.h"

#include <chrono>
#include <iomanip>
#include <sstream>

#include "glm/glm.hpp"
#include "imgui.h"

Event<GLFWwindow*, int, int> Utils::s_setFrameBufferSizeEvent;
std::vector<std::string> Utils::s_logs;

glm::vec3 Utils::ToVec3(nlohmann::json arr)
{
    if(!IsVec3(arr))
    {
        return {};
    }
    
    return {
        arr[0].get<float>(),
        arr[1].get<float>(),
        arr[2].get<float>()
    };
}

glm::vec4 Utils::ToVec4(nlohmann::json arr)
{
    if(!IsVec4(arr))
    {
        return {};
    }
    
    return {
        arr[0].get<float>(),
        arr[1].get<float>(),
        arr[2].get<float>(),
        arr[3].get<float>()
    };
}

std::vector<std::string> Utils::ToDirectories(const std::string& path)
{
    std::string curPath = path;
    if(!curPath.empty() && curPath[0] == '/')
    {
        curPath = curPath.substr(1);
    }
    if(!curPath.empty() && curPath[curPath.size() - 1] == '/')
    {
        curPath = curPath.substr(0, curPath.size() - 1);
    }

    std::vector<std::string> directories;
    std::string directory;
    std::stringstream ss(curPath);
    while(std::getline(ss, directory, '/'))
    {
        directories.push_back(directory);
    }

    return directories;
}

std::string Utils::GetRealAssetPath(const std::string& relativePath)
{
    return std::string("../assets/") + relativePath;
}

std::string Utils::GetRealAssetPath(const std::string& relativePath, const std::string& curPath)
{
    auto relativePathDirectories = ToDirectories(relativePath);
    auto curPathDirectories = ToDirectories(curPath);

    if(relativePathDirectories.empty() || curPathDirectories.empty())
    {
        return GetRealAssetPath(relativePath);
    }

    if(relativePathDirectories[0] != "." && relativePathDirectories[0] != "..")
    {
        return GetRealAssetPath(relativePath);
    }

    // 如果当前路径是文件，则把最后的文件名去掉
    if(curPathDirectories[curPathDirectories.size() - 1] != "." &&
        curPathDirectories[curPathDirectories.size() - 1].find(".") != std::string::npos)
    {
        curPathDirectories.pop_back();
    }

    for (const auto& dir : relativePathDirectories)
    {
        if(dir == "..")
        {
            if(curPathDirectories.empty())
            {
                return "pathError";
            }
            curPathDirectories.pop_back();
        }
        else if(dir == ".")
        {
            // do nothing
        }
        else
        {
            curPathDirectories.push_back(dir);
        }
    }

    return GetRealAssetPath(JoinStrings(curPathDirectories, "/"));
}

std::string Utils::GetCurrentTimeFormatted()
{
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();

    // 转换为 time_t 格式
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

    // 提取毫秒部分
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    // 使用 localtime_s 提供线程安全的本地时间
    std::tm now_tm;
    localtime_s(&now_tm, &now_time_t); // 安全的本地时间转换

    // 使用字符串流格式化时间
    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%H:%M:%S") // 格式化小时、分钟、秒
        << "." << std::setfill('0') << std::setw(3) << now_ms.count(); // 添加毫秒部分

    return oss.str();
}

std::string Utils::FormatLog(const LogType type, const std::string& msg)
{
    std::stringstream ss;
    ss << "[";
    ss << GetCurrentTimeFormatted();
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

    ss << msg;
    return ss.str();
}

std::string Utils::ToString(const float val, const int fixed)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(fixed) << val;
    return oss.str();
}

std::string Utils::ToString(const glm::vec3& val)
{
    return "(" + ToString(val.x, 2) + ", " + ToString(val.y, 2) + ", " + ToString(val.z, 2) + ")";   
}

std::string Utils::ToString(const glm::vec4& val)
{
    return "(" + ToString(val.x, 2) + ", " + ToString(val.y, 2) + ", " + ToString(val.z, 2) + ", " + ToString(val.w, 2) + ")";   
}

std::string Utils::ToString(const glm::mat4& val)
{
    return "\t|" + ToString(val[0][0], 2) + ", " + ToString(val[1][0], 2) + ", " + ToString(val[2][0], 2) + ", " + ToString(val[3][0], 2) + "|\n" +
           "\t|" + ToString(val[0][1], 2) + ", " + ToString(val[1][1], 2) + ", " + ToString(val[2][1], 2) + ", " + ToString(val[3][1], 2) + "|\n" +
           "\t|" + ToString(val[0][2], 2) + ", " + ToString(val[1][2], 2) + ", " + ToString(val[2][2], 2) + ", " + ToString(val[3][2], 2) + "|\n" +
           "\t|" + ToString(val[0][3], 2) + ", " + ToString(val[1][3], 2) + ", " + ToString(val[2][3], 2) + ", " + ToString(val[3][3], 2) + "|";
}

float* Utils::ToArr(const glm::vec3& val)
{
    auto arr = new float[3];
    arr[0] = val.x;
    arr[1] = val.y;
    arr[2] = val.z;

    return arr;
}

glm::vec3 Utils::FromArr(const float* arr)
{
    return {arr[0], arr[1], arr[2]};
}

bool Utils::IsVec(const nlohmann::json& jsonValue, const int components)
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

bool Utils::IsVec3(const nlohmann::json& jsonValue)
{
    return IsVec(jsonValue, 3);
}

bool Utils::IsVec4(const nlohmann::json& jsonValue)
{
    return IsVec(jsonValue, 4);
}

void Utils::ClearGlError()
{
    GLuint error;
    while((error = glGetError()) != GL_NO_ERROR)
    {
    }
}

void Utils::CheckGlError(const std::string& position)
{
    std::vector<int> errors;
    GLenum error;
    while((error = glGetError()) != GL_NO_ERROR)
    {
        errors.push_back(error);
    }

    if(!errors.empty())
    {
        std::stringstream ss;
        ss << "在 [" << position << "] 位置检测到以下GL错误：";
        for (int i = 0; i < errors.size(); ++i)
        {
            if(i != 0)
            {
                ss << ", ";
            }
            ss << std::hex << errors.back();
            errors.pop_back();
        }
        throw std::runtime_error(ss.str());
    }
}

void Utils::BeginDebugGroup(const std::string& groupName)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, groupName.c_str());
}

void Utils::EndDebugGroup()
{
    glPopDebugGroup();
}

bool Utils::EndsWith(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() && str.rfind(suffix) == str.size() - suffix.size();
}

std::string Utils::JoinStrings(const std::vector<std::string>& strings, std::string delimiter)
{
    std::stringstream ss;
    for (int i = 0; i < strings.size(); ++i)
    {
        ss << strings[i];
        if (i != strings.size() - 1)
        {
            ss << delimiter;
        }
    }
    return ss.str();
}

// 将世界坐标转换为屏幕坐标的工具函数
glm::vec3 Utils::WorldToScreen(const glm::vec3& worldPos, const glm::mat4& viewMatrix, const glm::mat4& projMatrix, const glm::vec2& screenSize) {
    glm::vec4 clipSpacePos = projMatrix * viewMatrix * glm::vec4(worldPos, 1.0f);

    // 透视除法，转到 NDC（Normalized Device Coordinates）空间
    if (clipSpacePos.w != 0.0f) {
        clipSpacePos.x /= clipSpacePos.w;
        clipSpacePos.y /= clipSpacePos.w;
    }

    // 转换到屏幕坐标
    glm::vec3 screenPos;
    screenPos.x = (clipSpacePos.x * 0.5f + 0.5f) * screenSize.x; // NDC x [-1,1] -> screen x [0,width]
    screenPos.y = (1.0f - (clipSpacePos.y * 0.5f + 0.5f)) * screenSize.y; // NDC y [-1,1] -> screen y [height,0]
    screenPos.z = clipSpacePos.z;
    return screenPos;
}

// 封装的绘制线条函数
void Utils::DebugDrawLine(const glm::vec3& worldStart, const glm::vec3& worldEnd, 
                   const glm::mat4& viewMatrix, const glm::mat4& projMatrix, 
                   const glm::vec2& screenSize, 
                   ImU32 color, float thickness) {
    
    auto drawList = ImGui::GetBackgroundDrawList();
    
    // 转换世界坐标到屏幕坐标
    glm::vec3 screenStart = WorldToScreen(worldStart, viewMatrix, projMatrix, screenSize);
    glm::vec3 screenEnd = WorldToScreen(worldEnd, viewMatrix, projMatrix, screenSize);

    if(screenStart.z < 0 || screenEnd.z < 0)
    {
        return;
    }

    float x0 = screenStart.x, y0 = screenStart.y, x1 = screenEnd.x, y1 = screenEnd.y;

    auto inScreen = CohenSutherlandClip(x0, y0, x1, y1, 0, 0, screenSize.x, screenSize.y);
    if(!inScreen)
    {
        return;
    }

    // 使用 ImGui 的绘图 API 绘制线条
    drawList->AddLine(ImVec2(x0, y0), ImVec2(x1, y1), color, thickness);
}

void Utils::DebugDrawCube(
    const Bounds& bounds,
    const glm::mat4& viewMatrix,
    const glm::mat4& projMatrix,
    const glm::vec2& screenSize,
    ImU32 color,
    float thickness)
{
    // 计算八个顶点
    std::array<glm::vec3, 8> vertices =
    {
        bounds.center + glm::vec3(-bounds.extents.x, -bounds.extents.y, -bounds.extents.z),
        bounds.center + glm::vec3( bounds.extents.x, -bounds.extents.y, -bounds.extents.z),
        bounds.center + glm::vec3( bounds.extents.x,  bounds.extents.y, -bounds.extents.z),
        bounds.center + glm::vec3(-bounds.extents.x,  bounds.extents.y, -bounds.extents.z),
        bounds.center + glm::vec3(-bounds.extents.x, -bounds.extents.y,  bounds.extents.z),
        bounds.center + glm::vec3( bounds.extents.x, -bounds.extents.y,  bounds.extents.z),
        bounds.center + glm::vec3( bounds.extents.x,  bounds.extents.y,  bounds.extents.z),
        bounds.center + glm::vec3(-bounds.extents.x,  bounds.extents.y,  bounds.extents.z)
    };

    // 定义12条边 [11]()
    const std::array<std::pair<int, int>, 12> edges =
    {{
        {0,1}, {1,2}, {2,3}, {3,0}, // 底面 
        {4,5}, {5,6}, {6,7}, {7,4}, // 顶面
        {0,4}, {1,5}, {2,6}, {3,7}  // 侧面连接 
    }};

    // 绘制所有边
    for (const auto& edge : edges)
    {
        DebugDrawLine(
            vertices[edge.first],
            vertices[edge.second],
            viewMatrix,
            projMatrix,
            screenSize,
            color,
            thickness);
    }
}

// 定义区域码（Cohen-Sutherland算法）
constexpr int INSIDE = 0; // 0000
constexpr int LEFT = 1;   // 0001
constexpr int RIGHT = 2;  // 0010
constexpr int BOTTOM = 4; // 0100
constexpr int TOP = 8;    // 1000

// 计算点的区域码
int Utils::ComputeRegionCode(float x, float y, float xmin, float ymin, float xmax, float ymax)
{
    int code = INSIDE;

    if (x < xmin) code |= LEFT;    // 左侧
    else if (x > xmax) code |= RIGHT; // 右侧
    if (y < ymin) code |= BOTTOM; // 下侧
    else if (y > ymax) code |= TOP;    // 上侧

    return code;
}

// Cohen-Sutherland线段裁剪算法
bool Utils::CohenSutherlandClip(float &x1, float &y1, float &x2, float &y2, 
                         float xmin, float ymin, float xmax, float ymax)
{
    int code1 = ComputeRegionCode(x1, y1, xmin, ymin, xmax, ymax);
    int code2 = ComputeRegionCode(x2, y2, xmin, ymin, xmax, ymax);

    bool accept = false;

    while (true) {
        if (code1 == 0 && code2 == 0)
        {
            // 两个点都在矩形内
            accept = true;
            break;
        }
        
        if (code1 & code2)
        {
            // 两个点在矩形外同一侧
            break;
        }
        
        // 线段至少部分在矩形内
        int codeOut;
        float x = 0, y = 0;

        // 选择一个在矩形外的点
        if (code1 != 0)
        {
            codeOut = code1;
        }
        else
        {
            codeOut = code2;
        }

        // 找到交点
        if (codeOut & TOP)
        { // 上侧
            x = x1 + (x2 - x1) * (ymax - y1) / (y2 - y1);
            y = ymax;
        }
        else if (codeOut & BOTTOM)
        { // 下侧
            x = x1 + (x2 - x1) * (ymin - y1) / (y2 - y1);
            y = ymin;
        }
        else if (codeOut & RIGHT)
        { // 右侧
            y = y1 + (y2 - y1) * (xmax - x1) / (x2 - x1);
            x = xmax;
        }
        else if (codeOut & LEFT)
        { // 左侧
            y = y1 + (y2 - y1) * (xmin - x1) / (x2 - x1);
            x = xmin;
        }

        // 更新点的位置，并重新计算区域码
        if (codeOut == code1)
        {
            x1 = x;
            y1 = y;
            code1 = ComputeRegionCode(x1, y1, xmin, ymin, xmax, ymax);
        }
        else
        {
            x2 = x;
            y2 = y;
            code2 = ComputeRegionCode(x2, y2, xmin, ymin, xmax, ymax);
        }
    }

    return accept;
}
