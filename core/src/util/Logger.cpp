#include "ogfx/pch.h"
#include <sstream>
#include <chrono>

#include "ogfx/util/Logger.h"

using namespace ogfx;

void Logger::Init() {
    m_log_file = std::ofstream{"ogfx-log.txt", std::ios::trunc};
}

inline std::string GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::ostringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%H:%M:%S");
    return ss.str();
}

const char* Logger::GetLogColour(LogType type) {
    switch(type) {
        case LogType::L_TRACE:    return "\033[0m"; // default
        case LogType::L_INFO:     return "\033[92m"; // green
        case LogType::L_WARN:     return "\033[93m"; // yellow
        case LogType::L_ERROR:    return "\033[91m"; // red
        case LogType::L_CRITICAL: return "\033[41;97m"; // red background, white text
        default:                  return "\033[0m";  // reset
    }
}

std::string Logger::GetLogFormatStr(LogType type) {
    std::stringstream ss;
    ss << '[' << GetTimestamp() << "][CORE]";

    switch (type) {
        case LogType::L_TRACE:
            ss << "[TRC]";
            break;
        case LogType::L_INFO:
            ss << "[INF]";
            break;
        case LogType::L_WARN:
            ss << "[WRN]";
            break;
        case LogType::L_ERROR:
            ss << "[ERR]";
            break;
        case LogType::L_CRITICAL:
            ss << "[CRT]";
            break;
    }

    ss << " ";

    return ss.str();
}