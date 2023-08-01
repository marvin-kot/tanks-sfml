#include "Utils.h"


static bool Utils::isOutOfBounds(const sf::IntRect& rect) {
        return rect.left < 0
            || (rect.left+rect.width > globalConst::screen_w)
            || rect.top < 0
            || (rect.top+rect.height > globalConst::screen_h);
    }