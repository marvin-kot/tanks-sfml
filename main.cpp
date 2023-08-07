#include <iostream>
#include <fstream>
#include <vector>

#include <SFML/Graphics.hpp>


#include "GlobalConst.h"
#include "GameObject.h"
#include "Logger.h"
#include "Controller.h"
#include "ObjectsPool.h"
#include "Utils.h"
#include "MapCreator.h"
#include "AssetManager.h"
#include "Shootable.h"
#include "SoundPlayer.h"
#include "SpriteRenderer.h"

GameObject *ObjectsPool::playerObject = nullptr;
std::unordered_set<GameObject *> ObjectsPool::allGameObjects = {};
std::unordered_map<std::string, std::unordered_set<GameObject *>> ObjectsPool::objectsByType = {};


std::default_random_engine Utils::generator = {};
sf::RenderWindow Utils::window = {};

int globalVars::borderWidth = 0;
int globalVars::borderHeight = 0;
sf::IntRect globalVars::gameViewPort = sf::IntRect();



bool loadAssets()
{
    // load textures
    if (!AssetManager::instance().loadSpriteSheet("spriteSheet16x16"))
    {
        Logger::instance() << "[ERROR] Could not open assets/spriteSheet16x16.png";
        return false;
    }


    if (!AssetManager::instance().loadFont("assets/joystix.otf"))
    {
        Logger::instance() << "[ERROR] fonts not loaded";
        return false;
    }

    return true;
}

bool buildLevelMap(std::string fileName)
{
    MapCreatorFromCustomMatrixFile mapBuilder;
    mapBuilder.parseMapFile(fileName);
    if (mapBuilder.buildMapFromData() == -1)
    {
        return false;
    }

    if (mapBuilder.mapWidth() > globalConst::maxFieldWidth || mapBuilder.mapHeight() > globalConst::maxFieldHeight)
    {
        Logger::instance() << "[ERROR] assets/testmap.txt - the map size exceeds the limits of the screen. Aborting game...";
        return false;
    }

    return true;
}

enum GameState
{
    TitleScreen = 0,
    LoadNextLevel = 1,
    PlayingLevel = 2,
    GameOver = 3
};

GameState gameState = TitleScreen;
int currentLevel = 0;

std::vector<std::string> levelMaps = {
    "assets/testmap.txt",
    "assets/testmap2.txt",
    "assets/testmap3.txt"};

void showTitleScreen(sf::RenderWindow&);

