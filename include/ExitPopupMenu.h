#include <SFML/Graphics.hpp>
#include "GlobalTypes.h"

#pragma once

class ExitPopupMenu
{
    ExitPopupMenu();
    ExitPopupMenu(ExitPopupMenu &) = delete;
    void operator=(const ExitPopupMenu&) = delete;

    int _currentCursor = 0;
    bool _isOpen;

    sf::Sprite _sprite;
    sf::Texture _texture;

    void drawCursorOnButton(int x,  int y);
public:
    static ExitPopupMenu& instance();

    void open();
    void close();

    void draw();
    void moveCursorLeft();
    void moveCursorRight();
    globalTypes::GameState applyChoice();
    bool isOpen() const { return _isOpen; }
    int cursorPosition() const { return _currentCursor; }
};