#include "Game.h"

#include "AssetManager.h"
#include "BonusShopWindow.h"
#include "GameObject.h"
#include "GameOverScreen.h"
#include "GlobalConst.h"
#include "HUD.h"
#include "LevelUpPopupMenu.h"
#include "Logger.h"
#include "MissionSelectScreen.h"
#include "ObjectsPool.h"
#include "PersistentGameData.h"
#include "PlayerController.h"
#include "SoundPlayer.h"
#include "TitleScreen.h"
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

using namespace globalTypes;


Game::Game() : gameState(GameState::TitleScreen) {}

bool Game::loadAssets()
{
    if (!AssetManager::instance().loadSpriteSheet("spriteSheet32x32")) {
        Logger::instance() << "[ERROR] Could not open assets/spriteSheet32x32.png";
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
    Utils::window.setKeyRepeatEnabled(false);
    return true;
}

void Game::initializeVariables()
{
    currentLevel = 0;
    framesToDie = -1;
    framesToWin = -1;
    _killsCount = 0;
    PersistentGameData::instance().loadDataFromDisk();
    PlayerUpgrade::generatePerks();
    gameState = GameState::TitleScreen;
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

    if (ObjectsPool::bossObject && !globalVars::gameIsPaused)
        SoundPlayer::instance().playBossTheme();
    // play sounds
    SoundPlayer::instance().processQueuedSounds();

    drawGameScreen();
    if (!globalVars::gameIsPaused)
        recalculateViewPort();
    drawObjects();
    drawBorders();
    HUD::instance().draw();



    if (LevelUpPopupMenu::instance().isOpen()) {
        SoundPlayer::instance().pauseAllSounds();
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

    static int i=0;
    static int frameTimes[512];

    if (i < 300) {
        frameTimes[i] = Utils::lastFrameTime.asMilliseconds();
        i++;
    } else {
        i=0;
        Logger::instance() << "last 500 frames: ";
        for (int j=0;j<300;j++)
            Logger::instance() << frameTimes[j] << ", ";
        Logger::instance() << "\n";
    }
}

void Game::processWindowEvents()
{
    sf::Event event;
    if (Utils::window.pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                PlayerUpgrade::deletePerks();
                PersistentGameData::instance().saveDataToDisk();
                Utils::window.close();
                break;
            case sf::Event::KeyPressed:
                if (gameState == GameState::TitleScreen) {
                    TitleScreen::instance().processKeyboardPress(event.key.scancode);
                } else if (gameState == GameState::StartLevelScreen) {
                    if (event.key.scancode == sf::Keyboard::Scan::Enter)
                        gameState = GameState::StartLevel;
                    else if (event.key.scancode == sf::Keyboard::Scan::Escape) {
                        SoundPlayer::instance().stopSound(SoundPlayer::briefingTheme);
                        gameState = GameState::TitleScreen;
                    }
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
                        gameState = GameState::GameOver;
                    } else if (event.key.scancode == sf::Keyboard::Scan::Pause || event.key.scancode == sf::Keyboard::Scan::Enter) {
                        pause(!globalVars::gameIsPaused);
                    }
                } else if (gameState == BonusShop) {
                    if (event.key.scancode == sf::Keyboard::Scan::Left)
                        BonusShopWindow::instance().moveCursorLeft();
                    else if (event.key.scancode == sf::Keyboard::Scan::Right)
                        BonusShopWindow::instance().moveCursorRight();
                    else if (event.key.scancode == sf::Keyboard::Scan::Down)
                        BonusShopWindow::instance().moveCursorDown();
                    else if (event.key.scancode == sf::Keyboard::Scan::Up)
                        BonusShopWindow::instance().moveCursorUp();
                    else if (event.key.scancode == sf::Keyboard::Scan::Enter)
                        BonusShopWindow::instance().getSelectedUpgrade();
                    else if (event.key.scancode == sf::Keyboard::Scan::Escape) {
                        SoundPlayer::instance().stopSound(SoundPlayer::ShopTheme);
                        gameState = GameState::TitleScreen;
                    }
                    else if (event.key.scancode == sf::Keyboard::Scan::Space) {
                        SoundPlayer::instance().stopSound(SoundPlayer::ShopTheme);
                        gameState = GameState::SelectLevel;
                    }
                } else if (gameState == GameState::SelectLevel) {
                    if (event.key.scancode == sf::Keyboard::Scan::Down)
                        MissionSelectScreen::instance().moveCursorDown();
                    else if (event.key.scancode == sf::Keyboard::Scan::Up)
                        MissionSelectScreen::instance().moveCursorUp();
                    else if (event.key.scancode == sf::Keyboard::Scan::Enter) {
                        MissionSelectScreen::instance().selectLevel();
                        gameState = GameState::LoadNextLevel;
                    } else if (event.key.scancode == sf::Keyboard::Scan::Escape) {
                        if (PersistentGameData::instance().isShopUnlocked())
                            gameState = GameState::BonusShop;
                        else
                            gameState = GameState::TitleScreen;
                    }
                }
                else if (gameState == GameState::GameOverScreen) {
                    if (event.key.scancode == sf::Keyboard::Scan::Enter || event.key.scancode == sf::Keyboard::Scan::Enter || event.key.scancode == sf::Keyboard::Scan::Escape)
                        GameOverScreen::instance().increaseCountSpeed();
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
        SoundPlayer::instance().pauseAllSounds();
        SoundPlayer::instance().playSound(SoundPlayer::SoundType::pause);
        globalChronometer.pause();
        globalFreezeChronometer.pause();
    } else {
        globalChronometer.resume();
        globalFreezeChronometer.resume();
    }
}

int Game::processStateChange()
{
    if (gameState == GameState::TitleScreen) {
        gameState = TitleScreen::instance().draw();
        return 0;
    } else if (gameState == SelectLevel) {
        MissionSelectScreen::instance().draw();
        return 0;
    }
    else if (gameState == LoadNextLevel) {
        _won = false; framesToWin = -1; framesToDie = -1;
        SoundPlayer::instance().stopAllSounds();
        ObjectsPool::clearEverything();

        globalVars::player1Lives = globalConst::InitialLives;
        globalVars::player1PowerLevel = globalConst::InitialPowerLevel;

        std::string filename = MissionSelectScreen::instance().getSelectedFilename();

        Logger::instance() << "Building map..." << filename << "\n";
        if (!buildLevelMap(filename)) {
            PlayerUpgrade::deletePerks();
            PersistentGameData::instance().saveDataToDisk();
            Utils::window.close();
            Logger::instance() << "Failed to build map\n";
            return -1;
        }
        Logger::instance() << "Level map is built\n";
        gameState = GameState::StartLevelScreen;
        SoundPlayer::instance().playSound(SoundPlayer::briefingTheme);
        _killsCount = 0;
        return 0;
    } else if (gameState == GameState::StartLevelScreen) {
        drawStartLevelScreen();
        return 0;
    }
    else if (gameState == GameState::StartLevel) {
        SoundPlayer::instance().stopSound(SoundPlayer::briefingTheme);
        SoundPlayer::instance().gameOver = false;
        HUD::instance().showFail(false);
        HUD::instance().showWin(false);
        globalVars::globalChronometer.reset(true);
        _killsCount = 0;
        gameState = GameState::PlayingLevel;
    }
    else if (gameState == GameState::GameOver) {
        SoundPlayer::instance().stopAllSounds();
        ObjectsPool::clearEverything();
        gameState = GameState::GameOverScreen;
        const int missionIndex = MissionSelectScreen::instance().getSelectedIndex();
        GameOverScreen::instance().setMissionOutcome(_won, missionIndex, _killsCount, (int)_finishTime.asSeconds());
        PlayerUpgrade::playerOwnedPerks.clear();
        return 0;
    } else if (gameState == GameState::GameOverScreen) {
        gameState = GameOverScreen::instance().draw();
        if (gameState == GameState::BonusShop) {
            if (_won)
                BonusShopWindow::instance().afterWin = true;
            else
                BonusShopWindow::instance().afterGameOver = true;
            PlayerUpgrade::generatePerks();
        }
        return 0;
    } else if (gameState == GameState::BonusShop) {
        SoundPlayer::instance().playShopTheme();
        BonusShopWindow::instance().draw();
        return 0;
    } else if (gameState == GameState::ExitGame) {
        PlayerUpgrade::deletePerks();
        PersistentGameData::instance().saveDataToDisk();
        Utils::window.close();
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
        while (!ObjectsPool::objectsToDelete.empty()) {
            GameObject *obj = ObjectsPool::objectsToDelete.front();

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
                if (obj->isFlagSet(GameObject::Explosive)) {
                    GameObject *explosion = new GameObject("bigExplosion");
                    explosion->setController(new ExplosionController(explosion, false));
                    explosion->setRenderer(new OneShotAnimationRenderer(explosion), 4);
                    explosion->setFlags(GameObject::BulletPassable | GameObject::TankPassable);
                    explosion->copyParentPosition(obj);
                    explosion->damage = 1;
                    objectsToAdd.push_back(explosion);
                    SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::smallExplosion, true);
                } else {
                    GameObject *explosion = new GameObject("smallExplosion");
                    explosion->setRenderer(new OneShotAnimationRenderer(explosion), 4);
                    explosion->setFlags(GameObject::BulletPassable | GameObject::TankPassable);
                    explosion->copyParentPosition(obj);

                    objectsToAdd.push_back(explosion);
                }

                if (obj->getParentObject() && obj->getParentObject()->isFlagSet(GameObject::Player))
                    SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::bulletHitWall, true);
            }

            if (obj->isFlagSet(GameObject::Player | GameObject::NPC | GameObject::Eagle)) {
                GameObject *explosion = new GameObject("bigExplosion");
                explosion->setRenderer(new OneShotAnimationRenderer(explosion), 4);
                explosion->setFlags(GameObject::BulletPassable | GameObject::TankPassable);
                if (obj->isFlagSet(GameObject::Explosive)) {
                    explosion->setController(new ExplosionController(explosion, false));
                    explosion->damage = 1;
                }
                explosion->copyParentPosition(obj);

                objectsToAdd.push_back(explosion);

                if (obj->isFlagSet(GameObject::Player | GameObject::Eagle | GameObject::Boss))
                    SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::bigExplosion, true);
                else
                    SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::smallExplosion, true);
            }

            if (obj->isFlagSet(GameObject::NPC) && !obj->isFlagSet(GameObject::Explosive)) {
                _killsCount++;
            }

            if (obj->isFlagSet(GameObject::BonusOnHit)) {
                obj->generateDrop();
                SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::bonusAppear, true);
            } else
                obj->dropXp();

            ObjectsPool::kill(obj);
            ObjectsPool::objectsToDelete.pop();
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
    //sf::RectangleShape greyRect(sf::Vector2f(screen_w, screen_h));
    //greyRect.setFillColor(sf::Color(102, 102, 102));
    //Utils::window.draw(greyRect);

    // draw view port

    int w = std::min<int>(globalVars::mapSize.x*globalConst::spriteDisplaySizeX, globalVars::gameViewPort.width);
    int h = std::min<int>(globalVars::mapSize.y*globalConst::spriteDisplaySizeY, globalVars::gameViewPort.height);
    sf::RectangleShape blackRect(sf::Vector2f(w, h));
    blackRect.setPosition(sf::Vector2f(globalVars::gameViewPort.left, globalVars::gameViewPort.top));
    blackRect.setFillColor(sf::Color(50, 0, 0));
    Utils::window.draw(blackRect);
}

