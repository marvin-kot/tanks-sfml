#include "AssetManager.h"
#include "GameObject.h"
#include "GlobalConst.h"
#include "HUD.h"
#include "ObjectsPool.h"
#include "PlayerUpgrade.h"
#include "PlayerController.h"
#include "Utils.h"

void drawText(std::string str, int fontSize, int x, int y, bool leftOrientation = false)
{
    sf::Text text;
    text.setFont(AssetManager::instance().defaultFont());
    text.setString(str);
    text.setCharacterSize(fontSize);
    text.setFillColor(sf::Color::White);

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

void drawCursor(sf::RectangleShape& parentRect, int pos)
{
    int cursorX = 0;
    int cursorY = parentRect.getPosition().y - parentRect.getSize().y/8;
    switch (pos) {
        case 0:
            cursorX = parentRect.getPosition().x - parentRect.getSize().x/4;
            break;
        case 1:
            cursorX = parentRect.getPosition().x;
            break;
        case 2:
            cursorX = parentRect.getPosition().x + parentRect.getSize().x/4;
            break;
    }

    sf::RectangleShape whiteRect(sf::Vector2f(18, 18));
    whiteRect.setScale(globalConst::spriteScaleX+1, globalConst::spriteScaleY+1);
    whiteRect.setOrigin(whiteRect.getSize().x/2, whiteRect.getSize().y/2);
    whiteRect.setPosition(cursorX, cursorY);
    whiteRect.setFillColor(sf::Color(200, 200, 200));
    Utils::window.draw(whiteRect);
}

////////////////////////////////////////

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
    drawPlayerUpgrades();
}

void HUD::drawPlayerLives()
{
    constexpr int iconWidth = 7;
    constexpr int iconHeight = 8;
    constexpr int iconWidthScaled = iconWidth * globalConst::spriteScaleX/2;

    sf::IntRect rect = sf::IntRect(377, 144, iconWidth, iconHeight);
    drawMiniIcon(rect, 16, 32);

    std::string lives = std::to_string(globalVars::player1Lives);
    const int titleSize = 24;
    drawText(lives, titleSize, 16 + iconWidthScaled + 8, 32, true);
}

void HUD::drawPlayerXP()
{
    const int fontSize = 24;
    std::string levelAndXp = std::to_string(globalVars::player1Level) + "/" + std::to_string(globalVars::player1XP);
    drawText(levelAndXp, fontSize, 8, 32 + fontSize + 8, true);
}

void HUD::drawPlayerUpgrades()
{
    if (ObjectsPool::playerObject == nullptr)
        return;

    auto pController = ObjectsPool::playerObject->getComponent<PlayerController>();

    const int upgradesNumber = pController->numberOfUpgrades();

    for (int i=0; i<upgradesNumber; i++) {
        drawMiniIcon(
            pController->getUpgrade(i)->iconRect(),
            32 + i * (32 + 16),
            32 + 24 + 64
        );

        drawText(
            std::to_string(pController->getUpgrade(i)->currentLevel()+1),
            24,
            32 + i * (32 + 16),
            32 + 24 + 64 + 24
        );
    }
}

void HUD::drawMiniIcon(const sf::IntRect& iconRect, int x, int y)
{
    _sprite.setTextureRect(iconRect);
    _sprite.setScale(globalConst::spriteScaleX / 2, globalConst::spriteScaleY / 2);
    _sprite.setOrigin(iconRect.width/2, iconRect.height/2);
    _sprite.setPosition(x, y);

    Utils::window.draw(_sprite);
}

void HUD::drawIcon(const sf::IntRect& iconRect, int x, int y)
{
    _sprite.setTextureRect(iconRect);
    _sprite.setScale(globalConst::spriteScaleX+1, globalConst::spriteScaleY+1);
    _sprite.setOrigin(iconRect.width/2, iconRect.height/2);
    _sprite.setPosition(x, y);

    Utils::window.draw(_sprite);
}

void HUD::drawUpgrade(int index, int x, int y)
{
    assert(index >=0 && index < PlayerUpgrade::currentThreeRandomUpgrades.size());
    auto upgrade = PlayerUpgrade::currentThreeRandomUpgrades[index];

    assert(upgrade != nullptr);
    drawIcon(upgrade->iconRect(), x, y);

    std::string caption = upgrade->name();
    const int captionFontSize = 16;
    drawText(caption, captionFontSize, x, y + 100 + captionFontSize);

    std::string description = upgrade->currentEffectDescription();
    const int descriptionFontSize = 12;
    drawText(description, descriptionFontSize, x, y + 100 + captionFontSize + 20 + descriptionFontSize);

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

    // draw title
    {
        std::string lvl = "You reached level " + std::to_string(globalVars::player1Level) + "!";
        const int titleFontSize = 24;
        drawText( lvl, titleFontSize,
            greyRect.getPosition().x,
            greyRect.getPosition().y - greyRect.getSize().y/2 + titleFontSize);

        const int subtitleSize = 16;
        drawText( "Select one of these upgrades:", subtitleSize,
            greyRect.getPosition().x,
            greyRect.getPosition().y - greyRect.getSize().y/2 + titleFontSize + subtitleSize*2);
    }


    // draw cursor
    drawCursor(greyRect, cursorPos);

    const int iconY = greyRect.getPosition().y - greyRect.getSize().y/8;
    const int centerX = greyRect.getPosition().x;
    const int offsetX = greyRect.getSize().x/4;

    // draw icons
    drawUpgrade(0, centerX - offsetX, iconY);
    drawUpgrade(1, centerX, iconY);
    drawUpgrade(2, centerX + offsetX, iconY);
}