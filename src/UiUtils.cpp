#include "AssetManager.h"
#include "UiUtils.h"
#include "Utils.h"



UiUtils::UiUtils()
{
    _sprite.setTexture(AssetManager::instance().mainSpriteSheetTexture());
}

UiUtils& UiUtils::instance()
{
    static UiUtils _instance;
    return _instance;
}

void UiUtils::drawRect(sf::IntRect rect, sf::Color color)
{
    sf::RectangleShape shape(sf::Vector2f(rect.width, rect.height));
    shape.setPosition(rect.left, rect.top);
    shape.setFillColor(color);
    Utils::window.draw(shape);
}

void UiUtils::drawText(std::string str, int fontSize, int x, int y, bool leftOrientation, sf::Color color)
{
    sf::Text text;
    text.setFont(AssetManager::instance().defaultFont());
    text.setString(str);
    text.setCharacterSize(fontSize);
    text.setFillColor(color);

    if (leftOrientation)
        text.setOrigin(0, fontSize/2);
    else {
        size_t length = str.find("\n");
        if (length == std::string::npos)
            length = str.length();
        text.setOrigin(fontSize * length / 2.5, fontSize/2);
    }
    text.setPosition(x, y);
    Utils::window.draw(text);
}

void UiUtils::drawMiniIcon(const sf::IntRect& iconRect, int x, int y)
{
    _sprite.setTextureRect(iconRect);
    _sprite.setScale(globalConst::spriteScaleX / 2, globalConst::spriteScaleY / 2);
    _sprite.setOrigin(iconRect.width/2, iconRect.height/2);
    _sprite.setPosition(x, y);

    Utils::window.draw(_sprite);
}

void UiUtils::drawIcon(const sf::IntRect& iconRect, int x, int y)
{
    _sprite.setTextureRect(iconRect);
    _sprite.setScale(globalConst::spriteScaleX+1, globalConst::spriteScaleY+1);
    _sprite.setOrigin(iconRect.width/2, iconRect.height/2);
    _sprite.setPosition(x, y);

    Utils::window.draw(_sprite);
}