#pragma once

#include <nlohmann/json.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Text.hpp>

#include <fstream>
#include <cassert>

using json = nlohmann::json;

namespace Assets
{
    struct AnimationFrame {
        int id;
        sf::IntRect rect;
        int duration;
    };

    struct Animation {
        std::vector<AnimationFrame> frames;
    };

    using AnimationMap = std::unordered_map<std::string, Animation>;
};

class AssetManager
{
    json data;

    bool texLoaded = false;
    bool jsonLoaded = false;
    bool jsonParsed = false;

    sf::Texture _spriteSheetTexture;
    sf::Font _titleFont;

public:

    std::unordered_map<std::string, Assets::AnimationMap> objectTypeAnimations;

public:

public:
    static AssetManager& instance();

    bool loadSpriteSheet(std::string textureName);

    std::vector<Assets::AnimationFrame> getAnimationFrames(std::string type, std::string animation);

    Assets::AnimationFrame getAnimationFrame(std::string type, std::string animation, int frameNum);

    bool loadFont(std::string file);

    sf::Font& defaultFont() { return _titleFont; }
    const sf::Texture& mainSpriteSheetTexture() const { return _spriteSheetTexture; }

    AssetManager(AssetManager& ) = delete;
    void operator=(const AssetManager&) = delete;
protected:
    bool loadTexture(std::string textureName);

    bool parseJsonFileToJsonStruct(std::string jsonName);
    bool parseJsonToDataStructure();
private:

    AssetManager() {}
    ~AssetManager() {}
};