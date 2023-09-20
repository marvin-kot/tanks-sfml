#include "AssetManager.h"

#include "Logger.h"

#include <filesystem>

using namespace Assets;
const std::string assetFolderPath = "assets";

AssetManager& AssetManager::instance()
{
    static AssetManager _instance;
    return _instance;
}



bool AssetManager::loadSpriteSheet(std::string textureName)
{
    std::string pngFileName = assetFolderPath + "/" + textureName + ".png";
    if (!std::filesystem::exists(pngFileName)) {
        Logger::instance() << pngFileName << " does not exist\n";
        return false;
    }

    std::string jsonFileName = assetFolderPath + "/" + textureName + ".json";
    if (!std::filesystem::exists(jsonFileName)) {
        Logger::instance() << jsonFileName << " does not exist\n";
        return false;
    }

    if (!_spriteSheetTexture.loadFromFile(pngFileName)) {
        Logger::instance() << pngFileName << " failed to load texture\n";
        return false;
    }

    if (!parseJsonFileToJsonStruct(jsonFileName)) {
        Logger::instance() << "[ERROR] parsing sprite sheet json file";
        return false;
    }

    if (!parseJsonToDataStructure()) {
        Logger::instance() << "[ERROR] parsing sprite sheet json structure";
        return false;
    }

    return true;
}


// deprecated
bool AssetManager::loadTexture(std::string textureName)
{
    Logger::instance() << "load texture";
    assert(texLoaded == false);
    bool result = _spriteSheetTexture.loadFromFile(textureName);
    if (result)
        texLoaded = true;

    return result;
}

bool AssetManager::parseJsonFileToJsonStruct(std::string jsonName)
{
    Logger::instance() << "parse json ";

    std::ifstream f(jsonName);

    Logger::instance() << jsonName << "\n";

    try {
        data = json::parse(f);
    }
    catch (json::parse_error& ex)
    {
        std::cerr << "parse error at byte " << ex.byte << std::endl;
        return false;
    }

    jsonLoaded = true;

    return true;
}

bool AssetManager::parseJsonToDataStructure()
{
    assert(jsonLoaded == true);

    try {
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

                    if (jframe.contains("dur"))
                        frame.duration = jframe["dur"];
                    else
                        frame.duration = 0;

                    animation.frames.push_back(frame);
                    framesById[frame.id] = frame;
                }

                animationMap[animationName] = animation;
            }
            objectTypeAnimations[objectType] = animationMap;
        }
    }
    catch (json::parse_error& ex) {
        std::cerr << "parse error at byte " << ex.byte << std::endl;
        return false;
    }

    jsonParsed = true;
    return true;
}

std::vector<AnimationFrame> AssetManager::getAnimationFrames(std::string type, std::string animation)
{
    assert(jsonLoaded == true);
    assert(jsonParsed == true);
    assert(objectTypeAnimations.contains(type) == true);
    assert(objectTypeAnimations.at(type).contains(animation) == true);
    return objectTypeAnimations.at(type).at(animation).frames;
}

AnimationFrame AssetManager::getAnimationFrame(std::string type, std::string animation, int frameNum)
{
    assert(objectTypeAnimations.empty() == false);
    assert(objectTypeAnimations.contains(type) == true);
    assert(objectTypeAnimations.at(type).contains(animation) == true);
    assert(objectTypeAnimations.at(type).at(animation).frames.empty() == false);
    assert(objectTypeAnimations.at(type).at(animation).frames.size() >= frameNum);

    return objectTypeAnimations.at(type).at(animation).frames[frameNum];
}

bool AssetManager::loadFont(std::string file)
{
    return _titleFont.loadFromFile(file);
}
