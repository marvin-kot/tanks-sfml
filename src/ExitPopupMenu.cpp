#include "GameObject.h"
#include "GlobalConst.h"
#include "ExitPopupMenu.h"
#include "ObjectsPool.h"
#include "PlayerController.h"
#include "SoundPlayer.h"
#include "Utils.h"
#include "UiUtils.h"

#include <cassert>

struct YesNoChoice {
    bool positive;
    std::string caption;
    sf::Color color;

    YesNoChoice(bool p, std::string cap, sf::Color col): positive(p), caption(cap), color(col) {}
};

const std::vector<YesNoChoice> menuChoices = {
    YesNoChoice(true, "ESCAPE", sf::Color::Red),
    YesNoChoice(false, "STAY", sf::Color(55, 56, 59)),
};



ExitPopupMenu::ExitPopupMenu() : _currentCursor(1)
{
    if (!_texture.loadFromFile("assets/levelup-popup.png")) {
        Logger::instance() << " failed to load texture\n";
        return;
    }

    _sprite.setTexture(_texture);
    _sprite.setOrigin(_sprite.getLocalBounds().width / 2, _sprite.getLocalBounds().height / 2 );
    _sprite.setScale(8, 8);
    _sprite.setPosition(globalConst::screen_w/2, globalConst::screen_h/2);
}

ExitPopupMenu& ExitPopupMenu::instance()
{
    static ExitPopupMenu _instance;
    return _instance;
}

void ExitPopupMenu::open()
{
    _isOpen = true;
    SoundPlayer::instance().pauseAllSounds();
    SoundPlayer::instance().playSound(SoundPlayer::SoundType::pause);
    globalVars::gameIsPaused = true;
    globalVars::globalChronometer.pause();
    globalVars::globalFreezeChronometer.pause();
    _currentCursor = 1;
}

void ExitPopupMenu::close()
{
    globalVars::gameIsPaused = false;
    globalVars::openLevelUpMenu = false;
    globalVars::globalChronometer.resume();
    globalVars::globalFreezeChronometer.resume();
    _isOpen = false;
}

void ExitPopupMenu::draw()
{
    using namespace globalConst;
    using namespace globalVars;

    // draw frame
    sf::RectangleShape greyRect(sf::Vector2f(screen_w * 4 / 5, screen_h / 2));
    greyRect.setOrigin(greyRect.getSize().x/2, greyRect.getSize().y/2);
    greyRect.setPosition(sf::Vector2f(screen_w/2, screen_h/2));

    const int rectWidth = _sprite.getLocalBounds().width * 8;
    const int rectHeight = _sprite.getLocalBounds().height * 8;

    //_sprite.setOrigin(_sprite.getLocalBounds().width / 2 )
    Utils::window.draw(_sprite);

    // draw title
    const int titleFontSize = 28;
    float currY = _sprite.getPosition().y - rectHeight/2 + titleFontSize*4;
    std::string lvl = "Do you want to escape the mission?";
    UiUtils::instance().drawText( lvl, titleFontSize, _sprite.getPosition().x, currY);


    const int subtitleSize = 20;
    currY += subtitleSize*2;
    UiUtils::instance().drawText( "Select your choice and press [ENTER]", subtitleSize,
        _sprite.getPosition().x,
        currY,
        false, sf::Color::Yellow);

    auto rectDefault = AssetManager::instance().getAnimationFrame("button", "default", 0).rect;
    auto rectSelected = AssetManager::instance().getAnimationFrame("button", "selected", 0).rect;

    const int screenParts = menuChoices.size() + 1;
    const int baseOffsetX = rectWidth/screenParts + 40;
    const int iconY = _sprite.getPosition().y + titleFontSize*2;

    // draw cursor
    //const int centerX = greyRect.getPosition().x;

    for (int i = 0; i < menuChoices.size(); i++) {
        auto currentOffsetX = baseOffsetX + 96 + i*baseOffsetX;

        if (_currentCursor == i) {
            drawCursorOnButton(currentOffsetX, iconY);
        }

        UiUtils::instance().drawIcon(_currentCursor == i ? rectSelected : rectDefault, currentOffsetX, iconY);

        const auto caption = menuChoices[i].caption;
        const auto color = menuChoices[i].color;
        const int captionFontSize = 25;

        UiUtils::instance().drawText(caption, captionFontSize, currentOffsetX, iconY, false, color);
    }

    // draw icons
}

void ExitPopupMenu::drawCursorOnButton(int x,  int y)
{
    const auto rect = AssetManager::instance().getAnimationFrame("button", "default", 0).rect;

    sf::RectangleShape whiteRect(sf::Vector2f(rect.getSize().x+2, rect.getSize().y+2));
    whiteRect.setScale(globalConst::spriteScaleX+1, globalConst::spriteScaleY+1);
    whiteRect.setOrigin(whiteRect.getSize().x/2, whiteRect.getSize().y/2);
    whiteRect.setPosition(x, y);
    whiteRect.setFillColor(sf::Color(200, 200, 200));

    Utils::window.draw(whiteRect);
}

void ExitPopupMenu::moveCursorLeft()
{
    SoundPlayer::instance().playSound(SoundPlayer::SoundType::tick);
    if (--_currentCursor < 0)
        _currentCursor = menuChoices.size() - 1;
}

void ExitPopupMenu::moveCursorRight()
{
    SoundPlayer::instance().playSound(SoundPlayer::SoundType::tick);
    if (++_currentCursor >= menuChoices.size())
        _currentCursor = 0;
}

globalTypes::GameState ExitPopupMenu::applyChoice()
{
    assert(_currentCursor >=0 && _currentCursor < menuChoices.size());

    if (menuChoices[_currentCursor].positive)
        return globalTypes::GameState::GameOver;
    else
        return globalTypes::GameState::PlayingLevel;
}