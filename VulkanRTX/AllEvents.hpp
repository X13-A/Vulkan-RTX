#pragma once
#include "entt.hpp"

struct MouseMoveEvent 
{
    double x, y;
};

struct MouseScrollEvent
{
    double x, y;
};

struct WindowResizeEvent
{
    int width, height;
};