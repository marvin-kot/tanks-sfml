#include "MapCreator.h"
#include "EagleController.h"
#include "GameObject.h"
#include "Controller.h"
#include "Shootable.h"
#include "Damageable.h"
#include "ObjectsPool.h"
#include "GlobalConst.h"
#include "SpriteRenderer.h"

#include <string>

int MapCreator::placeSpawnerObjects()
{
    constexpr int tileSize = globalConst::spriteOriginalSizeX;
    constexpr int tileCenter = tileSize / 2;
    for (auto spawner : _spawners) {
        GameObject *object = createSpawnerObject(spawner);
        if (object == nullptr)
            return -1;
        object->setSize(tileSize, tileSize);
        object->setPosition(spawner.col * tileSize + tileCenter, spawner.row * tileSize + tileCenter);
        ObjectsPool::addObject(object);
    }

    return 0;
}

GameObject *MapCreator::createSpawnerObject(const SpawnerData& data)
{
    Logger::instance() << "Creating an enemy spawner...\n";

    GameObject *object = new GameObject(std::string("spawner_") + data.type);
    object->setFlags(GameObject::TankPassable | GameObject::BulletPassable);
    object->setRenderer(new LoopAnimationSpriteRenderer(object, "spark"));

    SpawnController *controller = new SpawnController(object, data.type, data.delay, data.timeout, data.quantity);
    object->setController(controller);

    return object;
}

GameObject *MapCreator::buildObject(std::string type)
{
    if (type == "spawner_player") {
        GameObject *spawner = new GameObject("spawner_player");
        spawner->setFlags(GameObject::PlayerSpawner | GameObject::TankPassable | GameObject::BulletPassable);
        spawner->setRenderer(new LoopAnimationSpriteRenderer(spawner, "spark"));
        spawner->setController(new PlayerSpawnController(spawner, globalVars::player1Lives, globalVars::player1PowerLevel));

        return spawner;
    }

    if (type == "eagle") {
        GameObject *eagle = new GameObject("eagle");
        eagle->setFlags(GameObject::Eagle | GameObject::BulletKillable);
        eagle->setRenderer(new SpriteRenderer(eagle));
        eagle->setDamageable(new Damageable(eagle, 1));
        eagle->setController(new EagleController(eagle));

        return eagle;
    }

    if (type == "brickWall") {
        GameObject *wall = new GameObject("brickWall");
        wall->setFlags(GameObject::BulletKillable | GameObject::Static);
        wall->setRenderer(new SpriteRenderer(wall));
        wall->setDamageable(new Damageable(wall, 1));

        return wall;
    }
    // wall parts
    if (type == "brickWall1x1" || type == "brickWall2x1" || type == "brickWall1x2" || type == "brickWall2x2") {
        GameObject *wall = new GameObject(type);
        wall->setFlags(GameObject::BulletKillable | GameObject::Static);
        wall->setRenderer(new SpriteRenderer(wall));
        wall->setDamageable(new Damageable(wall, 1));

        return wall;
    }


    if (type == "concreteWall") {
        GameObject *wall = new GameObject("concreteWall");
        wall->setFlags(GameObject::Static);
        wall->setRenderer(new SpriteRenderer(wall));
        auto damageable = new Damageable(wall, 0);
        damageable->makeInvincible(true);
        wall->setDamageable(damageable);

        return wall;
    }

    if (type == "tree") {
        GameObject *tree = new GameObject("tree");
        tree->setFlags(GameObject::TankPassable | GameObject::BulletPassable | GameObject::Static);
        tree->setRenderer(new SpriteRenderer(tree));

        return tree;
    }

    if (type == "water") {
        GameObject *water = new GameObject("water");
        water->setFlags(GameObject::BulletPassable | GameObject::Static);
        water->setRenderer(new SpriteRenderer(water));
        water->setCurrentAnimation("default");

        return water;
    }

    if (type == "ice") {
        GameObject *water = new GameObject("ice");
        water->setFlags(GameObject::BulletPassable | GameObject::TankPassable | GameObject::Static | GameObject::Ice);
        water->setRenderer(new SpriteRenderer(water));
        water->setCurrentAnimation("default");

        return water;
    }

    return nullptr;
}

