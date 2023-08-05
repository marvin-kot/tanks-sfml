#pragma once


#include "GlobalConst.h"

#include <SFML/Graphics/RenderWindow.hpp>

#include <random>

class Utils
{
public:

    static std::default_random_engine generator;
    static sf::RenderWindow window;

    static bool isOutOfBounds(const sf::IntRect& rect);
    static void gameOver();

};