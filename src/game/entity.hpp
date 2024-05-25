#pragma once

#include "transform.h"

class Entity
{
public:
    Entity();
    ~Entity();

    void draw();

public:
    Transform transform;
};

Entity::Entity()
{
}
