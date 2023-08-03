#pragma once

#include <unordered_map>
#include <cassert>

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

#include "AnimationSpriteSheet.h"

class SpriteRenderer
{
    friend class GameObject;
    std::string _objectType = "";
    std::string _currentAnimation = "";

    std::vector<AnimationSpriteSheet::AnimationFrame> _currentAnimationFrames;
    int _currentFrame = 0;

    sf::Sprite _sprite;

    sf::Clock _clock;

    bool _animate = false;
public:
    SpriteRenderer(std::string t);
    virtual ~SpriteRenderer() {}
    void setCurrentAnimation(std::string id);
    void draw();
    void playAnimation(bool);

protected:
    void showAnimationFrame(int i);
};
