#pragma once

#include <unordered_map>
#include <cassert>

#include <SFML/Graphics.hpp>

enum Direction
{
    Up,
    Left,
    Down,
    Right
};

class SpriteRenderer
{
    friend class GameObject;
    sf::Sprite _sprite;
    std::string _objectType;
    std::string _currentAnimation;
    int _currentFrame;

public:
    SpriteRenderer(std::string t);
    virtual ~SpriteRenderer() {}
    void setCurrentAnimation(std::string id);

protected:
    void setDefaultSprite();
    void showAnimationFrame(int i);
};
