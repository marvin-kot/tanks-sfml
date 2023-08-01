#pragma once

#include <fstream>
#include <nlohmann/json.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <cassert>

#include "Logger.h"

using json = nlohmann::json;


class AnimationSpriteSheet
{
    json data;

    bool texLoaded = false;
    bool jsonLoaded = false;
    bool jsonParsed = false;

public:

    struct AnimationFrame {
        int id;
        sf::IntRect rect;
    };

    struct Animation {
        std::vector<AnimationFrame> frames;
    };

    using AnimationMap = std::unordered_map<std::string, Animation>;

    std::unordered_map<std::string, AnimationMap> objectTypeAnimations;
    
public:
    sf::Texture spriteSheetTexture;
    
public:
    static AnimationSpriteSheet& instance()
    {
        static AnimationSpriteSheet _instance;
        return _instance;
    }

    bool loadTexture(std::string textureName)
    {
        Logger::instance() << "load texture";
        assert(texLoaded == false);
        bool result = spriteSheetTexture.loadFromFile(textureName, sf::IntRect(0, 0, 384, 256));
        if (result)
            texLoaded = true;
        
        return result;
    }

    void parseJsonFileToJsonAtruct(std::string jsonName)
    {
        Logger::instance() << "parse json ";

        std::ifstream f(jsonName);

        Logger::instance() << jsonName;
        data = json::parse(f);

        Logger::instance() << "file parsed to json structure";
        jsonLoaded = true;
    }
    
    void parseJsonToDataStructure()
    {
        assert(jsonLoaded == true);

        Logger::instance() << "parse json structure to c++ data structures";
        for (json::iterator it = data.begin(); it != data.end(); ++it) {
            json mmap = *it;
            std::string objectType = mmap["type"];

            AnimationMap animationMap;

            json animations = mmap["animations"];
            for (json::iterator ita = animations.begin(); ita != animations.end(); ++ita) {
                json j = *ita;
                Animation animation;
                std::string animationName = j["name"];
                json frames = j["frames"];
                for (json::iterator itf = frames.begin(); itf != frames.end(); ++itf) {
                    json jframe = *itf;
                    AnimationFrame frame;
                    frame.id = jframe["id"];
                    int x = jframe["x"], y = jframe["y"];
                    int w = jframe["w"], h = jframe["h"];
                    frame.rect = sf::IntRect(x, y, w, h);

                    animation.frames.push_back(frame);
                }

                animationMap[animationName] = animation;
            }
            objectTypeAnimations[objectType] = animationMap;
        }

        jsonParsed = true;
        Logger::instance() << "json structure parsed";
    }

    AnimationMap getObjectAnimationMap(std::string type) const
    {
        assert(jsonLoaded == true);
        assert(jsonParsed == true);
        return objectTypeAnimations.at(type);
    }

    std::vector<AnimationFrame> getAnimationFrames(std::string type, std::string animation)
    {
        assert(jsonLoaded == true);
        assert(jsonParsed == true);
        return objectTypeAnimations.at(type).at(animation).frames;
    }

    AnimationFrame getAnimationFrame(std::string type, std::string animation, int frameNum)
    {
        assert(objectTypeAnimations.empty() == false);
        assert(objectTypeAnimations.find(type) != objectTypeAnimations.end());
        assert(objectTypeAnimations.at(type).find(animation) != objectTypeAnimations.at(type).end());
        assert(objectTypeAnimations.at(type).at(animation).frames.empty() == false);
        assert(objectTypeAnimations.at(type).at(animation).frames.size() >= frameNum);

        return objectTypeAnimations.at(type).at(animation).frames[frameNum];
    }

    AnimationSpriteSheet(AnimationSpriteSheet& ) = delete;
    void operator=(const AnimationSpriteSheet&) = delete;
private:
    AnimationSpriteSheet() {}
    ~AnimationSpriteSheet() {}
};