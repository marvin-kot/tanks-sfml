#include "Utils.h"

bool Utils::isOutOfBounds(const sf::IntRect& rect) {
    using namespace globalConst;
    using namespace globalVars;

    return rect.left < gameViewPort.left
        || (rect.left+rect.width > gameViewPort.width + gameViewPort.left)
        || rect.top < gameViewPort.top
        || (rect.top+rect.height > gameViewPort.height + gameViewPort.top);
}

void Utils::gameOver()
{
    window.close();
}