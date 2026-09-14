#pragma once
#include <functional>
#include <limits>
#include <memory>
#include <unordered_map>

#include "otl/String.h"
#include "ogfx/util/Log.h"

#define OGFX_EVENT_CLASS(name_str) static uint64_t GetId() { \
    static constexpr uint64_t id = otl::HashString(name_str); \
    return id; \
} \
    void Dispatch() {ogfx::EventManager::Dispatch(*this);}

namespace ogfx {
    struct Event {
        static void Dispatch() { OGFX_ASSERT(false, "Event subclasses must contain 'OGFX_EVENT_CLASS'."); }

        static uint64_t GetId() { OGFX_ASSERT(false, "Event subclasses must contain 'OGFX_EVENT_CLASS'."); return 0; }
    };

    template<std::derived_from<Event> EventType>
    class EventListenerBase {
    public:
        friend class EventManager;
        using EventT = EventType;

        std::function<void(EventType&)> OnEvent = nullptr;
    protected:
        inline static constexpr uint64_t INVALID_ID = std::numeric_limits<uint64_t>::max();

        std::unordered_multimap<uint64_t, EventListenerBase<Event>>::iterator m_reg;
        bool m_registered = false;
    };
}