int main()
{
    using namespace globalConst;

    // initialize logger
    Logger::instance() << "Loading assets...";
    if (!loadAssets())
    {
        return -1;
    }


    Logger::instance() << "Assets are loaded";

    Utils::window.create(sf::VideoMode(screen_w, screen_h), "Retro Tank Massacre SFML");
    sf::RenderWindow& window = Utils::window;
    window.setVerticalSyncEnabled(true);


    Logger::instance() << "Starting the Game...";
    // main loop

    int framesToDie = -1, framesToWin = -1;
    constexpr int MaxFramesToDie = 180;
    constexpr int MaxFramesToWin = 200;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::KeyPressed:
                    if (gameState == TitleScreen) {
                        if (event.key.scancode == sf::Keyboard::Scan::Escape)
                            window.close();
                        else if (event.key.scancode == sf::Keyboard::Scan::Space) {
                            currentLevel = 0;
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


        if (gameState == TitleScreen) {
            showTitleScreen(window);
            continue;
        } else if (gameState == LoadNextLevel) {
            assert (currentLevel < levelMaps.size());
            framesToWin = -1; framesToDie = -1;
            SoundPlayer::instance().stopAllSounds();
            ObjectsPool::clearEverything();

            Logger::instance() << "Building map..." << levelMaps[currentLevel];
            if (!buildLevelMap(levelMaps[currentLevel])) {
                window.close();
                Logger::instance() << "Failed to build map";
                return -1;
            }
            Logger::instance() << "Level map is built";
            SoundPlayer::instance().gameOver = false;
            gameState = PlayingLevel;
            continue;
        } else if (gameState == GameOver) {
            SoundPlayer::instance().stopAllSounds();
            ObjectsPool::clearEverything();
            gameState = TitleScreen;
            continue;
        }

        assert(ObjectsPool::playerObject != nullptr);

        std::vector<GameObject *> objectsToAdd;
        auto &allObjects = ObjectsPool::getAllObjects();
        // update object states
        for (auto it = allObjects.begin(); it != allObjects.end(); ) {
            GameObject *obj = *it;
            if (obj->mustBeDeleted()) {
                it = allObjects.erase(it);

                if (obj->isFlagSet(GameObject::Player | GameObject::Eagle)) {
                    framesToDie = MaxFramesToDie;
                    SoundPlayer::instance().stopAllSounds();
                    SoundPlayer::instance().gameOver = true;
                    // do not break here - we still need to create explosion few lines later!
                }

                if (obj->isFlagSet(GameObject::Bullet)) {
                    GameObject *explosion = new GameObject("smallExplosion");
                    explosion->spriteRenderer = new OneShotAnimationRenderer(explosion);
                    explosion->setFlags(GameObject::BulletPassable | GameObject::TankPassable);
                    explosion->copyParentPosition(obj);

                    objectsToAdd.push_back(explosion);

                    if (obj->getParentObject()->isFlagSet(GameObject::Player))
                        SoundPlayer::instance().playBulletHitWallSound();
                }

                if (obj->isFlagSet(GameObject::Player | GameObject::NPC | GameObject::Eagle)) {
                    GameObject *explosion = new GameObject("bigExplosion");
                    explosion->spriteRenderer = new OneShotAnimationRenderer(explosion);
                    explosion->setFlags(GameObject::BulletPassable | GameObject::TankPassable);
                    explosion->copyParentPosition(obj);

                    objectsToAdd.push_back(explosion);

                    SoundPlayer::instance().playSmallExplosionSound();
                }
                delete obj;
            } else {
                if (!obj->isFlagSet(GameObject::Static))
                    obj->update();
                ++it;
            }
        }

        for (auto obj : objectsToAdd) {
            ObjectsPool::addObject(obj);
        }

        // as the game just cleared all objects marked for deletion, all the existing enemies must be alive at this point
        int countEnemiesAlive = ObjectsPool::countObjectsByTypes({"npcGreenArmoredTank", "spawner_ArmoredTank"});
        if (countEnemiesAlive < 1 && framesToWin == -1) {
            framesToWin = MaxFramesToWin;
            SoundPlayer::instance().stopAllSounds();
            SoundPlayer::instance().gameOver = true;
        }

        window.clear();
        // draw objects (order matter)
        // draw border
        sf::RectangleShape greyRect(sf::Vector2f(screen_w, screen_h));
        greyRect.setFillColor(sf::Color(102, 102, 102));
        window.draw(greyRect);

        // draw view port
        sf::RectangleShape blackRect(sf::Vector2f(globalVars::gameViewPort.width, globalVars::gameViewPort.height));
        blackRect.setPosition(sf::Vector2f(globalVars::gameViewPort.left, globalVars::gameViewPort.top));
        blackRect.setFillColor(sf::Color(50, 0, 0));
        window.draw(blackRect);

        // 1. draw ice and water
        auto objectsToDrawFirst = ObjectsPool::getObjectsByTypes({"ice", "water"});
        std::for_each(objectsToDrawFirst.cbegin(), objectsToDrawFirst.cend(), [](GameObject *obj) { obj->draw(); });

        // 2. draw tanks and bullets
        std::unordered_set<GameObject *> objectsToDrawSecond = ObjectsPool::getObjectsByTypes({"player", "eagle", "npcGreenArmoredTank", "bullet"});
        std::for_each(objectsToDrawSecond.begin(), objectsToDrawSecond.end(), [](GameObject *obj) { if (obj) obj->draw(); });

        // 3. draw walls and trees
        auto objectsToDrawThird = ObjectsPool::getObjectsByTypes({"brickWall", "concreteWall", "tree", "smallExplosion", "bigExplosion"});
        std::for_each(objectsToDrawThird.cbegin(), objectsToDrawThird.cend(), [](GameObject *obj) { obj->draw(); });

        // 4. visual effects
        auto objectsToDrawFourth = ObjectsPool::getObjectsByTypes({"spawner_ArmoredTank"});
        std::for_each(objectsToDrawFourth.cbegin(), objectsToDrawFourth.cend(), [](GameObject *obj) { obj->draw(); });


        window.display();

        if (framesToDie > -1) {
            if (--framesToDie <= 0)
                gameState = GameOver;
        }

        if (framesToWin > -1) {
            if (--framesToWin <= 0) {
                gameState =  (++currentLevel < levelMaps.size()) ? LoadNextLevel : GameOver;
            }
        }
    }

    Logger::instance() << "Game window is closed";

    return 0;
}



void showTitleScreen(sf::RenderWindow& window)
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

    window.clear();

    sf::RectangleShape blackRect(sf::Vector2f(globalConst::screen_w, globalConst::screen_h));
    blackRect.setFillColor(sf::Color(0, 0, 0));

    window.draw(blackRect);
    window.draw(text);
    window.draw(textSub);
    window.draw(textInstruction);
    window.display();
}