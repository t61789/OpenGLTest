#include "Utils.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vec4.hpp>
#include "glm.hpp"

Event<GLFWwindow*, int, int> Utils::s_setFrameBufferSizeEvent;

Bounds::Bounds() = default;

Bounds::Bounds(const glm::vec3 center, const glm::vec3 extents)
{
    this->center = center;
    this->extents = extents;
}

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

std::string Utils::FormatLog(const std::string& msg, const LogType type)
{
    std::stringstream ss;
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

void Utils::Log(const std::string& msg, const LogType type)
{
    std::cout << FormatLog(msg, type) << '\n';
}

void Utils::Log(const bool val, const LogType type)
{
    Log(ToString(val), type);
}

void Utils::Log(const int val, const LogType type)
{
    Log(ToString(val), type);
}

void Utils::Log(const unsigned int val, const LogType type)
{
    Log(ToString(val), type);
}

void Utils::Log(const long val, const LogType type)
{
    Log(ToString(val), type);
}

void Utils::Log(const unsigned long val, const LogType type)
{
    Log(ToString(val), type);
}

void Utils::Log(const long long val, const LogType type)
{
    Log(ToString(val), type);
}

void Utils::Log(const unsigned long long val, const LogType type)
{
    Log(ToString(val), type);
}

void Utils::Log(const float val, const LogType type)
{
    Log(ToString(val), type);
}

void Utils::Log(const double val, const LogType type)
{
    Log(ToString(val), type);
}

void Utils::Log(const long double val, const LogType type)
{
    Log(ToString(val), type);
}

void Utils::Log(const char val, const LogType type)
{
    Log(std::string(1, val), type);
}

void Utils::Log(const unsigned char val, const LogType type)
{
    Log(ToString(val), type);
}

void Utils::Log(const wchar_t val, const LogType type)
{
    Log(ToString(val), type);
}

void Utils::Log(const char16_t val, const LogType type)
{
    Log(ToString(val), type);
}

void Utils::Log(const char32_t val, const LogType type)
{
    Log(ToString(val), type);
}

void Utils::Log(const glm::vec3 val, const LogType type)
{
    Log(ToString(val), type);
}

void Utils::Log(const glm::vec4 val, const LogType type)
{
    Log(ToString(val), type);
}

void Utils::Log(const glm::mat4 val, const LogType type)
{
    Log(ToString(val), type);
}

std::string Utils::ToString(const bool val)
{
    return val ? "true" : "false";
}

std::string Utils::ToString(const int val)
{
    return std::to_string(val);
}

std::string Utils::ToString(const unsigned int val)
{
    return std::to_string(val);
}

std::string Utils::ToString(const long val)
{
    return std::to_string(val);
}

std::string Utils::ToString(const unsigned long val)
{
    return std::to_string(val);
}

std::string Utils::ToString(const long long val)
{
    return std::to_string(val);
}

std::string Utils::ToString(const unsigned long long val)
{
    return std::to_string(val);
}

std::string Utils::ToString(const float val)
{
    return ToString(val, 4);
}

std::string Utils::ToString(const double val)
{
    return std::to_string(val);
}

std::string Utils::ToString(const long double val)
{
    return std::to_string(val);
}

std::string Utils::ToString(const char val)
{
    return std::to_string(val);
}

std::string Utils::ToString(const unsigned char val)
{
    return std::to_string(val);
}

std::string Utils::ToString(const wchar_t val)
{
    return std::to_string(val);
}

std::string Utils::ToString(const char16_t val)
{
    return std::to_string(val);
}

std::string Utils::ToString(const char32_t val)
{
    return std::to_string(val);
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

bool Utils::IsVec(const nlohmann::json& jsonValue, const size_t components)
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
            ss << std::hex << error;
        }
        throw std::runtime_error(ss.str());
    }
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
