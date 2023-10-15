#pragma once

#include <SFML/Graphics.hpp>

#include <sftools/Chronometer.hpp>

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
    void drawPerks(int baseY);
    void drawGlobalTimer();
    void drawBullets(int baseY);
    void drawArmor(int baseY);

    void drawWinScreen();
    void drawFailScreen();

    bool _showWin = false;
    bool _showFail = false;

    std::string _surviveTimeStr;

    sftools::Chronometer _blinkClock;
    sftools::Chronometer _baseDamageClock;
    bool _pause = false;
    void checkForGamePause();
public:
    static HUD& instance();

    void draw();
    void showWin(bool val);
    void showFail(bool val);

    void onBaseDamaged();
};