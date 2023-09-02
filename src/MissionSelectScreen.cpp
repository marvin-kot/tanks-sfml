#include "GlobalConst.h"
#include "MissionSelectScreen.h"
#include "MapCreator.h"
#include "SoundPlayer.h"
#include "UiUtils.h"
#include "Utils.h"

#include <SFML/Graphics.hpp>

#include <cassert>
#include <filesystem>


MissionSelectScreen::MissionSelectScreen() {}

MissionSelectScreen& MissionSelectScreen::instance()
{
    static MissionSelectScreen _instance;
    return _instance;
}


void MissionSelectScreen::open()
{
    _missions.clear();

    namespace fs = std::filesystem;

    const fs::path dir("./assets/maps/");

    assert(fs::exists(dir) == true);
    assert(fs::is_directory(dir) == true);

    MapCreator mapCreator;

    for (const auto& entry : fs::directory_iterator(dir)) {
        if (!fs::is_regular_file(entry.status()))
            continue; // ignore directories

        std::string filename = entry.path().root_path().string() + entry.path().relative_path().string();

        mapCreator.parseMapFile(filename);
        auto props = mapCreator.levelProperties();

        MapInfo info;
        info.filename = filename;
        info.title = props.name;

        char formattedStr[100];
        const char *placeholder = Level::winDescriptionsMap.at(props.win);
        sprintf(formattedStr, placeholder, props.winParam);
        info.briefDescription = std::string(formattedStr);

        _missions.push_back(info);
        maxUnlockedMissions++;
    }

    _cursorPos = 0;
}

int MissionSelectScreen::draw()
{
    using namespace globalConst;
    using namespace globalVars;

    constexpr int screenCenterX = screen_w / 2;
    constexpr int screenCenterY = screen_h / 2;

    constexpr int screenQuarterY = screen_h / 4;

    const sf::Color greyColor = sf::Color(150, 150, 150);

    // draw frame
    sf::RectangleShape blackRect(sf::Vector2f(screen_w, screen_h));
    blackRect.setOrigin(blackRect.getSize().x/2, blackRect.getSize().y/2);
    blackRect.setPosition(sf::Vector2f(screenCenterX, screenCenterY));
    blackRect.setFillColor(sf::Color(0, 0, 0));
    Utils::window.draw(blackRect);

    const int menuWidth = blackRect.getSize().x;
    const int menuHeight = blackRect.getSize().y;

    int currentStringY = screenCenterY - blackRect.getSize().y/2 - 32;

    // draw title
    const int titleFontSize = 48;
    currentStringY += titleFontSize * 2;

    std::string title = "Select a mission";
    UiUtils::instance().drawText( title, titleFontSize, screenCenterX, currentStringY), true;

    currentStringY += titleFontSize + 32;
    const int lineSize = 34;

    UiUtils::instance().drawHorizontalLine(screenCenterX, currentStringY, globalConst::screen_w-64, greyColor);

    currentStringY += titleFontSize*2;

    for (int i = 0; i < _missions.size(); i++) {
        std::string line = std::to_string(i+1) + ". " + _missions[i].title;
        UiUtils::instance().drawText( line, lineSize, menuWidth/5-64, currentStringY, true, _cursorPos == i ? sf::Color::Yellow : sf::Color::White) ;
        currentStringY += lineSize ;
        UiUtils::instance().drawText("     " + _missions[i].briefDescription, lineSize*2/3, menuWidth/5-64, currentStringY, true, _cursorPos == i ? sf::Color::Yellow : sf::Color::White) ;
        currentStringY += lineSize * 2;
    }

    Utils::window.display();

    return 0;
}

void MissionSelectScreen::moveCursorDown()
{
    SoundPlayer::instance().playSound(SoundPlayer::SoundType::moveCursor);
    if (++_cursorPos >= maxUnlockedMissions)
        _cursorPos = 0;
}

void MissionSelectScreen::moveCursorUp()
{
    SoundPlayer::instance().playSound(SoundPlayer::SoundType::moveCursor);
    if (--_cursorPos < 0)
        _cursorPos = maxUnlockedMissions-1;
}

void MissionSelectScreen::selectLevel()
{
    SoundPlayer::instance().playSound(SoundPlayer::SoundType::startGame);
}

