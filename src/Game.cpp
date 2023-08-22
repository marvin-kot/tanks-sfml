#include "Game.h"

#include "AssetManager.h"
#include "GameObject.h"
#include "GlobalConst.h"
#include "HUD.h"
#include "LevelUpPopupMenu.h"
#include "ObjectsPool.h"
#include "PlayerController.h"
#include "Logger.h"
#include "SoundPlayer.h"
#include "Utils.h"
#include "UiUtils.h"

#include <SFML/Graphics.hpp>

#include <format>

namespace allinone {

std::vector<std::string> levelMaps = {
    "assets/testmap4.txt",
    //"assets/testmap.txt",
    //"assets/testmap2.txt"
};


Game::Game() : gameState(TitleScreen) {}

bool Game::loadAssets()
{
    if (!AssetManager::instance().loadSpriteSheet("spriteSheet16x16")) {
        Logger::instance() << "[ERROR] Could not open assets/spriteSheet16x16.png";
        return false;
    }


    if (!AssetManager::instance().loadFont("assets/joystix.otf")) {
        Logger::instance() << "[ERROR] fonts not loaded";
        return false;
    }

    return true;
}

bool Game::initializeWindow()
{
    using namespace globalConst;
    Utils::window.create(sf::VideoMode(screen_w, screen_h), "Retro Tank Massacre SFML");
    //Utils::window.setVerticalSyncEnabled(true);
    Utils::window.setFramerateLimit(FixedFrameRate);
    return true;
}

void Game::initializeVariables()
{
    currentLevel = 0;
    framesToDie = -1;
    framesToWin = -1;
    gameState = TitleScreen;
}

bool Game::update()
{
    updateFrameClock();
    processWindowEvents();
    int stateResult = processStateChange();
    if (stateResult == -1) {
        return false;
    } else if (stateResult == 0) {
        return true;
    }
    // else if 1 - proceed

    updateAllObjectControllers();

    if (!globalVars::gameIsPaused) {
        processDeletedObjects();
    }

    // play sounds
    SoundPlayer::instance().processQueuedSounds();

    drawGameScreen();
    if (!globalVars::gameIsPaused)
        recalculateViewPort();
    drawObjects();
    HUD::instance().draw();



    if (LevelUpPopupMenu::instance().isOpen()) {
        SoundPlayer::instance().stopAllSounds();
        LevelUpPopupMenu::instance().draw();
    }

    updateDisplay();

    if (!globalVars::gameIsPaused)
        checkStatePostFrame();

    return true;
}

bool Game::isWindowOpen() const
{
    return Utils::window.isOpen();
}

void Game::updateFrameClock()
{
    Utils::currentFrame++;
    Utils::lastFrameTime = Utils::refreshClock.getElapsedTime();
    Utils::refreshClock.restart();
}

void Game::processWindowEvents()
{
    sf::Event event;
    while (Utils::window.pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                Utils::window.close();
                break;
            case sf::Event::KeyPressed:
                if (gameState == TitleScreen) {
                    if (event.key.scancode == sf::Keyboard::Scan::Escape)
                        Utils::window.close();
                    else if (event.key.scancode == sf::Keyboard::Scan::Space) {
                        currentLevel = 0;
                        globalVars::player1Lives = globalConst::InitialLives;
                        globalVars::player1PowerLevel = globalConst::InitialPowerLevel;
                        gameState = LoadNextLevel;
                    }
                } else if (gameState == StartLevelScreen) {
                    if (event.key.scancode == sf::Keyboard::Scan::Space)
                        gameState = StartLevel;
                    else if (event.key.scancode == sf::Keyboard::Scan::Escape)
                        gameState = GameOver;
                } else if (gameState == PlayingLevel) {
                    if (LevelUpPopupMenu::instance().isOpen()) {
                        if (event.key.scancode == sf::Keyboard::Scan::Enter) {
                            LevelUpPopupMenu::instance().getSelectedUpgrade();
                            LevelUpPopupMenu::instance().close();
                        } else if (event.key.scancode == sf::Keyboard::Scan::Left)
                            LevelUpPopupMenu::instance().moveCursorLeft();
                        if (event.key.scancode == sf::Keyboard::Scan::Right)
                            LevelUpPopupMenu::instance().moveCursorRight();
                        break; // do not process further!
                    }

                    if (event.key.scancode == sf::Keyboard::Scan::Escape) {
                        // TODO: ask player confirmation
                        gameState = GameOver;
                    } else if (event.key.scancode == sf::Keyboard::Scan::Pause || event.key.scancode == sf::Keyboard::Scan::P) {
                        pause(!globalVars::gameIsPaused);
                    }
                }
                break;
            case sf::Event::Resized:
                std::cout << "new width: " << event.size.width << std::endl;
                std::cout << "new height: " << event.size.height << std::endl;
                break;
            default:
                break;
        }
    }
}

