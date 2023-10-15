#include "StartLevelScreen.h"

#include "GlobalConst.h"
#include "GlobalTypes.h"
#include "Logger.h"
#include "SoundPlayer.h"
#include "UiUtils.h"
#include "Utils.h"

StartLevelScreen::StartLevelScreen() {
    if (!_texture.loadFromFile("assets/shop-window.png")) {
        Logger::instance() << " failed to load texture\n";
        return;
    }

    _sprite.setTexture(_texture);
    _sprite.setScale(8, 8);
}

StartLevelScreen& StartLevelScreen::instance()
{
    static StartLevelScreen _instance;
    return _instance;
}

void StartLevelScreen::draw()
{
    using namespace globalConst;
    constexpr int screenCenterX = screen_w / 2;
    constexpr int screenCenterY = screen_h / 2;

    constexpr int screenQuarterY = screen_h / 4;

    // grey background
    Utils::window.draw(_sprite);

    int currentStringY = screenQuarterY - 64;
    // level name
    UiUtils::instance().drawText(
        _currentLevelProperties.name, 48,
        screenCenterX, currentStringY, false,
        sf::Color::White);

    currentStringY += 80;

    for (auto& brief : _currentLevelProperties.briefing) {
        UiUtils::instance().drawText( brief, 23, screenCenterX, currentStringY, false, sf::Color::White);
        currentStringY += 25;
    }

    // level goal
    currentStringY += 75;
    UiUtils::instance().drawText(
        "Win", 32,
        screenCenterX, currentStringY, false,
        sf::Color::Green);

    currentStringY += 32;
    const Level::WinCondition win = _currentLevelProperties.win;
    const int winParam = _currentLevelProperties.winParam;
    char formattedStr[100];
    {
        const char *placeholder = Level::winDescriptionsMap.at(win);
        sprintf(formattedStr, placeholder, winParam);
    }

    UiUtils::instance().drawText(
        formattedStr, 24,
        screenCenterX, currentStringY, false, sf::Color::White);

    // fail condition
    currentStringY += 60;
    UiUtils::instance().drawText(
        "Fail", 32,
        screenCenterX, currentStringY, false,
        sf::Color::Red);

    const Level::FailCondition fail = _currentLevelProperties.fail;
    const int failParam = _currentLevelProperties.failParam;
    {
        const char *placeholder = Level::failDescriptionsMap.at(fail);
        sprintf(formattedStr, placeholder, winParam);
    }
    currentStringY += 32;
    UiUtils::instance().drawText(
        formattedStr,
        24, screenCenterX, currentStringY, false, sf::Color::White);

    // prompt
    currentStringY += 150;

    auto rect = AssetManager::instance().getAnimationFrame("button", "default", 0).rect;
    UiUtils::instance().drawIcon(rect, screenCenterX, currentStringY);

    UiUtils::instance().drawText(
        "start", 26,
        screenCenterX, currentStringY, false,
        sf::Color::Yellow);
    Utils::window.display();
}

void StartLevelScreen::setLevelProperties(const Level::Properties& prop)
{
    _currentLevelProperties = prop;
}

void StartLevelScreen::processKeyboardPress(sf::Keyboard::Scancode scancode, globalTypes::GameState& gameState)
{
    if (scancode == sf::Keyboard::Scan::Enter)
        gameState = globalTypes::GameState::StartLevel;
    else if (scancode == sf::Keyboard::Scan::Escape) {
        SoundPlayer::instance().stopSound(SoundPlayer::briefingTheme);
        gameState = globalTypes::GameState::TitleScreen;
    }
}