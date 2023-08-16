#pragma once


#include "GlobalConst.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>

#include <random>

struct Utils
{
    static std::default_random_engine generator;
    static sf::RenderWindow window;

    static int currentFrame;
    static sf::Time lastFrameTime;
    static sf::Clock refreshClock;

    static bool isOutOfBounds(const sf::IntRect& rect);
    static bool isOutOfBounds(int x, int y);
    static void gameOver();

};