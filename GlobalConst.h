#pragma once

#include <SFML/Graphics/Rect.hpp>

namespace globalConst
{
    constexpr int screen_w = 1280;
    constexpr int screen_h = 720;

    constexpr int spriteScaleX = 4;
    constexpr int spriteScaleY = 4;

    constexpr int borderWidth = 128;
    constexpr int borderHeight = 40;
    
    const sf::IntRect gameViewPort = sf::IntRect(borderWidth, borderHeight, screen_w - borderWidth*2, screen_h - borderHeight * 2);
};

