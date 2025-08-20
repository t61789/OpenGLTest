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
