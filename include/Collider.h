#pragma once

#include <SFML/Graphics.hpp>

class Collider
{
    sf::IntRect rect;
public:
    Collider(const sf::IntRect& r) : rect(r)
    {}

    bool collides(const Collider& other) const {
        return rect.intersects(other.rect);
    }
};