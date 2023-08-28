#include "GlobalConst.h"
#include "SoundPlayer.h"
#include "TitleScreen.h"
#include "UiUtils.h"
#include "Utils.h"

#include <SFML/System/Time.hpp>

#include <format>

TitleScreen::TitleScreen() : _cursorPos(0), _blinkCount(0), _selected(false), _blink(false) {
    _clock.restart();
}


TitleScreen& TitleScreen::instance()
{
    static TitleScreen _instance;
    return _instance;
}

//void TitlwScreen::drawCursor()


int TitleScreen::draw()
{
    using namespace globalConst;

    constexpr int screenCenterX = screen_w / 2;
    constexpr int screenCenterY = screen_h / 2;

    // draw black rect
    UiUtils::instance().drawRect(sf::IntRect(0, 0, screen_w, screen_h), sf::Color(0, 0, 0));

    // game title
    constexpr int titleFontSize = 96;
    int currentY = screenCenterY - titleFontSize;
    UiUtils::instance().drawText( "RETRO TANK MASSACRE", titleFontSize, screenCenterX, currentY);

    // game version
    constexpr int versionFontSize = titleFontSize / 6;
    currentY += titleFontSize + versionFontSize;
    static std::string version = std::format("version {}.{}.{}", GameMajorVersion, GameMinorVersion, GameReleaseVersion);
    UiUtils::instance().drawText(version, versionFontSize, screenCenterX, currentY);

    // Draw options
    // start the game
    const sf::Color startGameColor = _cursorPos == 0 ? sf::Color::Yellow : sf::Color::White;
    constexpr int promptFontSize = titleFontSize / 4;
    currentY += titleFontSize + promptFontSize;

    if (_selected && _clock.getElapsedTime() > sf::milliseconds(200)) {
        _blink = !_blink;
        _clock.restart();
    }

    if (!_blink)
        UiUtils::instance().drawText( "start the game", promptFontSize, screenCenterX, currentY, false, startGameColor );

    // perk shop
    const sf::Color shopColor = _cursorPos == 1 ? sf::Color::Yellow : sf::Color::White;
    currentY += promptFontSize+10;
    UiUtils::instance().drawText( "perk shop", promptFontSize, screenCenterX, currentY, false, shopColor );

    // exit to desktop
    const sf::Color exitColor = _cursorPos == 2 ? sf::Color::Yellow : sf::Color::White;
    currentY += promptFontSize+10;
    UiUtils::instance().drawText( "exit to desktop", promptFontSize, screenCenterX, currentY, false, exitColor );

    Utils::window.display();

    if (!_selected)
        return 0; // TitleScreen

    if (--_blinkCount > 0)
        return 0;

    if (_cursorPos == 0) {
        _selected = false; _blink = false;
        return 1; // LoadNextLevel
    }
    else if (_cursorPos == 1) {
        _selected = false; _blink = false;
        return 6; // BonusShop
    }
    else {
        return 7; // ExitGame
    }
}

void TitleScreen::processKeyboardPress(sf::Keyboard::Scancode scancode)
{
    if (_selected) return;

    switch (scancode)
    {
        case sf::Keyboard::Scan::Escape:
            _cursorPos = 2;
            selectOption();
            break;
        case sf::Keyboard::Scan::Up:
            moveCursorUp();
            break;
        case sf::Keyboard::Scan::Down:
            moveCursorDown();
            break;
        case sf::Keyboard::Scan::Enter:
            selectOption();
            break;

    }
}


void TitleScreen::moveCursorUp()
{
    if (_selected) return;
    SoundPlayer::instance().playSound(SoundPlayer::SoundType::moveCursor);
    if (--_cursorPos < 0)
        _cursorPos = 2;
}

void TitleScreen::moveCursorDown()
{
    if (_selected) return;
    SoundPlayer::instance().playSound(SoundPlayer::SoundType::moveCursor);
    if (++_cursorPos > 2)
        _cursorPos = 0;
}

void TitleScreen::selectOption()
{
    _selected = true;

    if (_cursorPos == 0) {
        SoundPlayer::instance().playSound(SoundPlayer::SoundType::startGame);
        _blinkCount = 90;
        //_clock.restart();
    }
}