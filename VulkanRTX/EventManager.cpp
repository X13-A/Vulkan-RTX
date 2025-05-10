#include "EventManager.hpp"

entt::dispatcher& EventManager::get() 
{
    static entt::dispatcher instance;
    return instance;
}
