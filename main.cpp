#include <iostream>
#include <fstream>
#include <SFML/Graphics.hpp>

#include "GlobalConst.h"
#include "GameObject.h"
#include "Logger.h"
#include "Controller.h"




int main()
{
    using namespace globalConst;

    Logger::instance() << "Starting the Game...";
 
    float posx = screen_w/2, posy = screen_h/2;

    sf::RenderWindow window(sf::VideoMode(screen_w, screen_h), "SFML works!");
    window.setVerticalSyncEnabled(true);

    sf::Texture texture;
    if (!texture.loadFromFile("assets/spriteSheet16x16.png", sf::IntRect(0, 0, 256, 256)))
    {
        Logger::instance() << "[ERROR] Could not open assets/spriteSheet16x16.png";
        return -1;
    }

    AnimatedGameObject pc("player", window);
    {
        pc.setController(new PlayerController(&pc));
        //pc.createCollider(sf::IntRect(-8, -8, 16, 16));
        pc.createSpriteRenderer(texture);

        pc.assignAnimationFrame("up", 0, 0, 16, 16);
        pc.assignAnimationFrame("left", 32, 0, 16, 16);
        pc.assignAnimationFrame("down", 64, 0, 16, 16);
        pc.assignAnimationFrame("right", 96, 0, 16, 16);

        pc.setCurrentFrame("up");

        pc.setPos(posx, posy);

        pc.update(); pc.draw();
    }


    AnimatedGameObject enemy("enemy", window);
    {
        enemy.createSpriteRenderer(texture);
        enemy.assignAnimationFrame("up", 0, 240, 16, 16);
        enemy.assignAnimationFrame("left", 32, 240, 16, 16);
        enemy.assignAnimationFrame("down", 64, 240, 16, 16);
        enemy.assignAnimationFrame("right", 96, 240, 16, 16);
        enemy.setController(new StupidController(&enemy));

        pc.setCurrentFrame("down");
        enemy.setPos(128, 128);

        enemy.update(); enemy.draw();
    }
        

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
        pc.update(); pc.draw();
        enemy.update(); enemy.draw();
        window.display();
    }

    Logger::instance() << "Game window is closed";

    return 0;
}