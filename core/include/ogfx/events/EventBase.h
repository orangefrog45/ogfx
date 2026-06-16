#pragma once
#include <functional>
#include <limits>
#include <memory>
#include <unordered_map>

#include "util/Logger.h"

#define OGFX_EVENT_CLASS(id) static uint64_t GetId() {return id;}\
    void Dispatch() override {EventManager::Dispatch(*this);}

namespace ogfx {
    struct Event {
        virtual ~Event() {}

        virtual void Dispatch() = 0;

        static uint64_t GetId() {OGFX_ASSERT_STR(false, "Event subclasses must implement 'GetId'."); return 0;}
    };

    template<std::derived_from<Event> EventType>
    class EventListenerBase {
    public:
        friend class EventManager;
        using EventT = EventType;

        std::function<void(const EventType&)> OnEvent = nullptr;
    protected:
        inline static constexpr uint64_t INVALID_ID = std::numeric_limits<uint64_t>::max();

        std::unordered_multimap<uint64_t, EventListenerBase<Event>>::iterator m_reg;
        bool m_registered = false;
    };
}
