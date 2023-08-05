#pragma once

#include <unordered_map>
#include <cassert>

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

#include "AssetManager.h"

class GameObject;

class SpriteRenderer
{
    friend class GameObject;
protected:
    GameObject *_parentObject;

    std::string _objectType = "";
    std::string _currentAnimation = "";

    std::vector<Assets::AnimationFrame> _currentAnimationFrames;
    int _currentFrame = 0;

    sf::Sprite _sprite;

    sf::Clock _clock;

    bool _animate = false;
public:
    SpriteRenderer(GameObject * parent);
    virtual ~SpriteRenderer() {}
    void setCurrentAnimation(std::string id);
    virtual void draw();
    void playAnimation(bool);

protected:
    void showAnimationFrame(int i);
};


class OneShotAnimationRenderer : public SpriteRenderer
{
public:
    OneShotAnimationRenderer(GameObject * parent);
    void draw() override;
};
