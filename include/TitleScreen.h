#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

class TitleScreen
{
    TitleScreen();
    TitleScreen(TitleScreen &) = delete;
    void operator=(const TitleScreen&) = delete;

    int _cursorPos;
    int _blinkCount;
    bool _selected;
    void moveCursorUp();
    void moveCursorDown();

    sf::Clock _clock;
    bool _blink;
public:
    static TitleScreen& instance();

    int draw();
    void processKeyboardPress(sf::Keyboard::Scancode);

    void selectOption();
};