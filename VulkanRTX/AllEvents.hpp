#pragma once
#include "entt.hpp"

struct MouseMoveEvent 
{
    double xPos, yPos;
};

struct MouseOffsetEvent
{
    double xOffset, yOffset;
};

struct MouseScrollEvent
{
    double xOffset, yOffset;
};

struct WindowResizeEvent
{
    int width, height;
};