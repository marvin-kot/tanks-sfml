#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Clock.hpp>
#include <sftools/Chronometer.hpp>

namespace globalConst
{
    constexpr int GameMajorVersion = 0;
    constexpr int GameMinorVersion = 3;
    constexpr int GameReleaseVersion = 1;

    constexpr int screen_w = 1920;
    constexpr int screen_h = 1080;

    constexpr int FixedFrameRate = 30;
    constexpr float FixedFrameLength = 1 / (float)FixedFrameRate;

    constexpr int spriteOriginalSizeX = 32;
    constexpr int spriteOriginalSizeY = 32;

    constexpr int spriteScaleX = 3;
    constexpr int spriteScaleY = 3;

    constexpr int spriteDisplaySizeX = spriteOriginalSizeX * spriteScaleX;
    constexpr int spriteDisplaySizeY = spriteOriginalSizeY * spriteScaleY;

    constexpr int viewPortWidthTiles = screen_w / spriteDisplaySizeX - 1;
    constexpr int viewPortHeightTiles = screen_h / spriteDisplaySizeY - 1;

    constexpr int maxFieldWidth = 64;
    constexpr int maxFieldHeight = 64;

    constexpr int viewPortWidthPx = viewPortWidthTiles * spriteDisplaySizeX;
    constexpr int viewPortHeightPx = viewPortHeightTiles * spriteDisplaySizeY;
    constexpr float SpeedBase = 3;
    constexpr int DefaultPlayerSpeed = static_cast<int>(SpeedBase * spriteOriginalSizeX);
    constexpr int DefaultEnemySpeed = DefaultPlayerSpeed * 4 / 7;
    constexpr int FastEnemySpeed = DefaultEnemySpeed * 4 / 3;
    constexpr int SlowEnemySpeed = DefaultEnemySpeed * 2 / 3;

    constexpr int DefaultPlayerProtection = 1;
    constexpr int DefaultBaseProtection = 1;
    constexpr int DefaultPlayerBulletSpeed = DefaultPlayerSpeed * 7 / 3;
    constexpr int DoublePlayerBulletSpeed = DefaultPlayerBulletSpeed * 3 / 2;
    constexpr int DefaultEnemyBulletSpeed = DefaultEnemySpeed * 7 / 3;
    constexpr int DoubleEnemyBulletSpeed = DefaultEnemyBulletSpeed * 3 / 2;
    constexpr int DefaultDamage = 1;
    constexpr int DoubleDamage = 2;
    constexpr int DefaultTimeoutMs = 1000;
    constexpr int HalvedTimeoutMs = 500;
    constexpr int PlayerShootTimeoutMs = 250;
    constexpr int PlayerDefaultReloadTimeoutMs = 800;
    constexpr int PlayerDefaultMaxBullets = 4;
    constexpr int EnemyShootTimeoutMs = 500;
    constexpr int EnemyDefaultReloadTimeoutMs = 1000;
    constexpr int EnemyDefaultMaxBullets = 1;
    constexpr int RocketShootTimeoutMs = 500;
    constexpr int DefaultRocketSpeed = 170;

    constexpr int MinDamageTimeoutMs = 100;

    constexpr int DefaultBulletLifetimeMs = 2000;

    constexpr int InitialLives = 3;
    constexpr int InitialPowerLevel = 0;

    constexpr int MaxFramesToDie = 180;
    constexpr int MaxFramesToWin = 200;

    constexpr int MaxUpgradeLevel = 3;
    constexpr int PlayerUpgradesLimit = 4;
    //constexpr int EagleUpgradesLimit = 2;
    constexpr int NumOfUpgradesOnLevelup = 4;

};

namespace globalVars
{
    extern int borderWidth;
    extern int borderHeight;

    extern sf::IntRect gameViewPort;
    extern sf::IntRect mapViewPort;

    extern sf::Vector2i mapSize;

    extern bool gameIsPaused;
    extern bool globalTimeFreeze;

    extern sftools::Chronometer globalChronometer;
    extern sftools::Chronometer globalFreezeChronometer;
    extern int globalFreezeTimeout;

    extern int player1Lives;
    extern int player1PowerLevel;
    extern int player1XP;
    extern int player1Level;

    extern bool openLevelUpMenu;
};