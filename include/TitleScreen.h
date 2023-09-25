#pragma once

#include "GlobalTypes.h"

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

class TitleScreen
{
    enum States {
        Start,
        ShowIntro1,
        ShowIntro2,
        ShowTitle
    };

    States _state = Start;
    TitleScreen();
    TitleScreen(TitleScreen &) = delete;
    void operator=(const TitleScreen&) = delete;

    int _cursorPos;
    int _blinkCount;
    bool _selected;
    void moveCursorUp();
    void moveCursorDown();

    sf::Clock _clock;
    sf::Clock _delayClock;
    bool _blink;

    void drawIntro1(int screenCenterX, int screenCenterY);
    void drawIntro2(int screenCenterX, int screenCenterY);
    void drawTitleMenu(int screenCenterX, int screenCenterY);
public:
    static TitleScreen& instance();

    globalTypes::GameState draw();
    void processKeyboardPress(sf::Keyboard::Scancode);

    void selectOption();
};