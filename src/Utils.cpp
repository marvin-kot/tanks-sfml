#include "Utils.h"

std::default_random_engine Utils::generator = {};
sf::RenderWindow Utils::window = {};
int Utils::currentFrame = 0;
sf::Time Utils::lastFrameTime = {};
sf::Clock Utils::refreshClock = {};

// Add variables for screen shake
float Utils::shakeIntensity = 0;
int Utils::shakeDuration = 0;

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

// Add method to trigger screen shake
void Utils::triggerScreenShake(float intensity, int duration)
{
    if (duration < shakeDuration) {
        return; // Already shaking
    }
    shakeIntensity = intensity;
    shakeDuration = duration;
}

// Add method to apply screen shake effect
void Utils::applyScreenShake()
{
    if (shakeDuration > 0) {
        int offsetX = static_cast<int>((rand() % 100 - 50) / 50.0f * shakeIntensity);
        int offsetY = static_cast<int>((rand() % 100 - 50) / 50.0f * shakeIntensity);
        window.setView(sf::View(sf::FloatRect(offsetX, offsetY, globalConst::screen_w, globalConst::screen_h)));
        shakeDuration--;
    } else {
        window.setView(sf::View(sf::FloatRect(0, 0, globalConst::screen_w, globalConst::screen_h)));
    }
}