void MapCreator::setupScreenBordersBasedOnMapSize()
{
    assert(map_w > 0 && map_h > 0);
    globalVars::borderWidth = (globalConst::screen_w - globalVars::mapViewPort.width)/2; //(globalConst::screen_w - mapWidth()*globalConst::spriteDisplaySizeX) / 2;
    globalVars::borderHeight = (globalConst::screen_h - globalVars::mapViewPort.height)/2;// (globalConst::screen_h - mapHeight()*globalConst::spriteDisplaySizeY) / 2;
    globalVars::gameViewPort = sf::IntRect(globalVars::borderWidth, globalVars::borderHeight, globalVars::mapViewPort.width, globalVars::mapViewPort.height);

    // WARNING: side effect - save map size globally
    globalVars::mapSize = sf::Vector2i(mapWidth() * globalConst::spriteOriginalSizeX, mapHeight() * globalConst::spriteOriginalSizeX);
}


// ############ Custom Matrix Text ###################

MapCreatorFromCustomMatrixFile::MapCreatorFromCustomMatrixFile()
{
    charMap = {
            {'@', "spawner_player"},
            {'#', "brickWall"},
            {'*', "concreteWall"},
            {'!', "eagle"},
            {'&', "tree"},
            {'~', "water"},
            {'_', "ice"}};
}

int MapCreatorFromCustomMatrixFile::parseMapFile(std::string fileName)
{
    // file format:
    // line 1: width, height
    // next lines: map

    std::ifstream f(fileName);
    std::string line;
    // line 0: map name
    {
        std::getline(f, _name);
        assert(_name.length() > 0 && _name.length() < 100);
    }

    // line 1: goal of the level
    {
        std::getline(f, _goal);
        assert(_goal.length() > 0 && _goal.length() < 100);
    }
    // line 2: map size
    {
        std::string dummy;
        std::getline(f, line);
        std::istringstream iss(line);
        iss >> dummy >> map_w >> map_h;
        Logger::instance() << "Read map size: " << map_w << map_h << "\n";
    }

    assert(map_w > 0 && map_w < globalConst::maxFieldWidth);
    assert(map_h > 0 && map_h < globalConst::maxFieldHeight);

    // read the map
    for (int i = 0; i < map_h; i++) {
        std::getline(f, line);
        mapString.append(line);
    }

    // read data for placing tank spawners
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#')
            continue;
        std::istringstream iss(line);
        SpawnerData sd;
        iss >> sd.type >> sd.col >> sd.row >> sd.delay >> sd.timeout >> sd.quantity;

        assert(sd.type.empty() == false);
        assert(sd.row >= 0 && sd.row < globalConst::maxFieldWidth);
        assert(sd.col >= 0 && sd.col < globalConst::maxFieldHeight);
        assert(sd.delay >= 0 && sd.delay < 600);
        assert(sd.timeout >= 0 && sd.timeout < 60);
        assert(sd.quantity > 0 && sd.quantity < 100);

        _spawners.push_back(sd);
    }

    return 0;
}

int MapCreatorFromCustomMatrixFile::buildMapFromData()
{
    constexpr int basicTileSize = globalConst::spriteOriginalSizeX;
    constexpr int tileCenter = basicTileSize / 2;
    constexpr int subtileSize = basicTileSize / 2;
    constexpr int subtileCenter = subtileSize / 2;
    // screen border properties must be set before building the map to place objects correctly
    setupScreenBordersBasedOnMapSize();
    bool playerCreated = false;
    for (int y = 0; y < map_h; y++) {
        for (int x = 0; x < map_w; x++) {
            char tile = mapString[y*map_w + x];
            auto it = charMap.find(tile);
            if (it != charMap.end()) {
                using namespace std;
                string objType = it->second;
                if (objType == "brickWall") {
                    vector<string> parts = {"brickWall1x1", "brickWall2x1", "brickWall1x2", "brickWall2x2"};
                    int i=0;
                    for (auto part : parts) {
                        GameObject *object = MapCreator::buildObject(part);
                        assert(object != nullptr);
                        object->setSize(subtileSize, subtileSize);
                        object->setPosition(
                            x*basicTileSize + subtileCenter + (i%2)*subtileSize,
                            y*basicTileSize + subtileCenter + (i/2)*subtileSize);
                        ObjectsPool::addObject(object);
                        i++;
                    }
                    continue;
                }
                GameObject *object = MapCreator::buildObject(objType);
                if (object != nullptr) {
                    object->setSize(basicTileSize, basicTileSize);
                    object->setPosition(x*basicTileSize + tileCenter, y*basicTileSize + tileCenter);
                    ObjectsPool::addObject(object);
                    if (object->type() == "spawner_player") {
                        ObjectsPool::playerSpawnerObject = object;
                        playerCreated = true;
                    }

                    if (object->type() == "eagle") {
                        ObjectsPool::eagleObject = object;
                    }
                }
            }
        }
    }

    if (!playerCreated) {
        Logger::instance() << "[Error] No player created";
        return -1;
    }
    return 0;
}