void Game::drawBorders()
{
    //using namespace globalConst;
    // draw 4 grey rects
    const int viewPortWidth = std::min<int>(globalVars::mapSize.x*globalConst::spriteDisplaySizeX, globalVars::gameViewPort.width);
    const int viewPortHeight = std::min<int>(globalVars::mapSize.y*globalConst::spriteDisplaySizeY, globalVars::gameViewPort.height);
    const int borderWidth = (globalConst::screen_w - viewPortWidth)/2;
    const int borderHeight = (globalConst::screen_h - viewPortHeight)/2;
    {
        sf::RectangleShape greyRect(sf::Vector2f(borderWidth, globalConst::screen_h));
        greyRect.setPosition(0, 0);
        greyRect.setFillColor(sf::Color(102, 102, 102));
        Utils::window.draw(greyRect);
    }
    {
        sf::RectangleShape greyRect(sf::Vector2f(borderWidth, globalConst::screen_h));
        greyRect.setPosition(globalConst::screen_w-borderWidth, 0);
        greyRect.setFillColor(sf::Color(102, 102, 102));
        Utils::window.draw(greyRect);
    }
    {
        sf::RectangleShape greyRect(sf::Vector2f(viewPortWidth, borderHeight));
        greyRect.setPosition(borderWidth, 0);
        greyRect.setFillColor(sf::Color(102, 102, 102));
        Utils::window.draw(greyRect);
    }
    {
        sf::RectangleShape greyRect(sf::Vector2f(viewPortWidth, borderHeight));
        greyRect.setPosition(borderWidth, borderHeight+viewPortHeight);
        greyRect.setFillColor(sf::Color(102, 102, 102));
        Utils::window.draw(greyRect);
    }
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
    auto objectsToDrawFirst = ObjectsPool::getObjectsByDrawOrder(1);
    std::for_each(objectsToDrawFirst.cbegin(), objectsToDrawFirst.cend(), [](GameObject *obj) { obj->draw(); });

    // 2. draw tanks and bullets
    std::unordered_set<GameObject *> objectsToDrawSecond = ObjectsPool::getObjectsByDrawOrder(2);
    std::for_each(objectsToDrawSecond.begin(), objectsToDrawSecond.end(), [&](GameObject *obj) { if (obj) obj->draw(); });

    // 3. draw walls and trees
    auto objectsToDrawThird = ObjectsPool::getObjectsByDrawOrder(3);
    std::for_each(objectsToDrawThird.cbegin(), objectsToDrawThird.cend(), [](GameObject *obj) { obj->draw(); });

    // 4. visual effects
    auto objectsToDrawFourth = ObjectsPool::getObjectsByDrawOrder(4);
    std::for_each(objectsToDrawFourth.cbegin(), objectsToDrawFourth.cend(), [&](GameObject *obj) { obj->draw(); });
}

