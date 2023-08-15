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
    int map_w, map_h;
    int _wavesNum;
    struct EnemyWave
    {
        int delay;
        int timeout;
        int quantity;
    };

    std::vector<EnemyWave> _waves;
public:
    virtual int parseMapFile(std::string fileName) = 0;
    virtual int buildMapFromData() = 0;
    int mapWidth() const { return map_w; }
    int mapHeight() const { return map_h; }

protected:
    GameObject *buildObject(std::string type);
    SpawnController *createSpawnController(GameObject *parent, std::string type, int wave);

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
