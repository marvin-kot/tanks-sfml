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

bool loadAssets()
{
    // load textures
    if (!AssetManager::instance().loadSpriteSheet("spriteSheet16x16"))
    {
        Logger::instance() << "[ERROR] Could not open assets/spriteSheet16x16.png";
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


int main(int argc, char *argv[])
{
    using namespace globalConst;

    if (argc < 2)
        return 0;

    std::string levelMap(argv[1]);

    // initialize logger
    Logger::instance() << "Loading assets...";
    if (!loadAssets())
    {
        return -1;
    }

    Utils::window.create(sf::VideoMode(screen_w, screen_h), "Retro Tank Massacre SFML [MAP VISUALIZER]");
    sf::RenderWindow& window = Utils::window;
    window.setVerticalSyncEnabled(true);

    // main loop
    if (!buildLevelMap(levelMap)) {
        window.close();
        Logger::instance() << "Failed to build map";
        return -1;
    }

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::KeyPressed:
                    if (event.key.scancode == sf::Keyboard::Scan::Escape)
                        window.close();
                    break;
                case sf::Event::Resized:
                    std::cout << "new width: " << event.size.width << std::endl;
                    std::cout << "new height: " << event.size.height << std::endl;
                    break;
                default:
                    break;
            }
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
        std::unordered_set<GameObject *> objectsToDrawSecond = ObjectsPool::getObjectsByTypes({"player", "eagle", "npcArmorTank", "bullet"});
        std::for_each(objectsToDrawSecond.begin(), objectsToDrawSecond.end(), [](GameObject *obj) { if (obj) obj->draw(); });

        // 3. draw walls and trees
        auto objectsToDrawThird = ObjectsPool::getObjectsByTypes({"brickWall", "concreteWall", "tree", "smallExplosion", "bigExplosion"});
        std::for_each(objectsToDrawThird.cbegin(), objectsToDrawThird.cend(), [](GameObject *obj) { obj->draw(); });

        window.display();
    }

    Logger::instance() << "Game window is closed";

    return 0;
}
