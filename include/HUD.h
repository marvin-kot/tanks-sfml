#pragma once

#include <SFML/Graphics.hpp>


class HUD
{
    HUD();
    HUD(HUD &) = delete;
    void operator=(const HUD&) = delete;

    void drawPlayerXP(int baseY);
    void drawTankLives(int baseY);
    void drawTankUpgrades(int baseY);
    void drawBaseLives(int baseY);
    void drawBaseUpgrades(int baseY);
    void drawGlobalTimer();

    void drawMiniIcon(const sf::IntRect& iconRect, int x, int y);
    void drawIcon(const sf::IntRect& iconRect, int x, int y);
    void drawUpgrade(int index, int x, int y);
public:
    static HUD& instance();

    void draw();
    void drawLevelUpPopupMenu(int pos);

};