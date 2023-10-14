#include "AssetManager.h"
#include "BonusShopWindow.h"
#include "GlobalConst.h"
#include "PersistentGameData.h"
#include "SoundPlayer.h"
#include "TitleScreen.h"
#include "UiUtils.h"
#include "Utils.h"

#include <SFML/System/Time.hpp>

#include <format>


TitleScreen::TitleScreen()
: _cursorPos(0)
, _blinkCount(0)
, _selected(false)
, _blink(false)
{
    _clock.restart();
}

TitleScreen::~TitleScreen()
{
}

TitleScreen& TitleScreen::instance()
{
    static TitleScreen _instance;
    return _instance;
}

//void TitlwScreen::drawCursor()


globalTypes::GameState TitleScreen::draw()
{
    using namespace globalTypes;
    using namespace globalConst;

    constexpr int screenCenterX = screen_w / 2;
    constexpr int screenCenterY = screen_h / 2;

    // draw black rect
    UiUtils::instance().drawRect(sf::IntRect(0, 0, screen_w, screen_h), sf::Color(0, 0, 0));


    switch (_state) {
        case Start:
            SoundPlayer::instance().playSound(SoundPlayer::SoundType::TitleTheme);
            _state = ShowIntro1;
            _delayClock.restart();
            break;
        case ShowIntro1:
            drawIntro1(screenCenterX, screenCenterY);
            break;
        case ShowIntro2:
            drawIntro2(screenCenterX, screenCenterY);
            break;
        case ShowTitle:
            drawTitleMenu(screenCenterX, screenCenterY);
            break;
    }

    Utils::window.display();


    if (!_selected)
        return GameState::TitleScreen;

    if (--_blinkCount > 0)
        return GameState::TitleScreen;

    if (_cursorPos == 0) {
        _selected = false; _blink = false;
        if (PersistentGameData::instance().isShopUnlocked()) {
            BonusShopWindow::instance().afterGameOver = false;
            return GameState::BonusShop;
        }
        else {
            return GameState::LoadNextLevel;
        }
    } else {
        SoundPlayer::instance().stopSound(SoundPlayer::SoundType::TitleTheme);
        return GameState::ExitGame;
    }
}

void TitleScreen::drawIntro1(int screenCenterX, int screenCenterY)
{
    using namespace globalTypes;
    using namespace globalConst;

    // game title

    int delay1Start = 200;
    int delay2Start = 400;
    int delayShow = 4850;
    int delay1Hide = 5050;
    int delay2Hide = 5250;

    int diff = _delayClock.getElapsedTime().asMilliseconds();

    sf::Color startGameColor = sf::Color::White;

    if (diff < delay1Start)
        startGameColor = sf::Color::Black;
    else if (diff < delay2Start)
        startGameColor = sf::Color(102,102,102);
    else if (diff < delayShow)
        startGameColor = sf::Color::White;
    else if (diff<delay1Hide)
        startGameColor = sf::Color(102,102,102);
    else if (diff<delay2Hide)
        startGameColor = sf::Color::Black;
    else {
        _state = ShowIntro2;
        _delayClock.restart();
        return;
    }

    constexpr int titleFontSize = 36;
    int currentY = screenCenterY;
    UiUtils::instance().drawText( "mountain shark games", titleFontSize, screenCenterX, currentY, false, startGameColor);

    // game version
    constexpr int versionFontSize = titleFontSize / 3;
    currentY += titleFontSize + versionFontSize;
    static std::string version = std::format("presents");
    UiUtils::instance().drawText(version, versionFontSize, screenCenterX, currentY, false, startGameColor);
}

void TitleScreen::drawIntro2(int screenCenterX, int screenCenterY)
{
    using namespace globalTypes;
    using namespace globalConst;

    // game title
    constexpr int titleFontSize = 36;
    int currentY = screenCenterY;

    int delay1Start = 200;
    int delay2Start = 400;
    int delayShow = 4850;
    int delay1Hide = 5050;
    int delay2Hide = 5280;

    int diff = _delayClock.getElapsedTime().asMilliseconds();

    sf::Color startGameColor = sf::Color::White;

    if (diff < delay1Start)
        startGameColor = sf::Color::Black;
    else if (diff < delay2Start)
        startGameColor = sf::Color(102,102,102);
    else if (diff < delayShow)
        startGameColor = sf::Color::White;
    else if (diff<delay1Hide)
        startGameColor = sf::Color(102,102,102);
    else if (diff<delay2Hide) {
        startGameColor = sf::Color::Black;
    } else {
        _state = ShowTitle;
        _delayClock.restart();
        return;
    }

    UiUtils::instance().drawText( "a game by alex akulchyk", titleFontSize, screenCenterX, currentY, false, startGameColor);
}


