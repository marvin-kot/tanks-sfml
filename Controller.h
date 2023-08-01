#include <SFML/Window/Keyboard.hpp>

class AnimatedGameObject;

class Controller
{
protected:
    AnimatedGameObject *_gameObject;
public:
    Controller(AnimatedGameObject *obj);
    virtual void update() {}
};

class PlayerController : public Controller
{
    const int moveSpeed = 5;
public:
    PlayerController(AnimatedGameObject *obj);
    void update() override;
};


