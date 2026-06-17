#pragma once

#include <chrono>

#pragma once

namespace ogfx {
    class Timer {
    public:
        Timer() {
            Reset();
        }

        double Reset() {
            auto elapsed = ElapsedMilliseconds();
            m_start = std::chrono::steady_clock::now();
            return elapsed;
        }

        double ElapsedSeconds() const {
            return std::chrono::duration<double>(
                std::chrono::steady_clock::now() - m_start
            ).count();
        }

        double ElapsedMilliseconds() const {
            return std::chrono::duration<double, std::milli>(
                std::chrono::steady_clock::now() - m_start
            ).count();
        }
    private:
        std::chrono::steady_clock::time_point m_start;
    };
}
