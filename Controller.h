#include <SFML/Window/Keyboard.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Clock.hpp>

#include <random>


class AnimatedGameObject;

class Controller
{

protected:
    AnimatedGameObject *_gameObject;
    sf::Clock clock;
    sf::Time lastActionTime;
    bool isMoving = false;
public:
    Controller(AnimatedGameObject *obj);
    virtual void update() {}
};

class StupidController : public Controller
{
    const int moveSpeed = 3;
    const sf::Time actionTimeout = sf::seconds(0.5);

    int currMoveX = 0;
    int currMoveY = 0;

    std::uniform_int_distribution<int> distribution;
public:
    StupidController(AnimatedGameObject *obj);
    void update() override;
};

class PlayerController : public Controller
{
    const int moveSpeed = 5;
public:
    PlayerController(AnimatedGameObject *obj);
    void update() override;
};