void Game::pause(bool p)
{
    using namespace globalVars;
    gameIsPaused = p;
    if (p) {
        SoundPlayer::instance().stopAllSounds();
        SoundPlayer::instance().playPauseSound();
        globalChronometer.pause();
        globalFreezeChronometer.pause();
    } else {
        globalChronometer.resume();
        globalFreezeChronometer.resume();
    }
}

int Game::processStateChange()
{
    if (gameState == TitleScreen) {
        drawTitleScreen();
        return 0;
    } else if (gameState == LoadNextLevel) {
        if (currentLevel >= levelMaps.size()) {
            gameState = TitleScreen;
            return 0;
        }

        framesToWin = -1; framesToDie = -1;
        SoundPlayer::instance().stopAllSounds();
        ObjectsPool::clearEverything();

        globalVars::player1Lives = globalConst::InitialLives;
        globalVars::player1PowerLevel = globalConst::InitialPowerLevel;

        Logger::instance() << "Building map..." << levelMaps[currentLevel] << "\n";
        if (!buildLevelMap(levelMaps[currentLevel])) {
            Utils::window.close();
            Logger::instance() << "Failed to build map\n";
            return -1;
        }
        Logger::instance() << "Level map is built\n";
        gameState = StartLevelScreen;
        return 0;
    } else if (gameState == StartLevelScreen) {
        drawStartLevelScreen();
        return 0;
    }
    else if (gameState == StartLevel) {
        SoundPlayer::instance().gameOver = false;
        HUD::instance().showFail(false);
        HUD::instance().showWin(false);
        globalVars::globalChronometer.reset(true);
        gameState = PlayingLevel;
    }
    else if (gameState == GameOver) {
        SoundPlayer::instance().stopAllSounds();
        ObjectsPool::clearEverything();
        gameState = TitleScreen;
        return 0;
    }

    return 1;
}

bool Game::buildLevelMap(std::string fileName)
{
    MapCreator mapBuilder;
    mapBuilder.parseMapFile(fileName);
    _currentLevelProperties = mapBuilder.buildMapFromData();
    if (_currentLevelProperties.failedToLoad)
        return false;

    if (mapBuilder.mapWidth() > globalConst::maxFieldWidth || mapBuilder.mapHeight() > globalConst::maxFieldHeight) {
        Logger::instance() << "[ERROR] the map size exceeds the limits of the screen. Aborting game..." << fileName;
        return false;
    }

    mapBuilder.placeSpawnerObjects();

    _surviveTimeoutSec = 60 * 5;

    return true;
}

void Game::updateAllObjectControllers()
{
    auto &allObjects = ObjectsPool::getAllObjects();
    // update object states
    for (GameObject *obj : allObjects) {
        obj->update();
    }
}

void Game::processDeletedObjects()
{
    auto &allObjects = ObjectsPool::getAllObjects();
    std::vector<GameObject *> objectsToAdd;

        // delete objects marked for deletion on previous step
        for (auto it = allObjects.begin(); it != allObjects.end(); ) {
            GameObject *obj = *it;
            if (obj->mustBeDeleted()) {
                if (obj->isFlagSet(GameObject::Player)) {
                    globalVars::player1Lives--;
                    ObjectsPool::playerObject = nullptr;
                }

                if (obj->isFlagSet(GameObject::Eagle)) {
                    ObjectsPool::eagleObject = nullptr;
                    // do not break here - we still need to create explosion few lines later!
                }

                if (obj->isFlagSet(GameObject::PlayerSpawner))
                    ObjectsPool::playerSpawnerObject = nullptr;

                if (obj->isFlagSet(GameObject::Bullet)) {
                    GameObject *explosion = new GameObject("smallExplosion");
                    explosion->setRenderer(new OneShotAnimationRenderer(explosion));
                    explosion->setFlags(GameObject::BulletPassable | GameObject::TankPassable);
                    explosion->copyParentPosition(obj);

                    objectsToAdd.push_back(explosion);

                    if (obj->getParentObject()->isFlagSet(GameObject::Player))
                        SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::bulletHitWall, true);
                }

                if (obj->isFlagSet(GameObject::Player | GameObject::NPC | GameObject::Eagle)) {
                    GameObject *explosion = new GameObject("bigExplosion");
                    explosion->setRenderer(new OneShotAnimationRenderer(explosion));
                    explosion->setFlags(GameObject::BulletPassable | GameObject::TankPassable);
                    explosion->copyParentPosition(obj);

                    objectsToAdd.push_back(explosion);

                    if (obj->isFlagSet(GameObject::Player | GameObject::Eagle))
                        SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::bigExplosion, true);
                    else
                        SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::smallExplosion, true);
                }

                if (obj->isFlagSet(GameObject::BonusOnHit)) {
                    obj->generateDrop();
                    SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::bonusAppear, true);
                } else
                    obj->dropXp();

                it = ObjectsPool::kill(it);
                delete obj;
            } else ++it;
        }

        // temporary explosion effects
        for (auto obj : objectsToAdd) {
            ObjectsPool::addObject(obj);
        }
}


