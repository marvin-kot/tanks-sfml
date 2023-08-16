#pragma once

#include <fstream>
#include <nlohmann/json.hpp>

#include <map>


using json = nlohmann::json;


class GameObject;
class SpawnController;

class MapCreator
{
protected:
    std::string _name;
    std::string _goal;
    int map_w, map_h;

    struct SpawnerData
    {
        std::string type;
        int row, col;
        int delay;
        int timeout;
        int quantity;
    };

    std::vector<SpawnerData> _spawners;
public:
    virtual int parseMapFile(std::string fileName) = 0;
    virtual int buildMapFromData() = 0;
    int placeSpawnerObjects();
    int mapWidth() const { return map_w; }
    int mapHeight() const { return map_h; }
    std::string mapName() { return _name; }
    std::string mapGoal() { return _goal; }

protected:
    GameObject *buildObject(std::string type);
    GameObject *createSpawnerObject(const SpawnerData&);

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
