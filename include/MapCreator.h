#pragma once

#include <fstream>
#include <nlohmann/json.hpp>

#include <map>


using json = nlohmann::json;


class GameObject;

class MapCreator
{
protected:
    int map_w, map_h;
public:
    virtual int parseMapFile(std::string fileName) = 0;
    virtual int buildMapFromData() = 0;
    int mapWidth() const { return map_w; }
    int mapHeight() const { return map_h; }

protected:
    static GameObject *buildObject(std::string type);
    void setupScreenBordersBasedOnMapSize();

};

class MapCreatorFromCustomMatrixFile : public MapCreator
{

    std::string mapString;
    std::map<char, std::string> charMap;
public:
    MapCreatorFromCustomMatrixFile();

    int parseMapFile(std::string fileName) override;
    int buildMapFromData() override;
};



class MapCreatorFromJson : public MapCreator
{
    json data;

public:
    MapCreatorFromJson() {}

    int parseMapFile(std::string jsonName) override;
    int buildMapFromData() override;

};