void Game::drawGameScreen()
{
    Utils::window.clear();
    // draw objects (order matter)
    // draw border
    using namespace globalConst;
    sf::RectangleShape greyRect(sf::Vector2f(screen_w, screen_h));
    greyRect.setFillColor(sf::Color(102, 102, 102));
    Utils::window.draw(greyRect);

    // draw view port

    int w = std::min<int>(globalVars::mapSize.x*globalConst::spriteDisplaySizeX, globalVars::gameViewPort.width);
    int h = std::min<int>(globalVars::mapSize.y*globalConst::spriteDisplaySizeY, globalVars::gameViewPort.height);
    sf::RectangleShape blackRect(sf::Vector2f(w, h));
    blackRect.setPosition(sf::Vector2f(globalVars::gameViewPort.left, globalVars::gameViewPort.top));
    blackRect.setFillColor(sf::Color(50, 0, 0));
    Utils::window.draw(blackRect);
}

void Game::recalculateViewPort()
{
    GameObject *centerObject = nullptr;

    if (ObjectsPool::playerObject)
        centerObject = ObjectsPool::playerObject;
    else if (ObjectsPool::playerSpawnerObject)
        centerObject = ObjectsPool::playerSpawnerObject;
    else
        return;

    using namespace globalConst;
    using namespace globalVars;
    // get (mapped) player coordinates
    auto ppos = centerObject->position();
    int mpX = ppos.x * spriteScaleX;
    int mpY = ppos.y * spriteScaleY;

    mapViewPort.left = mpX - mapViewPort.width/2;
    mapViewPort.top = mpY - mapViewPort.height/2;

    if (mapViewPort.left < 0)
        mapViewPort.left = 0;
    if (mapViewPort.top < 0)
        mapViewPort.top = 0;

    int realMapSizeX = mapSize.x * globalConst::spriteScaleX;
    int realMapSizeY = mapSize.y * globalConst::spriteScaleY;
    if (mapViewPort.left + mapViewPort.width > realMapSizeX)
        mapViewPort.left = realMapSizeX - mapViewPort.width;
    if (mapViewPort.top + mapViewPort.height > realMapSizeY)
        mapViewPort.top = realMapSizeY - mapViewPort.height;
}

void Game::drawObjects()
{
    // 1. draw ice and water
    auto objectsToDrawFirst = ObjectsPool::getObjectsByTypes({"ice", "water"});
    std::for_each(objectsToDrawFirst.cbegin(), objectsToDrawFirst.cend(), [](GameObject *obj) { obj->draw(); });

    // 2. draw tanks and bullets
    std::unordered_set<GameObject *> objectsToDrawSecond = ObjectsPool::getObjectsByTypes({"player", "eagle", "npcBaseTank", "npcFastTank", "npcPowerTank", "npcArmorTank", "bullet"});
    std::for_each(objectsToDrawSecond.begin(), objectsToDrawSecond.end(), [&](GameObject *obj) { if (obj) obj->draw(); });

    // 3. draw walls and trees
    auto objectsToDrawThird = ObjectsPool::getObjectsByTypes({
                "brickWall", "brickWall1x1", "brickWall2x1", "brickWall1x2", "brickWall2x2",
                "concreteWall", "tree"});
    std::for_each(objectsToDrawThird.cbegin(), objectsToDrawThird.cend(), [](GameObject *obj) { obj->draw(); });

    // 4. visual effects
    auto objectsToDrawFourth = ObjectsPool::getObjectsByTypes({
        "spawner_player", "spawner_npcBaseTank", "spawner_npcFastTank", "spawner_npcPowerTank", "spawner_npcArmorTank",
        "helmetCollectable", "timerCollectable", "shovelCollectable", "starCollectable", "grenadeCollectable", "tankCollectable",
        "100xp", "200xp", "300xp", "400xp", "500xp",
        "smallExplosion", "bigExplosion"
        });
    std::for_each(objectsToDrawFourth.cbegin(), objectsToDrawFourth.cend(), [&](GameObject *obj) { obj->draw(); });
}

void Game::updateDisplay()
{
    Utils::window.display();
}

