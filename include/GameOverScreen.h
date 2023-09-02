#pragma once

#include <SFML/System/Clock.hpp>

class GameOverScreen
{
private:
    enum States
    {
        Start,
        DelayAfterStart,
        CountSeconds,
        CountKills,
        DelayAfterCount,
        AddingToDeposit,
        DelayAfterAdding,
        UnlockShop,
        UnlockNewMission,
        Exit
    };

    States _state;
    sf::Clock _clock;
    sf::Clock _delayClock;

    int _enemiesKilled;
    int _secondsSurvived;


    int _countSec;
    int _countKills;
    int _countMoney;

    bool _shopJustUnlocked = false;
    bool _newMissionJustUnlocked = false;

    bool _faster = false;

    bool _won = false;

    GameOverScreen(GameOverScreen&) = delete;
    void operator=(const GameOverScreen&) = delete;

    GameOverScreen();
public:
    static GameOverScreen& instance();

    void setMissionOutcome(bool won, int kills, int time);
    int draw();
    void increaseCountSpeed() { _faster = true; }
};