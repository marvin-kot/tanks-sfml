#pragma once

#include <fstream>
#include <nlohmann/json.hpp>


using json = nlohmann::json;


/* std::ifstream f("example.json");
json */
class MapCreatorFromJson
{
    json data;
public:

    MapCreatorFromJson() {}

    int parseJsonMap(std::string jsonName) {
        std::ifstream f(jsonName);
        data = json::parse(f);

        return 0;
    }

    int buildMapFromData() {
        for (json::iterator it = data.begin(); it != data.end(); ++it) {
            json j = *it;

            std::string objType = j["type"];
            int x = j["x"];
            int y = j["y"];

            GameObject *object = buildObject(objType, x, y);
            if (object != nullptr)
                ObjectsPool::obstacles.insert(object);
        }

        return 0;
    }


    GameObject *buildObject(std::string type, int x, int y)
    {
        if (type == "brickWall") {
            GameObject *wall = new GameObject("brickWall");
            wall->createSpriteRenderer();
            wall->setPos(x*64 + 32, y*64 + 32);

            return wall;
        }
        if (type == "concreteWall") {
            GameObject *wall = new GameObject("concreteWall");
            wall->createSpriteRenderer();
            wall->setPos(x*64 + 32, y*64 + 32);

            return wall;
        }

        return nullptr;
    }
};