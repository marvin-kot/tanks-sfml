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
protected:
    sf::Texture& _texture;
    sf::Sprite _sprite;

    int _sizeX, _sizeY;
    int _offsetX, _offsetY;

public:
    SpriteRenderer(sf::Texture& tex);
    virtual ~SpriteRenderer() {}
    void setMainSprite(int ofx, int ofy, int sx, int sy);
    virtual void assignAnimationFrame(std::string id, int ofx, int ofy, int sx, int sy);
    virtual void showAnimationFrame(std::string id);
};

class AnimatedSpriteRenderer : public SpriteRenderer
{
protected:
    std::unordered_map<std::string, sf::IntRect> _animationFrames;

public:
    AnimatedSpriteRenderer(sf::Texture& tex);
    void assignAnimationFrame(std::string id, int ofx, int ofy, int sx, int sy) override;
    void showAnimationFrame(std::string id) override;
};