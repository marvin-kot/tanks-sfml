#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Clock.hpp>

namespace globalConst
{
    constexpr int screen_w = 1920;
    constexpr int screen_h = 1080;

    constexpr int spriteOriginalSizeX = 16;
    constexpr int spriteOriginalSizeY = 16;

    constexpr int spriteScaleX = 6;
    constexpr int spriteScaleY = 6;

    constexpr int spriteDisplaySizeX = spriteOriginalSizeX * spriteScaleX;
    constexpr int spriteDisplaySizeY = spriteOriginalSizeY * spriteScaleY;

    constexpr int viewPortWidthTiles = screen_w / spriteDisplaySizeX - 1;
    constexpr int viewPortHeightTiles = screen_h / spriteDisplaySizeY - 1;

    constexpr int maxFieldWidth = 64;
    constexpr int maxFieldHeight = 64;

    constexpr int viewPortWidthPx = viewPortWidthTiles * spriteDisplaySizeX;
    constexpr int viewPortHeightPx = viewPortHeightTiles * spriteDisplaySizeY;

    constexpr int DefaultPlayerSpeed = 64;
    constexpr int DefaultEnemySpeed = DefaultPlayerSpeed * 3 / 4;

    constexpr int DefaultBulletSpeed = DefaultPlayerSpeed * 2;
    constexpr int DoubleBulletSpeed = DefaultBulletSpeed * 3 / 2;
    constexpr int DefaultDamage = 1;
    constexpr int DoubleDamage = 2;
    constexpr int DefaultTimeoutMs = 1000;
    constexpr int HalvedTimeoutMs = 500;
    constexpr int PlayerShootTimeoutMs = 100;

    constexpr int InitialLives = 3;
    constexpr int InitialPowerLevel = 0;

    constexpr int MaxFramesToDie = 180;
    constexpr int MaxFramesToWin = 200;

};

namespace globalVars
{
    extern int borderWidth;
    extern int borderHeight;

    extern sf::IntRect gameViewPort;
    extern sf::IntRect mapViewPort;

    extern sf::Vector2i mapSize;

    extern bool globalTimeFreeze;
    extern sf::Clock globalFreezeClock;

    extern int player1Lives;
    extern int player1PowerLevel;
};