#pragma once
#include <SFML/Graphics.hpp>
#include "GameObject.h"

#include <memory>

class UiUtils
{
private:
    UiUtils();
    UiUtils(UiUtils &) = delete;
    void operator=(const UiUtils&) = delete;

    std::unique_ptr<GameObject> _eagleCursorObject;
    sf::Sprite _sprite;


public:
    static UiUtils& instance();

    //sf::RenderWindow window;

    void drawRect(sf::IntRect, sf::Color);
    void drawText(std::string str, int fontSize, int x, int y, bool leftOrientation = false, sf::Color color = sf::Color::White);
    void drawMiniIcon(const sf::IntRect& iconRect, int x, int y);
    void drawIcon(const sf::IntRect& iconRect, int x, int y);
    void drawAnimatedEagleCursor(int x, int y);
    void drawHorizontalLine(int x, int y, int width, sf::Color color);
};