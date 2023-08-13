#pragma once

#include <SFML/Graphics.hpp>


class HUD
{
    sf::Sprite _sprite;

    HUD();
    HUD(HUD &) = delete;
    void operator=(const HUD&) = delete;

    void drawPlayerLives();
    void drawPlayerXP();
public:
    static HUD& instance();

    void draw();
    void drawLevelUpPopupMenu(int pos);

};