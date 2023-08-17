#include <SFML/Graphics.hpp>

#pragma once

class LevelUpPopupMenu
{
    LevelUpPopupMenu();
    LevelUpPopupMenu(LevelUpPopupMenu &) = delete;
    void operator=(const LevelUpPopupMenu&) = delete;

    int _currentUpgradeCursor = 0;
    bool _isOpen;

    void drawUpgrade(int index, int x, int y);
    void drawCursor(sf::RectangleShape& parentRect);
public:
    static LevelUpPopupMenu& instance();

    void open();
    void close();

    void draw();
    void moveCursorLeft();
    void moveCursorRight();
    void getSelectedUpgrade();
    bool isOpen() const { return _isOpen; }
};