#include "Game.h"

#include "AssetManager.h"
#include "GlobalConst.h"
#include "ObjectsPool.h"
#include "Logger.h"
#include "MapCreator.h"
#include "SoundPlayer.h"
#include "Utils.h"

#include <SFML/Graphics.hpp>


std::vector<std::string> levelMaps = {
    "assets/testmap.txt",
    "assets/testmap2.txt",
    "assets/testmap3.txt"};


Game::Game() {}

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
    //window.setVerticalSyncEnabled(true);
    Utils::window.setFramerateLimit(30);
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
    processDeletedObjects();

    drawGameScreen();
    drawObjects();
    updateDisplay();
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
                } else if (gameState == PlayingLevel && event.key.scancode == sf::Keyboard::Scan::Escape) {
                    gameState = GameOver;
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

        Logger::instance() << "Building map..." << levelMaps[currentLevel];
        if (!buildLevelMap(levelMaps[currentLevel])) {
            Utils::window.close();
            Logger::instance() << "Failed to build map";
            return -1;
        }
        Logger::instance() << "Level map is built";
        SoundPlayer::instance().gameOver = false;
        gameState = PlayingLevel;
        return 0;
    } else if (gameState == GameOver) {
        SoundPlayer::instance().stopAllSounds();
        ObjectsPool::clearEverything();
        gameState = TitleScreen;
        return 0;
    }

    return 1;
}

bool Game::buildLevelMap(std::string fileName)
{
    MapCreatorFromCustomMatrixFile mapBuilder;
    mapBuilder.parseMapFile(fileName);
    if (mapBuilder.buildMapFromData() == -1) {
        return false;
    }

    if (mapBuilder.mapWidth() > globalConst::maxFieldWidth || mapBuilder.mapHeight() > globalConst::maxFieldHeight) {
        Logger::instance() << "[ERROR] the map size exceeds the limits of the screen. Aborting game..." << fileName;
        return false;
    }

    return true;
}

void Game::updateAllObjectControllers()
{
    auto &allObjects = ObjectsPool::getAllObjects();
    // update object states
    for (GameObject *obj : allObjects) {
        if (!obj->isFlagSet(GameObject::Static))
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
                    framesToDie = globalConst::MaxFramesToDie;
                    SoundPlayer::instance().stopAllSounds();
                    SoundPlayer::instance().gameOver = true;
                    // do not break here - we still need to create explosion few lines later!
                }

                if (obj->isFlagSet(GameObject::Bullet)) {
                    GameObject *explosion = new GameObject("smallExplosion");
                    explosion->setRenderer(new OneShotAnimationRenderer(explosion));
                    explosion->setFlags(GameObject::BulletPassable | GameObject::TankPassable);
                    explosion->copyParentPosition(obj);

                    objectsToAdd.push_back(explosion);

                    if (obj->getParentObject()->isFlagSet(GameObject::Player))
                        SoundPlayer::instance().playBulletHitWallSound();
                }

                if (obj->isFlagSet(GameObject::Player | GameObject::NPC | GameObject::Eagle)) {
                    GameObject *explosion = new GameObject("bigExplosion");
                    explosion->setRenderer(new OneShotAnimationRenderer(explosion));
                    explosion->setFlags(GameObject::BulletPassable | GameObject::TankPassable);
                    explosion->copyParentPosition(obj);

                    objectsToAdd.push_back(explosion);

                    if (obj->isFlagSet(GameObject::Player | GameObject::Eagle))
                        SoundPlayer::instance().playBigExplosionSound();
                    else
                        SoundPlayer::instance().playSmallExplosionSound();
                }

                if (obj->isFlagSet(GameObject::BonusOnHit)) {
                    obj->generateDrop();
                    SoundPlayer::instance().playBonusAppearSound();
                }
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
    sf::RectangleShape blackRect(sf::Vector2f(globalVars::gameViewPort.width, globalVars::gameViewPort.height));
    blackRect.setPosition(sf::Vector2f(globalVars::gameViewPort.left, globalVars::gameViewPort.top));
    blackRect.setFillColor(sf::Color(50, 0, 0));
    Utils::window.draw(blackRect);
}

