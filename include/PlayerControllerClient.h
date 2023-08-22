#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <map>

namespace net
{
    struct PlayerInput;
}

class PlayerControllerClient
{
    sf::Keyboard::Key _mostRecentKey;
    sf::Clock _clock;
    bool _pause = false;
    sf::Time _lastActionTime;

    enum KeysPressed {
        NothingPressed  = 0x0,
        SpacePressed    = 0x1,
        LeftPressed     = 0x2,
        UpPressed       = 0x4,
        RightPressed    = 0x8,
        DownPressed     = 0x10
    };

    int _pressedStates = 0;

    void setPressedFlag(KeysPressed flag, bool state);
    bool wasPressed(KeysPressed flag);

    std::map<sf::Keyboard::Key, sf::Time> _pressedKeys;

private:
    PlayerControllerClient();
    PlayerControllerClient(PlayerControllerClient &) = delete;
    void operator=(const PlayerControllerClient&) = delete;
    ~PlayerControllerClient();

public:
    static PlayerControllerClient& instance();

    void update(net::PlayerInput *);

};