void Game::updateDisplay()
{
    Utils::window.display();
}

void Game::checkStatePostFrame()
{
    if (gameState != GameState::PlayingLevel)
        return;

    if (framesToDie > -1) {
        if (--framesToDie <= 0) {
            gameState = GameState::GameOver;
            _won = false;
        }
    }

    if (framesToWin > -1) {
        if (--framesToWin <= 0) {
            gameState = GameState::GameOver;
            _won = true;
        }
    }

    // as the game just cleared all objects marked for deletion, all the existing enemies must be alive at this point
    if (framesToWin == -1 && framesToDie == -1 && winConditionsMet()) {
        // instant kill all enemies
        auto enemiesAlive = ObjectsPool::getObjectsByTypes({
            "npcBaseTank", "npcFastTank", "npcPowerTank", "npcArmorTank", "npcDoubleCannonArmorTank", "npcKamikazeTank",
            "spawner_npcBaseTank", "spawner_npcFastTank", "spawner_npcPowerTank", "spawner_npcArmorTank", "spawner_npcGiantTank", "spawner_npcDoubleCannonArmorTank", "spawner_npcKamikazeTank"});
        for (auto enemy : enemiesAlive) enemy->markForDeletion();

        framesToWin = globalConst::MaxFramesToWin;
        SoundPlayer::instance().stopAllSounds();
        SoundPlayer::instance().gameOver = true;
        SoundPlayer::instance().playSound(SoundPlayer::SoundType::win);
        HUD::instance().showWin(true);
        _finishTime = globalVars::globalChronometer.getElapsedTime();
    }

    if (framesToWin == -1 && framesToDie == -1 && failConditionsMet()) {
        framesToDie = globalConst::MaxFramesToDie;
        SoundPlayer::instance().stopAllSounds();
        SoundPlayer::instance().gameOver = true;
        SoundPlayer::instance().playSound(SoundPlayer::SoundType::fail);
        HUD::instance().showFail(true);
        _finishTime = globalVars::globalChronometer.getElapsedTime();
    }
}

bool Game::winConditionsMet() const
{
    if (_currentLevelProperties.win == Level::KillEmAll) {
        int countEnemiesAlive = ObjectsPool::countObjectsByTypes({
            "npcBaseTank", "npcFastTank", "npcPowerTank", "npcArmorTank", "npcGiantTank", "npcKamikazeTank",
            "spawner_npcBaseTank", "spawner_npcFastTank", "spawner_npcPowerTank", "spawner_npcArmorTank", "spawner_npcGiantTank"});
        if (countEnemiesAlive < 1)
            return true;

    } else if (_currentLevelProperties.win == Level::SurviveTime) {
        const int secondsToWin = _currentLevelProperties.winParam * 60;

        if (ObjectsPool::bossObject != nullptr)
            return false;
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
        "Press [enter] to start", 24,
        screenCenterX, currentStringY, false,
        sf::Color::Yellow);
    Utils::window.display();
}


}