void Game::drawObjects()
{
    // 1. draw ice and water
    auto objectsToDrawFirst = ObjectsPool::getObjectsByTypes({"ice", "water"});
    std::for_each(objectsToDrawFirst.cbegin(), objectsToDrawFirst.cend(), [](GameObject *obj) { obj->draw(); });

    // 2. draw tanks and bullets
    std::unordered_set<GameObject *> objectsToDrawSecond = ObjectsPool::getObjectsByTypes({"player", "eagle", "npcBaseTank", "npcFastTank", "npcPowerTank", "npcArmorTank", "bullet"});
    std::for_each(objectsToDrawSecond.begin(), objectsToDrawSecond.end(), [](GameObject *obj) { if (obj) obj->draw(); });

    // 3. draw walls and trees
    auto objectsToDrawThird = ObjectsPool::getObjectsByTypes({"brickWall", "concreteWall", "tree", "smallExplosion", "bigExplosion"});
    std::for_each(objectsToDrawThird.cbegin(), objectsToDrawThird.cend(), [](GameObject *obj) { obj->draw(); });

    // 4. visual effects
    auto objectsToDrawFourth = ObjectsPool::getObjectsByTypes({
        "spawner_player", "spawner_BaseTank", "spawner_FastTank", "spawner_PowerTank", "spawner_ArmorTank",
        "helmetCollectable", "timerCollectable", "shovelCollectable", "starCollectable", "grenadeCollectable", "tankCollectable"
        });
    std::for_each(objectsToDrawFourth.cbegin(), objectsToDrawFourth.cend(), [](GameObject *obj) { obj->draw(); });
}

void Game::updateDisplay()
{
    Utils::window.display();
}

void Game::checkStatePostFrame()
{
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
    int countEnemiesAlive = ObjectsPool::countObjectsByTypes({
        "npcBaseTank", "npcFastTank", "npcPowerTank", "npcArmorTank",
        "spawner_BaseTank", "spawner_FastTank", "spawner_PowerTank", "spawner_ArmorTank"});
    if (countEnemiesAlive < 1 && framesToWin == -1) {
        framesToWin = globalConst::MaxFramesToWin;
        SoundPlayer::instance().stopAllSounds();
        SoundPlayer::instance().gameOver = true;
    }

    int countPlayerObjects = ObjectsPool::countObjectsByTypes({"player", "spawner_player"});
    if (countPlayerObjects < 1 && framesToDie == -1) {
        framesToDie = globalConst::MaxFramesToDie;
        SoundPlayer::instance().stopAllSounds();
        SoundPlayer::instance().gameOver = true;
    }
}

void Game::drawTitleScreen()
{
    sf::Text text;
    text.setFont(AssetManager::instance().defaultFont());
    std::string title = "RETRO TANK MASSACRE";
    text.setString(title);
    const int titleSize = 96;
    text.setCharacterSize(titleSize);
    text.setFillColor(sf::Color::White);
    text.setOrigin(titleSize * title.length() / 2.5, titleSize/2);
    text.setPosition(globalConst::screen_w/2, globalConst::screen_h/2 - titleSize);

    sf::Text textSub;
    std::string subtitle = "version  0.1";
    textSub.setFont(AssetManager::instance().defaultFont());
    textSub.setString(subtitle);
    textSub.setCharacterSize(titleSize/6);
    textSub.setFillColor(sf::Color::White);
    textSub.setOrigin(titleSize/6 * subtitle.length() / 4, titleSize/4/2);
    textSub.setPosition(globalConst::screen_w/2, globalConst::screen_h/2 + 12);

    sf::Text textInstruction;
    std::string instruction = "Press [space] to start the game";
    textInstruction.setFont(AssetManager::instance().defaultFont());
    textInstruction.setString(instruction);
    textInstruction.setCharacterSize(titleSize/4);
    textInstruction.setFillColor(sf::Color::Yellow);
    textInstruction.setOrigin(titleSize/4 * instruction.length() / 2.5, titleSize/2/2);
    textInstruction.setPosition(globalConst::screen_w/2, globalConst::screen_h/2 + 12 + titleSize + 12);

    Utils::window.clear();

    sf::RectangleShape blackRect(sf::Vector2f(globalConst::screen_w, globalConst::screen_h));
    blackRect.setFillColor(sf::Color(0, 0, 0));

    Utils::window.draw(blackRect);
    Utils::window.draw(text);
    Utils::window.draw(textSub);
    Utils::window.draw(textInstruction);
    Utils::window.display();
}
