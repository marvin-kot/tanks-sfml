#include "Utils.h"

std::default_random_engine Utils::generator = {};
sf::RenderWindow Utils::window = {};
int Utils::currentFrame = 0;

bool Utils::isOutOfBounds(const sf::IntRect& rect) {
    using namespace globalConst;
    using namespace globalVars;

    return rect.left < gameViewPort.left-1
        || (rect.left+rect.width > gameViewPort.width + gameViewPort.left)
        || rect.top < gameViewPort.top
        || (rect.top+rect.height > gameViewPort.height + gameViewPort.top);
}

void Utils::gameOver()
{
    window.close();
}