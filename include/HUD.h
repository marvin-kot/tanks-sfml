#pragma once

#include <SFML/Graphics.hpp>


class HUD
{
    sf::Sprite _sprite;

    HUD();
    HUD(HUD &) = delete;
    void operator=(const HUD&) = delete;

public:
    static HUD& instance();

    void drawPlayerLives();
};