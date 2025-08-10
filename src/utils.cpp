#include "utils.h"

#include <boost/beast/core/detail/base64.hpp>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <fstream>

#include <tracy/Tracy.hpp>

#include "imgui.h"

#include "bounds.h"

namespace op
{
    namespace fs = std::filesystem;

    Event<GLFWwindow*, int, int> Utils::s_setFrameBufferSizeEvent;
    std::vector<std::string> Utils::s_logs;

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

    std::string Utils::GetAbsolutePath(const std::string& relativePath)
    {
        return (fs::current_path() / relativePath).generic_string();
    }

    bool Utils::AssetExists(const std::string& path)
    {
        auto absPath = GetAbsolutePath(path);
        return fs::exists(absPath);
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
        ZoneScoped;
        
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

    Matrix4x4 Utils::CreateProjection(const float fov, const float aspect, const float near, const float far)
    {
        auto f = 1 / std::tan(fov * 0.5f * DEG2RAD);
        auto a = (far) / (near - far);
        auto b = (far * near) / (near - far);

        return {
            f / aspect, 0, 0, 0,
            0, f, 0, 0,
            0, 0, a, b,
            0, 0, -1, 0
        };
    }

    Matrix4x4 Utils::CreateOrthoProjection(const float r, const float l, const float t, const float b, const float f, const float n)
    {
        return {
            2 / (r - l), 0, 0, (l + r) / (l - r),
            0, 2 / (t - b), 0, (t + b) / (b - t),
            0, 0, 1 / (f - n), n / (n - f),
            0, 0, 0, 1
        };
    }

    // 将世界坐标转换为屏幕坐标的工具函数
    Vec3 Utils::WorldToScreen(const Vec3& worldPos, const Matrix4x4& viewMatrix, const Matrix4x4& projMatrix, const Vec2& screenSize)
    {
        Vec4 clipSpacePos = projMatrix * viewMatrix * Vec4(worldPos, 1.0f);

        // 透视除法，转到 NDC（Normalized Device Coordinates）空间
        if (clipSpacePos.w != 0.0f) {
            clipSpacePos.x /= clipSpacePos.w;
            clipSpacePos.y /= clipSpacePos.w;
        }

        // 转换到屏幕坐标
        Vec3 screenPos;
        screenPos.x = (clipSpacePos.x * 0.5f + 0.5f) * screenSize.x; // NDC x [-1,1] -> screen x [0,width]
        screenPos.y = (1.0f - (clipSpacePos.y * 0.5f + 0.5f)) * screenSize.y; // NDC y [-1,1] -> screen y [height,0]
        screenPos.z = clipSpacePos.z;
        return screenPos;
    }

    // 封装的绘制线条函数
    void Utils::DebugDrawLine(const Vec3& worldStart, const Vec3& worldEnd, 
                       const Matrix4x4& viewMatrix, const Matrix4x4& projMatrix, 
                       const Vec2& screenSize, 
                       ImU32 color, float thickness)
    {
        auto drawList = ImGui::GetBackgroundDrawList();
        
        // 转换世界坐标到屏幕坐标
        auto screenStart = WorldToScreen(worldStart, viewMatrix, projMatrix, screenSize);
        auto screenEnd = WorldToScreen(worldEnd, viewMatrix, projMatrix, screenSize);

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
        const Matrix4x4& viewMatrix,
        const Matrix4x4& projMatrix,
        const Vec2& screenSize,
        ImU32 color,
        float thickness)
    {
        // 计算八个顶点
        std::array<Vec3, 8> vertices =
        {
            bounds.center + Vec3(-bounds.extents.x, -bounds.extents.y, -bounds.extents.z),
            bounds.center + Vec3( bounds.extents.x, -bounds.extents.y, -bounds.extents.z),
            bounds.center + Vec3( bounds.extents.x,  bounds.extents.y, -bounds.extents.z),
            bounds.center + Vec3(-bounds.extents.x,  bounds.extents.y, -bounds.extents.z),
            bounds.center + Vec3(-bounds.extents.x, -bounds.extents.y,  bounds.extents.z),
            bounds.center + Vec3( bounds.extents.x, -bounds.extents.y,  bounds.extents.z),
            bounds.center + Vec3( bounds.extents.x,  bounds.extents.y,  bounds.extents.z),
            bounds.center + Vec3(-bounds.extents.x,  bounds.extents.y,  bounds.extents.z)
        };

        // 定义12条边 [11]()
        const std::array<std::pair<int, int>, 12> edges =
        {{
            {0,1}, {1,2}, {2,3}, {3,0}, // 底面 
            {4,5}, {5,6}, {6,7}, {7,4}, // 顶面
            {0,4}, {1,5}, {2,6}, {3,7}  // 侧面连接 
        }};

        // 绘制所有边
        for (const auto& [start, end] : edges)
        {
            DebugDrawLine(
                vertices[start],
                vertices[end],
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

    nlohmann::json Utils::LoadJson(const std::string& assetPath)
    {
        auto s = std::ifstream(GetAbsolutePath(assetPath));
        nlohmann::json json;
        s >> json;
        s.close();

        return json;
    }

    void Utils::MergeJson(nlohmann::json& json1, const nlohmann::json& json2, bool combineArray)
    {
        for (auto& it : json2.items())
        {
            const auto& key = it.key();
            const auto& value = it.value();

            if (!json1.contains(key))
            {
                // 1里没这个key，就直接添加
                json1[key] = value;
                continue;
            }

            if (combineArray && value.type() == nlohmann::json::value_t::array && json1[key].type() == nlohmann::json::value_t::array)
            {
                // 是数组就将2的加在1的后面
                json1[key].insert(json1[key].end(), value.begin(), value.end());
                continue;
            }

            if (value.type() == nlohmann::json::value_t::object && json1[key].type() == nlohmann::json::value_t::object)
            {
                // 是dict就递归合并
                MergeJson(json1[key], value);
                continue;
            }

            json1[key] = value;
        }
    }

    nlohmann::json Utils::GetResourceMeta(const std::string& assetPath)
    {
        auto metaPath = assetPath + ".meta";
        if (!fs::exists(GetAbsolutePath(metaPath)))
        {
            return nlohmann::json::object();
        }

        return LoadJson(metaPath);
    }

    std::vector<uint8_t> Utils::Base64ToBinary(const std::string& base64Str)
    {
        std::size_t decodedSize = boost::beast::detail::base64::decoded_size(base64Str.size());
        std::vector<uint8_t> binaryData(decodedSize);
    
        auto result = boost::beast::detail::base64::decode(
            binaryData.data(), 
            base64Str.data(), 
            base64Str.size());
    
        binaryData.resize(result.first);
        return binaryData;
    }
    
    std::vector<uint32_t> Utils::Binary8To32(const std::vector<uint8_t>& data)
    {
        std::vector<uint32_t> result;
        result.resize(data.size() / 4);
        memcpy(result.data(), data.data(), data.size());
        return result;
    }
}
