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
    void drawPlayerUpgrades();

    void drawMiniIcon(const sf::IntRect& iconRect, int x, int y);
    void drawIcon(const sf::IntRect& iconRect, int x, int y);
    void drawUpgrade(int index, int x, int y);
public:
    static HUD& instance();

    void draw();
    void drawLevelUpPopupMenu(int pos);

};