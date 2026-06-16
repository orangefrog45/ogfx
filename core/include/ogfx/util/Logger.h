#pragma once

#include <deque>
#include <string>
#include <format>
#include <fstream>
#include <iostream>
#include <stacktrace>

namespace ogfx {
    enum class LogType {
        L_TRACE,
        L_INFO,
        L_WARN,
        L_ERROR,
        L_CRITICAL
    };

    class Logger {
    public:
        static void Init();

        template<typename... Args>
        static void Log(LogType type, std::string_view fmt, Args&&... args) {
            std::string msg = std::vformat(fmt, std::make_format_args(args...));
            Log(type, msg);
        }

        template<typename T>
        static void Log(LogType type, T s) {
            std::string str = GetLogFormatStr(type) + std::format("{}", s);

            // Everything is flushed immediately for crash safety
            std::cout << GetLogColour(type) << str << "\033[0m\n" << std::flush;
            m_log_file << str << "\n";

            m_logs.emplace_back(type, str);

            if (m_logs.size() > 500)
                m_logs.pop_front();

            m_log_file.flush();
        }

        struct Entry {
            LogType type;
            std::string log;
        };
    private:
        static std::string GetLogFormatStr(LogType type);
        static const char* GetLogColour(LogType type);

        inline static std::deque<Entry> m_logs;
        inline static std::ofstream m_log_file;
    };

}

#define OGFX_BREAKPOINT __debugbreak()
#define OGFX_DBG_BREAKPOINT __debugbreak()

#define OGFX_CORE_TRACE(...) do {ogfx::Logger::Log(ogfx::LogType::L_TRACE, __VA_ARGS__); } while(false)
#define OGFX_CORE_INFO(...) do {ogfx::Logger::Log(ogfx::LogType::L_INFO, __VA_ARGS__); } while(false)
#define OGFX_CORE_WARN(...) do {ogfx::Logger::Log(ogfx::LogType::L_WARN, __VA_ARGS__); } while(false)
#define OGFX_CORE_ERROR(...) do {ogfx::Logger::Log(ogfx::LogType::L_ERROR, __VA_ARGS__); } while(false)
#define OGFX_CORE_CRITICAL(...) do {ogfx::Logger::Log(ogfx::LogType::L_CRITICAL, __VA_ARGS__); ogfx::Logger::Log(ogfx::LogType::L_CRITICAL, "Stacktrace:\n{}", std::to_string(std::stacktrace::current())); OGFX_BREAKPOINT; } while(false)

#define OGFX_ASSERT(x) do { if (!(x)) OGFX_CORE_CRITICAL("Assertion failed: '{}'", #x); } while(false)
#define OGFX_DBG_ASSERT(x) do { if (!(x)) OGFX_CORE_CRITICAL("Debug assertion failed: '{}'", #x); } while(false)
#define OGFX_ASSERT_STR(x, str) do { if (!(x)) OGFX_CORE_CRITICAL("Assertion failed: '{}' - {}", #x, str); } while(false)
#define OGFX_DBG_ASSERT_STR(x, str) do { if (!(x)) OGFX_CORE_CRITICAL("Debug assertion failed: '{}' - {}", #x, str); } while(false)