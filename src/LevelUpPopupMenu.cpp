#include "GameObject.h"
#include "GlobalConst.h"
#include "LevelUpPopupMenu.h"
#include "ObjectsPool.h"
#include "PlayerController.h"
#include "PlayerUpgrade.h"
#include "SoundPlayer.h"
#include "Utils.h"
#include "UiUtils.h"

#include <cassert>

LevelUpPopupMenu::LevelUpPopupMenu() : _currentUpgradeCursor(0)
{}

LevelUpPopupMenu& LevelUpPopupMenu::instance()
{
    static LevelUpPopupMenu _instance;
    return _instance;
}

void LevelUpPopupMenu::open()
{
    _isOpen = true;
    SoundPlayer::instance().stopAllSounds();
    SoundPlayer::instance().playBonusCollectSound();
    globalVars::gameIsPaused = true;
    globalVars::globalChronometer.pause();
    globalVars::globalFreezeChronometer.pause();
}

void LevelUpPopupMenu::close()
{
    globalVars::gameIsPaused = false;
    globalVars::openLevelUpMenu = false;
    globalVars::globalChronometer.resume();
    globalVars::globalFreezeChronometer.resume();
    _isOpen = false;
}

void LevelUpPopupMenu::draw()
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
        const int titleFontSize = 28;
        UiUtils::instance().drawText( lvl, titleFontSize,
            greyRect.getPosition().x,
            greyRect.getPosition().y - greyRect.getSize().y/2 + titleFontSize);

        const int subtitleSize = 20;
        UiUtils::instance().drawText( "Select 1 of 3 bonuses and press [ENTER]", subtitleSize,
            greyRect.getPosition().x,
            greyRect.getPosition().y - greyRect.getSize().y/2 + titleFontSize + subtitleSize*2);
    }

    // draw cursor
    drawCursor(greyRect);

    const int iconY = greyRect.getPosition().y - greyRect.getSize().y/8;
    const int centerX = greyRect.getPosition().x;
    const int offsetX = greyRect.getSize().x/4 + 32;

    // draw icons
    drawUpgrade(0, centerX - offsetX, iconY);
    drawUpgrade(1, centerX, iconY);
    drawUpgrade(2, centerX + offsetX, iconY);
}


void  LevelUpPopupMenu::drawCursor(sf::RectangleShape& parentRect)
{
    int cursorX = 0;
    int cursorY = parentRect.getPosition().y - parentRect.getSize().y/8;
    switch (_currentUpgradeCursor) {
        case 0:
            cursorX = parentRect.getPosition().x - parentRect.getSize().x/4 - 32;
            break;
        case 1:
            cursorX = parentRect.getPosition().x;
            break;
        case 2:
            cursorX = parentRect.getPosition().x + parentRect.getSize().x/4 + 32;
            break;
    }

    sf::RectangleShape whiteRect(sf::Vector2f(18, 18));
    whiteRect.setScale(globalConst::spriteScaleX+1, globalConst::spriteScaleY+1);
    whiteRect.setOrigin(whiteRect.getSize().x/2, whiteRect.getSize().y/2);
    whiteRect.setPosition(cursorX, cursorY);
    whiteRect.setFillColor(sf::Color(200, 200, 200));
    Utils::window.draw(whiteRect);
}

void LevelUpPopupMenu::drawUpgrade(int index, int x, int y)
{
    assert(index >=0 && index < PlayerUpgrade::currentThreeRandomUpgrades.size());
    auto upgrade = PlayerUpgrade::currentThreeRandomUpgrades[index];

    assert(upgrade != nullptr);
    UiUtils::instance().drawIcon(upgrade->iconRect(), x, y);

    std::string caption = upgrade->name();
    const int captionFontSize = 18;
    UiUtils::instance().drawText(caption, captionFontSize, x, y + 60 + captionFontSize);

    std::string description = upgrade->currentEffectDescription();
    const int descriptionFontSize = 14;
    UiUtils::instance().drawText(description, descriptionFontSize, x, y + 100 + captionFontSize + 20 + descriptionFontSize);
}

void LevelUpPopupMenu::moveCursorLeft()
{
    SoundPlayer::instance().playTickSound();
    if (--_currentUpgradeCursor < 0)
        _currentUpgradeCursor = 2;
}

void LevelUpPopupMenu::moveCursorRight()
{
    SoundPlayer::instance().playTickSound();
    if (++_currentUpgradeCursor > 2)
        _currentUpgradeCursor = 0;
}

void LevelUpPopupMenu::getSelectedUpgrade()
{
    assert(ObjectsPool::playerObject != nullptr);
    PlayerController *controller = ObjectsPool::playerObject->getComponent<PlayerController>();
    assert(controller != nullptr);
    controller->chooseUpgrade(_currentUpgradeCursor);
}