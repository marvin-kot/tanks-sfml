#pragma once

#include <random>

#include "GlobalConst.h"


class Utils
{
public:

    static std::default_random_engine generator;
    static sf::RenderWindow window;

    static bool isOutOfBounds(const sf::IntRect& rect) {
        using namespace globalConst;
        return rect.left < gameViewPort.left
            || (rect.left+rect.width > gameViewPort.width + gameViewPort.left)
            || rect.top < gameViewPort.top
            || (rect.top+rect.height > gameViewPort.height + gameViewPort.top);
    }


    static void gameOver() {
        window.close();
    }

};