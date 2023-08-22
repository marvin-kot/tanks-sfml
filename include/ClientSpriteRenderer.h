#pragma once

#include <unordered_map>


#include "AssetManager.h"
#include "NetGameTypes.h"

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

#include <cassert>

#include <vector>

class GameObject;

class ClientSpriteRenderer
{
public:

    static void drawAllObjects(const std::vector<net::ThinGameObject> objects)
    {
        sf::Sprite _sprite;

        _sprite.setTexture(AssetManager::instance().mainSpriteSheetTexture());

        for (auto it = objects.cbegin(); it != objects.cend(); ++it) {
            net::ThinGameObject go = (*it);
            sf::IntRect rect(go.spr_left, go.spr_top, go.spr_w, go.spr_h);
            _sprite.setTextureRect(rect);
            _sprite.setScale(globalConst::spriteScaleX, globalConst::spriteScaleY);
            _sprite.setOrigin(rect.width/2, rect.height/2);
            Utils::window.draw(_sprite);
        }
    }

    static void drawAllObjects(net::ThinGameObject objects[], int num_objects)
    {
        sf::Sprite _sprite;

        _sprite.setTexture(AssetManager::instance().mainSpriteSheetTexture());

        for (int i = 0; i < num_objects; i++) {
            net::ThinGameObject& go = objects[i];
            sf::IntRect rect(go.spr_left, go.spr_top, go.spr_w, go.spr_h);
            _sprite.setPosition(go.x, go.y);
            _sprite.setTextureRect(rect);
            _sprite.setScale(globalConst::spriteScaleX, globalConst::spriteScaleY);
            _sprite.setOrigin(rect.width/2, rect.height/2);
            Utils::window.draw(_sprite);
        }
    }

};

