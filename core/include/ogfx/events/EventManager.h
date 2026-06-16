#pragma once

#include <unordered_map>
#include <functional>
#include <mutex>

#include "ogfx/events/EventBase.h"
#include "ogfx/util/Logger.h"

namespace ogfx {
    using TypeId = uint64_t;

    class EventManager {
    public:
        EventManager(EventManager* _p_instance) {
            mp_instance = _p_instance;
        }

        static EventManager& Get() {
            static std::mutex creation_mux;

            {
                std::scoped_lock l{creation_mux};
                if (!mp_instance) mp_instance = new EventManager();
            }

            return *mp_instance;
        }

        template<std::derived_from<Event> EventT>
        static void Dispatch(const EventT& _event) {
            uint64_t id = EventT::GetId();

            std::scoped_lock l{Get().m_mux};
            auto [first, last] = Get().m_listeners.equal_range(id);
            for (auto it = first; it != last; ++it) {
                auto& listener = it->second;
                reinterpret_cast<EventListenerBase<EventT>*>(&listener)->OnEvent(_event);
            }
        }

        template<typename ListenerT>
        static void Register(ListenerT& listener) {
            if (listener.m_registered) {
                OGFX_CORE_WARN("Tried to register an event listener more than once.");
                return;
            }

            uint64_t event_type_id = ListenerT::EventT::GetId();
            listener.m_registered = true;

            std::scoped_lock l{Get().m_mux};
            listener.m_reg = Get().m_listeners.emplace(event_type_id, *reinterpret_cast<EventListenerBase<Event>*>(&listener));
        }

        template<typename ListenerT>
        static void Deregister(ListenerT& listener) {
            if (!listener.m_registered) return;

            listener.m_registered = false;
            std::scoped_lock l{Get().m_mux};
            Get().m_listeners.erase(listener.m_reg);
        }

    private:
        EventManager() = default;

        inline static EventManager* mp_instance = nullptr;

        std::unordered_multimap<TypeId, EventListenerBase<Event>> m_listeners{};

        std::mutex m_mux{};
        std::atomic_uint64_t m_id = 0;
    };
}
