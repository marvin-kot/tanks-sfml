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

void HUD::draw()
{
    drawPlayerLives();
    drawPlayerXP();
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

void HUD::drawPlayerXP()
{
    sf::Text text;
    text.setFont(AssetManager::instance().defaultFont());
    std::string lives = std::to_string(globalVars::player1Level) + "/" + std::to_string(globalVars::player1XP);
    text.setString(lives);
    const int titleSize = 24;
    text.setCharacterSize(titleSize);
    text.setFillColor(sf::Color::White);
    text.setOrigin(0, 0);
    text.setPosition(16, 16 + titleSize + 8);
    Utils::window.draw(text);
}

void HUD::drawLevelUpPopupMenu(int cursorPos)
{
    using namespace globalConst;
    using namespace globalVars;

    // draw frame
    sf::RectangleShape greyRect(sf::Vector2f(screen_w * 2 / 3, screen_h / 2));
    greyRect.setOrigin(greyRect.getSize().x/2, greyRect.getSize().y/2);
    greyRect.setPosition(sf::Vector2f(screen_w/2, screen_h/2));
    greyRect.setFillColor(sf::Color(102, 102, 102));
    Utils::window.draw(greyRect);

    // draw cursor
    int cursorX = 0;
    int cursorY = greyRect.getPosition().y - greyRect.getSize().y/4;
    switch (cursorPos) {
        case 0:
            cursorX = greyRect.getPosition().x - greyRect.getSize().x/4;
            break;
        case 1:
            cursorX = greyRect.getPosition().x;
            break;
        case 2:
            cursorX = greyRect.getPosition().x + greyRect.getSize().x/4;
            break;
    }

    sf::RectangleShape whiteRect(sf::Vector2f(20, 20));
    whiteRect.setScale(globalConst::spriteScaleX, globalConst::spriteScaleY);
    whiteRect.setOrigin(whiteRect.getSize().x/2, whiteRect.getSize().y/2);
    whiteRect.setPosition(cursorX, cursorY);
    whiteRect.setFillColor(sf::Color(200, 200, 200));
    Utils::window.draw(whiteRect);

    // draw icons
    {
        sf::IntRect rect = AssetManager::instance().getAnimationFrame("grenadeCollectable", "default", 0).rect;
        _sprite.setTextureRect(rect);
        _sprite.setScale(globalConst::spriteScaleX, globalConst::spriteScaleY);
        _sprite.setOrigin(rect.width/2, rect.height/2);
        _sprite.setPosition(greyRect.getPosition().x - greyRect.getSize().x/4, cursorY);
        Utils::window.draw(_sprite);

        sf::Text text;
        text.setFont(AssetManager::instance().defaultFont());
        std::string lvl = "Level 1";
        text.setString(lvl);
        const int titleSize = 16;
        text.setCharacterSize(titleSize);
        text.setFillColor(sf::Color::White);
        text.setOrigin(titleSize * lvl.length() / 2.5, titleSize/2);
        text.setPosition(greyRect.getPosition().x - greyRect.getSize().x/4, cursorY + 100 + 16);
        Utils::window.draw(text);
    }
    {
        sf::IntRect rect = AssetManager::instance().getAnimationFrame("starCollectable", "default", 0).rect;
        _sprite.setTextureRect(rect);
        _sprite.setScale(globalConst::spriteScaleX, globalConst::spriteScaleY);
        _sprite.setOrigin(rect.width/2, rect.width/2);
        _sprite.setPosition(greyRect.getPosition().x, cursorY);
        Utils::window.draw(_sprite);
    }
    {
        sf::IntRect rect = AssetManager::instance().getAnimationFrame("helmetCollectable", "default", 0).rect;
        _sprite.setTextureRect(rect);
        _sprite.setScale(globalConst::spriteScaleX, globalConst::spriteScaleY);
        _sprite.setOrigin(rect.width/2, rect.width/2);
        _sprite.setPosition(greyRect.getPosition().x + greyRect.getSize().x/4, cursorY);
        Utils::window.draw(_sprite);
    }

}