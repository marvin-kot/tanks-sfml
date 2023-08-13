#include "SpriteRenderer.h"
#include "GameObject.h"
#include "GlobalConst.h"
#include "AssetManager.h"
#include "Utils.h"
#include "Logger.h"

#include <cassert>
#include <SFML/System/Time.hpp>
#include <SFML/Graphics/Rect.hpp>

using namespace Assets;

SpriteRenderer::SpriteRenderer(GameObject * parent, std::string type)
: _parentObject(parent)
, _objectType(!type.empty() ? type : parent->type())
{
    assert(_parentObject != nullptr);
    assert(_objectType.empty() == false);

    _sprite.setTexture(AssetManager::instance().mainSpriteSheetTexture());
    setCurrentAnimation("default");
}

void SpriteRenderer::setCurrentAnimation(std::string id)
{
    assert(id.empty() == false);

    if (id == _currentAnimation)
        return;

    _currentAnimation = id;
    _currentAnimationFrames = AssetManager::instance().getAnimationFrames(_objectType, _currentAnimation);

    showAnimationFrame(0);
    playAnimation(true);
    _clock.restart();
}

void SpriteRenderer::hide(bool val)
{
    Logger::instance() << "SpriteRenderer::hide()";
    _hide = val;
}

bool SpriteRenderer::isHidden()
{
    return _hide;
}

void SpriteRenderer::showAnimationFrame(int frameNum)
{
    assert(_currentAnimation.empty() == false);

    _currentFrame = frameNum;

    sf::IntRect rect = _currentAnimationFrames[_currentFrame].rect;

    rect.left += _spriteSheetOffsetX;
    rect.top += _spriteSheetOffsetY;
    Logger::instance() << rect.width << "," << rect.height;
    _parentObject->setSize(rect.width, rect.height);
    _sprite.setTextureRect(rect);
    _sprite.setScale(globalConst::spriteScaleX, globalConst::spriteScaleY);
    _sprite.setOrigin(rect.width/2, rect.height/2);
}

void SpriteRenderer::draw(bool paused)
{
    if (isHidden()) return;

    if (!paused) {
        int framesCount = _currentAnimationFrames.size();
        // play set next frame if duration of current frame passed
        if (framesCount > 1 && _animate) {
            if (_clock.getElapsedTime() > sf::milliseconds(_currentAnimationFrames[_currentFrame].duration)) {
                int nextFrame = _currentFrame+1 < framesCount ? _currentFrame+1 : 0;
                showAnimationFrame(nextFrame);
                _clock.restart();
            }
        }
    }
    Utils::window.draw(_sprite);
}

void SpriteRenderer::playAnimation(bool play)
{
    _animate = play;
}

void SpriteRenderer::setSpriteSheetOffset(int x, int y)
{
    _spriteSheetOffsetX = x;
    _spriteSheetOffsetY = y;
}

OneShotAnimationRenderer::OneShotAnimationRenderer(GameObject * parent, std::string type) : SpriteRenderer(parent, type) {}

void OneShotAnimationRenderer::draw(bool paused)
{
    if (isHidden()) return;
    if (!paused) {
        int framesCount = _currentAnimationFrames.size();
        // play set next frame if duration of current frame passed
        if (framesCount > 1 && _animate) {
            if (_clock.getElapsedTime() > sf::milliseconds(_currentAnimationFrames[_currentFrame].duration)) {
                int nextFrame = _currentFrame+1;

                if (nextFrame == framesCount) {
                    _parentObject->_deleteme = true;
                    return;
                }
                showAnimationFrame(nextFrame);
                _clock.restart();
            }
        }
    }
    Utils::window.draw(_sprite);
}

LoopAnimationSpriteRenderer::LoopAnimationSpriteRenderer(GameObject * parent, std::string type) : SpriteRenderer(parent, type) {}

void LoopAnimationSpriteRenderer::draw(bool paused)
{
    if (isHidden()) return;

    if (!paused) {
        //Logger::instance() << "LoopAnimationSpriteRenderer::draw()";
        int framesCount = _currentAnimationFrames.size();
        // play set next frame if duration of current frame passed
        if (framesCount > 1 && _animate) {
            if (_clock.getElapsedTime() > sf::milliseconds(_currentAnimationFrames[_currentFrame].duration)) {
                int nextFrame = _currentFrame+1;

                if (nextFrame >= framesCount) {
                    nextFrame = 0;
                }
                showAnimationFrame(nextFrame);
                _clock.restart();
            }
        }
    }

    Utils::window.draw(_sprite);
}
