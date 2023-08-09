#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Clock.hpp>


namespace globalTypes
{
    enum Direction
    {
        Up,
        Left,
        Down,
        Right
    };
}

namespace globalConst
{
    constexpr int screen_w = 1920;
    constexpr int screen_h = 1080;

    constexpr int spriteOriginalSizeX = 16;
    constexpr int spriteOriginalSizeY = 16;

    constexpr int spriteScaleX = 4;
    constexpr int spriteScaleY = 4;

    constexpr int spriteDisplaySizeX = spriteOriginalSizeX * spriteScaleX;
    constexpr int spriteDisplaySizeY = spriteOriginalSizeY * spriteScaleY;

    constexpr int maxFieldWidth = screen_w / spriteDisplaySizeX;
    constexpr int maxFieldHeight = screen_h / spriteDisplaySizeY;

};

namespace globalVars
{
    extern int borderWidth;
    extern int borderHeight;

    extern sf::IntRect gameViewPort;

    extern sf::Vector2i mapSize;

    extern bool globalTimeFreeze;
    extern sf::Clock globalFreezeClock;
};
