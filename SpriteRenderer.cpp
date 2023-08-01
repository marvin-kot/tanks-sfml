#include "SpriteRenderer.h"
#include "GlobalConst.h"

#include <cassert>

SpriteRenderer::SpriteRenderer(sf::Texture& tex)
: _texture(tex)
{
    _sprite.setTexture(_texture);
}

void SpriteRenderer::setMainSprite(int ofx, int ofy, int sx, int sy)
{
    _sprite.setTextureRect(sf::IntRect(ofx, ofy, sx, sy));
    _sprite.setScale(globalConst::spriteScaleX, globalConst::spriteScaleY);
    _sprite.setOrigin(sx/2, sy/2);
}

void SpriteRenderer::assignAnimationFrame(std::string id, int ofx, int ofy, int sx, int sy)
{
    setMainSprite(ofx, ofy, sx, sy);
}

void SpriteRenderer::showAnimationFrame(std::string id)
{}

AnimatedSpriteRenderer::AnimatedSpriteRenderer(sf::Texture& tex)
: SpriteRenderer(tex)
{}

void AnimatedSpriteRenderer::assignAnimationFrame(std::string id, int ofx, int ofy, int sx, int sy)
{
    _animationFrames[id] = sf::IntRect(ofx, ofy, sx, sy);
}

void AnimatedSpriteRenderer::showAnimationFrame(std::string id)
{
    assert(_animationFrames.find(id) != _animationFrames.end());

    sf::IntRect rect = _animationFrames[id];
    _sprite.setTextureRect(rect);
    _sprite.setScale(globalConst::spriteScaleX, globalConst::spriteScaleY);
    _sprite.setOrigin(rect.width/2, rect.height/2);
}
