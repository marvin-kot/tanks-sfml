#include "SpriteRenderer.h"
#include "GlobalConst.h"
#include "AnimationSpriteSheet.h"

#include <cassert>

SpriteRenderer::SpriteRenderer(std::string t) : _objectType(t)
{
    _sprite.setTexture(AnimationSpriteSheet::instance().spriteSheetTexture);
    setDefaultSprite();
}

void SpriteRenderer::setDefaultSprite()
{
    auto frame = AnimationSpriteSheet::instance().getAnimationFrame(_objectType, "default", 0);
    _sprite.setTextureRect(frame.rect);
    _sprite.setScale(globalConst::spriteScaleX, globalConst::spriteScaleY);
    _sprite.setOrigin(frame.rect.width/2, frame.rect.height/2);
}


void SpriteRenderer::setCurrentAnimation(std::string id)
{
    assert(id.empty() == false);
    
    _currentAnimation = id;
    showAnimationFrame(0);
}

void SpriteRenderer::showAnimationFrame(int i)
{
    assert(_currentAnimation.empty() == false);

    auto frame = AnimationSpriteSheet::instance().getAnimationFrame(_objectType, _currentAnimation, i);

    _sprite.setTextureRect(frame.rect);
    _sprite.setScale(globalConst::spriteScaleX, globalConst::spriteScaleY);
    _sprite.setOrigin(frame.rect.width/2, frame.rect.height/2);
}
