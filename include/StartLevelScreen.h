#include "GlobalTypes.h"
#include "MapCreator.h"

#include <SFML/Graphics.hpp>

#pragma once

class StartLevelScreen
{
    StartLevelScreen();
    StartLevelScreen(StartLevelScreen &) = delete;
    void operator=(const StartLevelScreen&) = delete;

    int _currentUpgradeCursor = 0;
    bool _isOpen;

    //void drawCursor(int x, int y);
    //void drawCursorOnButton(int x, int y);
    //void drawEdging(int x, int y);

    //sf::Sprite _sprite;
    //sf::Texture _texture;

    //int buttonStartCursorPos;
    //int buttonExitCursorPos;
    Level::Properties _currentLevelProperties;

    sf::Sprite _sprite;
    sf::Texture _texture;
public:

    static StartLevelScreen& instance();

    void open();
    void close();

    void draw();
    //void moveCursorLeft();
    //void moveCursorRight();

    bool isOpen() const { return _isOpen; }
    int cursorPosition() const { return _currentUpgradeCursor; }

    void setLevelProperties(const Level::Properties& prop);

    void processKeyboardPress(sf::Keyboard::Scancode scancode, globalTypes::GameState& gameState);

};