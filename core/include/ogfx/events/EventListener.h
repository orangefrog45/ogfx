#pragma once

#include "ogfx/events/EventBase.h"
#include "ogfx/events/EventManager.h"

namespace ogfx {
    template<std::derived_from<Event> EventType>
    class EventListener : public EventListenerBase<EventType> {
    public:
        EventListener() = default;

        EventListener(const std::function<void(const EventType&)> _OnEvent, bool register_immediately = false) {
            this->OnEvent = _OnEvent;

            if (register_immediately)
                Register();
        }

        EventListener(const EventListener& other) = delete;
        EventListener& operator=(const EventListener& other) = delete;

        EventListener(EventListener&& other) noexcept {
            this->OnEvent = std::move(other.OnEvent);
            if (other.m_registered) {
                other.Deregister();
                Register();
            }
        }

        EventListener& operator=(EventListener&& other) noexcept {
            if (this != &other) {
                Deregister();
                this->OnEvent = std::move(other.OnEvent);
                if (other.m_registered) {
                    other.Deregister();
                    Register();
                }
            }
            return *this;
        }

        friend class EventManager;

        using EventT = EventType;

        ~EventListener() {
            Deregister();
        }

        void Register() {
            EventManager::Register(*this);
        }

        void Deregister() {
            EventManager::Deregister(*this);
        }
    };
}