void Game::checkStatePostFrame()
{
    if (gameState != PlayingLevel)
        return;

    if (framesToDie > -1) {
        if (--framesToDie <= 0)
            gameState = GameOver;
    }

    if (framesToWin > -1) {
        if (--framesToWin <= 0) {
            gameState =  (++currentLevel < levelMaps.size()) ? LoadNextLevel : GameOver;
        }
    }

    // as the game just cleared all objects marked for deletion, all the existing enemies must be alive at this point
    if (framesToWin == -1 && framesToDie == -1 && winConditionsMet()) {
        // instant kill all enemies
        auto enemiesAlive = ObjectsPool::getObjectsByTypes({
            "npcBaseTank", "npcFastTank", "npcPowerTank", "npcArmorTank",
            "spawner_npcBaseTank", "spawner_npcFastTank", "spawner_npcPowerTank", "spawner_npcArmorTank"});
        for (auto enemy : enemiesAlive) enemy->markForDeletion();

        framesToWin = globalConst::MaxFramesToWin;
        SoundPlayer::instance().stopAllSounds();
        SoundPlayer::instance().gameOver = true;
        SoundPlayer::instance().playWinJingle();
        HUD::instance().showWin(true);
    }

    if (framesToWin == -1 && framesToDie == -1 && failConditionsMet()) {
        framesToDie = globalConst::MaxFramesToDie;
        SoundPlayer::instance().stopAllSounds();
        SoundPlayer::instance().gameOver = true;
        SoundPlayer::instance().playFailJingle();
        HUD::instance().showFail(true);
    }
}

bool Game::winConditionsMet() const
{
    if (_currentLevelProperties.win == Level::KillEmAll) {
        int countEnemiesAlive = ObjectsPool::countObjectsByTypes({
            "npcBaseTank", "npcFastTank", "npcPowerTank", "npcArmorTank",
            "spawner_npcBaseTank", "spawner_npcFastTank", "spawner_npcPowerTank", "spawner_npcArmorTank"});
        if (countEnemiesAlive < 1)
            return true;

    } else if (_currentLevelProperties.win == Level::SurviveTime) {
        const int secondsToWin = _currentLevelProperties.winParam * 60;
        if (globalVars::globalChronometer.getElapsedTime() >= sf::seconds(secondsToWin))
            return true;
    }

    return false;
}

bool Game::failConditionsMet() const
{
    int countPlayerObjects = ObjectsPool::countObjectsByTypes({"player", "spawner_player"});
    if (countPlayerObjects < 1 )
        return true;

    if (_currentLevelProperties.fail == Level::LoseBase) {
        if (ObjectsPool::eagleObject == nullptr)
            return true;
    }

    return false;
}

void Game::drawTitleScreen()
{
    using namespace globalConst;

    constexpr int screenCenterX = screen_w / 2;
    constexpr int screenCenterY = screen_h / 2;

    // draw black rect
    UiUtils::instance().drawRect(sf::IntRect(0, 0, screen_w, screen_h), sf::Color(0, 0, 0));

    // game title
    constexpr int titleFontSize = 96;
    UiUtils::instance().drawText( "RETRO TANK MASSACRE", titleFontSize, screenCenterX, screenCenterY - titleFontSize);

    // game version
    constexpr int versionFontSize = titleFontSize / 6;
    static std::string version = std::format("version {}.{}.{}", GameMajorVersion, GameMinorVersion, GameReleaseVersion);
    UiUtils::instance().drawText(version, versionFontSize, screenCenterX, screenCenterY + versionFontSize);

    // prompt
    constexpr int promptFontSize = titleFontSize / 4;
    UiUtils::instance().drawText( "Press [space] to start the game", promptFontSize,
        screenCenterX, screenCenterY + versionFontSize + titleFontSize + promptFontSize,
        false, sf::Color::Yellow );


    Utils::window.display();
}

void Game::drawStartLevelScreen()
{
    using namespace globalConst;
    constexpr int screenCenterX = screen_w / 2;
    constexpr int screenCenterY = screen_h / 2;

    constexpr int screenQuarterY = screen_h / 4;

    // grey background
    UiUtils::instance().drawRect(sf::IntRect(0, 0, screen_w, screen_h), sf::Color(102, 102, 102));

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
    //std::string_view str = std::string_view(Level::winDescriptionsMap.at(win));
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
    UiUtils::instance().drawText(
        "Press [space] to start", 24,
        screenCenterX, currentStringY, false,
        sf::Color::Yellow);
    Utils::window.display();
}

void Game::drawGameOverScreen()
{
    using namespace globalConst;
    // grey background
    UiUtils::instance().drawRect(sf::IntRect(0, 0, screen_w, screen_h), sf::Color(102, 102, 102));

    UiUtils::instance().drawText(
        "GAME OVER" , 48,
        screen_w/2, screen_h/2 - 64, false,
        sf::Color::White);

    Utils::window.display();
}

}