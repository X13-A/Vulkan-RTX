#pragma once;
#include "entt.hpp"

class EventManager 
{
public:
    static entt::dispatcher& get();
};
