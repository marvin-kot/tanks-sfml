#pragma once
#include <SFML/Graphics.hpp>

class UiUtils
{
private:
    sf::Sprite _sprite;
    UiUtils();
    UiUtils(UiUtils &) = delete;
    void operator=(const UiUtils&) = delete;

public:
    static UiUtils& instance();

    //sf::RenderWindow window;

    void drawRect(sf::IntRect, sf::Color);
    void drawText(std::string str, int fontSize, int x, int y, bool leftOrientation = false, sf::Color color = sf::Color::White);
    void drawMiniIcon(const sf::IntRect& iconRect, int x, int y);
    void drawIcon(const sf::IntRect& iconRect, int x, int y);
};