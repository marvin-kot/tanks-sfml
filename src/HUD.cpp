#include "AssetManager.h"
#include "GlobalConst.h"
#include "HUD.h"
#include "Utils.h"

HUD::HUD()
{
    _sprite.setTexture(AssetManager::instance().mainSpriteSheetTexture());
}

HUD& HUD::instance()
{
    static HUD _instance;
    return _instance;
}

void HUD::drawPlayerLives()
{
    constexpr int iconWidth = 7;
    constexpr int iconHeight = 8;
    constexpr int iconWidthScaled = iconWidth * globalConst::spriteScaleX;

    sf::IntRect rect = sf::IntRect(377, 144, iconWidth, iconHeight);
    _sprite.setTextureRect(rect);
    _sprite.setScale(globalConst::spriteScaleX, globalConst::spriteScaleY);
    _sprite.setOrigin(0, 0);

    _sprite.setPosition(16, 16);

    Utils::window.draw(_sprite);

    sf::Text text;
    text.setFont(AssetManager::instance().defaultFont());
    std::string lives = std::to_string(globalVars::player1Lives);
    text.setString(lives);
    const int titleSize = 24;
    text.setCharacterSize(titleSize);
    text.setFillColor(sf::Color::White);
    text.setOrigin(0, 0);
    text.setPosition(16 + iconWidthScaled + 8, 16);
    Utils::window.draw(text);
}