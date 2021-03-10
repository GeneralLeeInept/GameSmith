#pragma once

#include "gamesmith/input/keyboard.h"

namespace gs
{

enum class EventType
{
    kApplicationSuspended,
    kApplicationResumed,
    kKeyPressed,
    kKeyReleased
};

struct Event
{
    EventType type;
};

struct KeyPressedEvent : public Event
{
    KeyCode code;
};

struct KeyReleasedEvent : public Event
{
    KeyCode code;
};

}