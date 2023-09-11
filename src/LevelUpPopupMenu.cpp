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
    SoundPlayer::instance().pauseAllSounds();
    SoundPlayer::instance().playSound(SoundPlayer::SoundType::bonusCollect);
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
    sf::RectangleShape greyRect(sf::Vector2f(screen_w * 3 / 4, screen_h / 2));
    greyRect.setOrigin(greyRect.getSize().x/2, greyRect.getSize().y/2);
    greyRect.setPosition(sf::Vector2f(screen_w/2, screen_h/2));
    greyRect.setFillColor(sf::Color(102, 102, 102));
    Utils::window.draw(greyRect);

    // draw title
    {
        std::string lvl = "You've reached level " + std::to_string(globalVars::player1Level) + "!";
        const int titleFontSize = 28;
        UiUtils::instance().drawText( lvl, titleFontSize,
            greyRect.getPosition().x,
            greyRect.getPosition().y - greyRect.getSize().y/2 + titleFontSize);

        const int subtitleSize = 20;
        UiUtils::instance().drawText( "Select 1 of 4 rewards and press [ENTER]", subtitleSize,
            greyRect.getPosition().x,
            greyRect.getPosition().y - greyRect.getSize().y/2 + titleFontSize + subtitleSize*2,
            false, sf::Color::Yellow);
    }

    // draw cursor
    drawCursor(greyRect);

    const int iconY = greyRect.getPosition().y - greyRect.getSize().y/8;
    //const int centerX = greyRect.getPosition().x;
    constexpr int screenParts = globalConst::NumOfUpgradesOnLevelup + 1;

    const int offsetX = greyRect.getSize().x/screenParts + 40;

    for (int i = 0; i < PlayerUpgrade::currentRandomUpgrades.size(); i++)
        drawUpgrade(i, offsetX + 128 + i*offsetX, iconY);
    // draw icons
}


void  LevelUpPopupMenu::drawCursor(sf::RectangleShape& parentRect)
{
    constexpr int screenParts = globalConst::NumOfUpgradesOnLevelup + 1;
    const int offsetX = parentRect.getSize().x/screenParts + 40;
    int cursorX = 128 + offsetX * (_currentUpgradeCursor+1);
    int cursorY = parentRect.getPosition().y - parentRect.getSize().y/8;
    /*switch (_currentUpgradeCursor) {
        case 0:
            cursorX = parentRect.getPosition().x - parentRect.getSize().x/4 - 32;
            break;
        case 1:
            cursorX = parentRect.getPosition().x;
            break;
        case 2:
            cursorX = parentRect.getPosition().x + parentRect.getSize().x/4 + 32;
            break;
    }*/

    sf::RectangleShape whiteRect(sf::Vector2f(globalConst::spriteOriginalSizeX+2, globalConst::spriteOriginalSizeY+2));
    whiteRect.setScale(globalConst::spriteScaleX+1, globalConst::spriteScaleY+1);
    whiteRect.setOrigin(whiteRect.getSize().x/2, whiteRect.getSize().y/2);
    whiteRect.setPosition(cursorX, cursorY);
    whiteRect.setFillColor(sf::Color(200, 200, 200));
    Utils::window.draw(whiteRect);
}

void LevelUpPopupMenu::drawUpgrade(int index, int x, int y)
{
    assert(index >=0 && index < PlayerUpgrade::currentRandomUpgrades.size());
    auto upgrade = PlayerUpgrade::currentRandomUpgrades[index];

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
    SoundPlayer::instance().playSound(SoundPlayer::SoundType::tick);
    if (--_currentUpgradeCursor < 0)
        _currentUpgradeCursor = PlayerUpgrade::currentRandomUpgrades.size() - 1;
}

void LevelUpPopupMenu::moveCursorRight()
{
    SoundPlayer::instance().playSound(SoundPlayer::SoundType::tick);
    if (++_currentUpgradeCursor >= PlayerUpgrade::currentRandomUpgrades.size())
        _currentUpgradeCursor = 0;
}

void LevelUpPopupMenu::getSelectedUpgrade()
{
    assert(ObjectsPool::playerObject != nullptr);
    PlayerController *controller = ObjectsPool::playerObject->getComponent<PlayerController>();
    assert(controller != nullptr);
    controller->chooseUpgrade(_currentUpgradeCursor);
}