void TitleScreen::drawTitleMenu(int screenCenterX, int screenCenterY)
{
    using namespace globalTypes;
    using namespace globalConst;


    int diff = _delayClock.getElapsedTime().asMilliseconds();

    sf::Color gameTitleColor = sf::Color::White;

    std::vector<int> delays;

    for (int i=1; i<9; i++) {
        delays.push_back(164*i);
    }

    if (diff < delays[0])
        gameTitleColor = sf::Color(250,50,50);
    else if (diff < delays[1])
        gameTitleColor = sf::Color(250,100,100);
    else if (diff < delays[2])
        gameTitleColor = sf::Color(250,150,150);
    else if (diff < delays[3])
        gameTitleColor = sf::Color(250,200,200);
    else if (diff < delays[4])
        gameTitleColor = sf::Color(250,250,250);
    else if (diff < delays[5])
        gameTitleColor = sf::Color(250,200,200);
    else if (diff < delays[6])
        gameTitleColor = sf::Color(250,100,100);
    else {
        gameTitleColor = sf::Color(250,50,50);
        _delayClock.restart();
    }

    // game title
    constexpr int titleFontSize = 100;
    int currentY = screenCenterY - titleFontSize;
    UiUtils::instance().drawText( "RETRO TANK MASSACRE", titleFontSize, screenCenterX, currentY, false, gameTitleColor);

    // game version
    constexpr int versionFontSize = titleFontSize / 6;
    currentY += titleFontSize + versionFontSize;
    static std::string version = std::format("version {}.{}.{}", GameMajorVersion, GameMinorVersion, GameReleaseVersion);
    UiUtils::instance().drawText(version, versionFontSize, screenCenterX, currentY);

    // Draw options
    // start the game
    const sf::Color startGameColor = _cursorPos == 0 ? sf::Color::Yellow : sf::Color::White;
    constexpr int promptFontSize = titleFontSize / 3;
    currentY += titleFontSize + promptFontSize;

    if (_selected && _clock.getElapsedTime() > sf::milliseconds(200)) {
        _blink = !_blink;
        _clock.restart();
    }

    if (!_blink) {
        std::string option = "start the game";
        if (_cursorPos == 0) {
            UiUtils::instance().drawAnimatedEagleCursor(screenCenterX - (promptFontSize * option.size() / 2) - 16, currentY);
            UiUtils::instance().drawAnimatedEagleCursor(screenCenterX + (promptFontSize * option.size() / 2) + 16, currentY);
        }
        UiUtils::instance().drawText( option, promptFontSize, screenCenterX, currentY, false, startGameColor );
    }

    // perk shop
    /*sf::Color shopColor = _cursorPos == 1 ? sf::Color::Yellow : sf::Color::White;

    if (!PersistentGameData::instance().isShopUnlocked())
        shopColor = sf::Color(102, 102, 102);
    currentY += promptFontSize+10;
    UiUtils::instance().drawText( "perk shop", promptFontSize, screenCenterX, currentY, false, shopColor );*/

    // exit to desktop
    const sf::Color exitColor = _cursorPos == 1 ? sf::Color::Yellow : sf::Color::White;
    currentY += promptFontSize+24;
    std::string option = "exit to desktop";
    if (_cursorPos == 1) {
        UiUtils::instance().drawAnimatedEagleCursor(screenCenterX - (promptFontSize * option.size() / 2) - 16, currentY);
        UiUtils::instance().drawAnimatedEagleCursor(screenCenterX + (promptFontSize * option.size() / 2) + 16, currentY);
    }
    UiUtils::instance().drawText( option, promptFontSize, screenCenterX, currentY, false, exitColor );
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
            if (_state != ShowTitle) return;
            moveCursorUp();
            break;
        case sf::Keyboard::Scan::Down:
            if (_state != ShowTitle) return;
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
        _cursorPos = 1;

    //if (_cursorPos == 1 && !PersistentGameData::instance().isShopUnlocked())
    //    --_cursorPos;
}

void TitleScreen::moveCursorDown()
{
    if (_selected) return;
    SoundPlayer::instance().playSound(SoundPlayer::SoundType::moveCursor);
    if (++_cursorPos > 1)
        _cursorPos = 0;

    //if (_cursorPos == 1 && !PersistentGameData::instance().isShopUnlocked())
    //    ++_cursorPos;
}

void TitleScreen::selectOption()
{
    if (_state < ShowTitle) {
        _state = ShowTitle;
        SoundPlayer::instance().playSoundWithOffset(SoundPlayer::SoundType::TitleTheme, 10660);
        return;
    }

    _selected = true;

    if (_cursorPos == 0) {
        SoundPlayer::instance().stopSound(SoundPlayer::SoundType::TitleTheme);
        SoundPlayer::instance().playSound(SoundPlayer::SoundType::startGame);
        _blinkCount = 90;
        //_clock.restart();
    }
}