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
#include "AnimationSpriteSheet.h"
#include "Shootable.h"

GameObject *ObjectsPool::playerObject = nullptr;
std::unordered_set<GameObject *> ObjectsPool::obstacles = {};
std::unordered_set<GameObject *> ObjectsPool::enemies = {};
std::unordered_set<GameObject *> ObjectsPool::bullets = {};

std::default_random_engine Utils::generator = {};
sf::RenderWindow Utils::window = {};

bool loadAssets()
{
    // load textures
    if (!AnimationSpriteSheet::instance().loadTexture("assets/spriteSheet16x16.png"))
    {
        Logger::instance() << "[ERROR] Could not open assets/spriteSheet16x16.png";
        return false;
    }

    AnimationSpriteSheet::instance().parseJsonFileToJsonAtruct("assets/spriteSheet16x16.json");
    AnimationSpriteSheet::instance().parseJsonToDataStructure();

    // TODO: load sounds

    return true;
}


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
    // load main texture sprite sheet

    float posx = screen_w/2, posy = screen_h/2;

    Utils::window.create(sf::VideoMode(screen_w, screen_h), "Retro Tank Massacre SFML");
    sf::RenderWindow& window = Utils::window;
    window.setVerticalSyncEnabled(true);

    Logger::instance() << "Creating player...";
    GameObject pc("player");
    {
        pc.setController(new PlayerController(&pc));
        pc.setShootable(new Shootable(&pc));
        pc.setFlags(GameObject::Player | GameObject::BulletKillable);
        pc.createSpriteRenderer();
        pc.setPos(posx, posy);
        pc.setCurrentDirection(globalTypes::Up);

        ObjectsPool::playerObject = &pc;
    }


    Logger::instance() << "Creating enemies...";
    for (int i = 0; i < 4; i++)
    {
        GameObject *enemy = new GameObject("npcGreenArmoredTank");
        enemy->setShootable(new Shootable(enemy));
        enemy->setFlags(GameObject::NPC | GameObject::BulletKillable);
        enemy->createSpriteRenderer();
        enemy->setController(new StupidController(enemy));

        enemy->setPos(32 + i*288, 32);

        ObjectsPool::enemies.insert(enemy);
    }

    // build map from json
    Logger::instance() << "Building map...";
    /*MapCreatorFromJson mapBuilder;
    mapBuilder.parseMapFile("assets/testmap.json");
    mapBuilder.buildMapFromData();*/
    MapCreatorFromCustomMatrixFile mapBuilder;
    mapBuilder.parseMapFile("assets/testmap.txt");
    mapBuilder.buildMapFromData();


    Logger::instance() << "Starting the Game...";
    while (window.isOpen())
    {
        sf::Event event;
        float rotation;
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
        // border
        sf::RectangleShape greyRect(sf::Vector2f(screen_w, screen_h));
        greyRect.setFillColor(sf::Color(102, 102, 102));
        window.draw(greyRect);

        // draw view port
        sf::RectangleShape blackRect(sf::Vector2f(gameViewPort.width, gameViewPort.height));
        blackRect.setPosition(sf::Vector2f(gameViewPort.left, gameViewPort.top));
        blackRect.setFillColor(sf::Color(50, 0, 0));
        window.draw(blackRect);
        // update pc
        pc.update(); 
        if (pc.mustBeDeleted()) {
            Utils::gameOver();
            break;
        }
        pc.draw();

        // update flying bullets
        for (auto it = ObjectsPool::bullets.begin(); it != ObjectsPool::bullets.end(); ) {
            GameObject *obj = *it;
            if (obj->mustBeDeleted()) {
                it = ObjectsPool::bullets.erase(it);
                delete obj;
            } else {
                obj->update(); obj->draw();
                ++it;
            }
        }

        // update enemies
        for (auto it = ObjectsPool::enemies.begin(); it != ObjectsPool::enemies.end(); ) {
            GameObject *obj = *it;
            if (obj->mustBeDeleted()) {
                it = ObjectsPool::enemies.erase(it);
                delete obj;
            } else {
                obj->update(); obj->draw();
                ++it;
            }
        }

        // update obstacles (delete if needed)
        for (auto it = ObjectsPool::obstacles.begin(); it != ObjectsPool::obstacles.end(); ) {
            GameObject *obj = *it;
            if (obj->mustBeDeleted()) {
                if (obj->isFlagSet(GameObject::Eagle))
                    Utils::gameOver();
                it = ObjectsPool::obstacles.erase(it);
                delete obj;
            } else {
                obj->update(); obj->draw();
                ++it;
            }
        }

        window.display();
    }

    Logger::instance() << "Game window is closed";

    return 0;
}
