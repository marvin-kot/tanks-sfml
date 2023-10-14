#include <SFML/Graphics.hpp>
#include "GlobalTypes.h"

#pragma once

class BonusShopWindow
{
    BonusShopWindow();
    BonusShopWindow(BonusShopWindow &) = delete;
    void operator=(const BonusShopWindow&) = delete;

    int _currentUpgradeCursor = 0;
    bool _isOpen;

    void drawUpgrade(int index, int x, int y);
    //void drawCursor(int startY, sf::RectangleShape& parentRect);
    void drawCursor(int x, int y);
    void drawCursorOnButton(int x, int y);
    void drawEdging(int x, int y);

    sf::Sprite _sprite;
    sf::Texture _texture;

    int buttonStartCursorPos;
    int buttonExitCursorPos;
public:
    bool afterGameOver = false;
    bool afterWin = false;

    static BonusShopWindow& instance();

    void open();
    void close();

    void draw();
    void moveCursorLeft();
    void moveCursorRight();
    void moveCursorUp();
    void moveCursorDown();
    void getSelectedUpgrade();
    bool isOpen() const { return _isOpen; }
    int cursorPosition() const { return _currentUpgradeCursor; }

    void processKeyboardPress(sf::Keyboard::Scancode scancode, globalTypes::GameState& gameState);

};