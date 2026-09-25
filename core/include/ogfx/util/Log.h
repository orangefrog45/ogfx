#pragma once

#include "otl/Log.h"

namespace ogfx {
    namespace log {
        using namespace otl;

        template <typename... Args>
        static void Trace(const std::string& fmt, Args&&... args) {
            Log::LogStr(LogType::Trace, "[OGFX] " + fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        static void Info(const std::string& fmt, Args&&... args) {
            Log::LogStr(LogType::Info, "[OGFX] " + fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        static void Warn(const std::string& fmt, Args&&... args) {
            Log::LogStr(LogType::Warn, "[OGFX] " + fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        static void Error(const std::string& fmt, Args&&... args) {
            Log::LogStr(LogType::Error, "[OGFX] " + fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        static void Critical(const std::string& fmt, Args&&... args) {
            Log::LogStr(LogType::Critical, "[OGFX] " + fmt, std::forward<Args>(args)...);
        }
    }
}

#define OGFX_BREAKPOINT() OTL_BREAKPOINT()
#define OGFX_DBG_BREAKPOINT() OTL_DBG_BREAKPOINT()

#define OGFX_ASSERT(x, ...) do { \
if (!(x)) { \
ogfx::log::Critical("Assertion failed: '{}'" __VA_OPT__(" - ") __VA_ARGS__, #x); \
OGFX_BREAKPOINT(); \
} \
} while(false)

#ifdef OGFX_DEBUG
#define OGFX_DBG_ASSERT(x, ...) OGFX_ASSERT(x, __VA_ARGS__)
#else
#define OGFX_DBG_ASSERT(x, ...)
#endif
