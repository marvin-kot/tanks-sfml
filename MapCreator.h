#pragma once

#include <fstream>
#include <nlohmann/json.hpp>

#include <map>


using json = nlohmann::json;


class GameObject;

class MapCreator
{
public:
    virtual int parseMapFile(std::string fileName) = 0;
    virtual int buildMapFromData() = 0;

protected:
    static GameObject *buildObject(std::string type, int x, int y);

};

class MapCreatorFromCustomMatrixFile : public MapCreator
{
    int map_w, map_h;
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