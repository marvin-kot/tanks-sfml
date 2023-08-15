#include "MapCreator.h"

#include "GameObject.h"
#include "Controller.h"
#include "Shootable.h"
#include "Damageable.h"
#include "ObjectsPool.h"
#include "GlobalConst.h"
#include "SpriteRenderer.h"

#include <string>

SpawnController *MapCreator::createSpawnController(GameObject *parent, std::string type, int wave)
{
    assert(!type.empty());
    assert(wave>=0 && wave<3);
    assert(wave < _waves.size());

    return new SpawnController(parent, type, _waves[wave].delay, _waves[wave].timeout, _waves[wave].quantity);
}

#include <string_view>
static bool endsWith(std::string_view str, std::string_view suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

static int determineWaveIndex(std::string type)
{
    if (endsWith(type, "w1"))
        return 0;
    if (endsWith(type, "w2"))
        return 1;
    if (endsWith(type, "w3"))
        return 2;

    assert(false);
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

    if (type.rfind("spawner_BaseTank", 16) == 0) {
        Logger::instance() << "Creating an enemy spawner...\n";
        GameObject *spawner = new GameObject("spawner_BaseTank");
        spawner->setFlags(GameObject::TankPassable | GameObject::BulletPassable);
        spawner->setRenderer(new LoopAnimationSpriteRenderer(spawner, "spark"));
        SpawnController *controller = createSpawnController(spawner, "npcBaseTank", determineWaveIndex(type));
        spawner->setController(controller);

        return spawner;
    }

    if (type.rfind("spawner_FastTank", 16) == 0) {
        Logger::instance() << "Creating an enemy spawner...\n";
        GameObject *spawner = new GameObject("spawner_FastTank");
        spawner->setFlags(GameObject::TankPassable | GameObject::BulletPassable);
        spawner->setRenderer(new LoopAnimationSpriteRenderer(spawner, "spark"));
        SpawnController *controller = createSpawnController(spawner, "npcFastTank", determineWaveIndex(type));
        spawner->setController(controller);

        return spawner;
    }
    if (type.rfind("spawner_PowerTank", 17) == 0) {
        Logger::instance() << "Creating an enemy spawner...\n";
        GameObject *spawner = new GameObject("spawner_PowerTank");
        spawner->setFlags(GameObject::TankPassable | GameObject::BulletPassable);
        spawner->setRenderer(new LoopAnimationSpriteRenderer(spawner, "spark"));
        SpawnController *controller = createSpawnController(spawner, "npcPowerTank", determineWaveIndex(type));
        spawner->setController(controller);

        return spawner;
    }

    if (type.rfind("spawner_ArmorTank", 17) == 0) {
        Logger::instance() << "Creating an enemy spawner...\n";
        GameObject *spawner = new GameObject("spawner_ArmorTank");
        spawner->setFlags(GameObject::TankPassable | GameObject::BulletPassable);
        spawner->setRenderer(new LoopAnimationSpriteRenderer(spawner, "spark"));
        SpawnController *controller = createSpawnController(spawner, "npcArmorTank", determineWaveIndex(type));
        spawner->setController(controller);

        return spawner;
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
            {'b', "spawner_BaseTank_w1"},
            {'B', "spawner_BaseTank_w2"},
            {'8', "spawner_BaseTank_w3"},
            {'f', "spawner_FastTank_w1"},
            {'F', "spawner_FastTank_w2"},
            {'1', "spawner_FastTank_w3"},
            {'p', "spawner_PowerTank_w1"},
            {'P', "spawner_PowerTank_w2"},
            {'9', "spawner_PowerTank_w3"},
            {'a', "spawner_ArmorTank_w1"},
            {'A', "spawner_ArmorTank_w2"},
            {'4', "spawner_ArmorTank_w3"},
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
    // line 1: map size
    {
        std::string dummy;
        std::getline(f, line);
        std::istringstream iss(line);
        iss >> dummy >> map_w >> map_h;
        Logger::instance() << "Read map size: " << map_w << map_h << "\n";
    }

    {
        std::getline(f, line);
        std::istringstream iss(line);
        std::string dummy;
        iss >> dummy >> _wavesNum;
        Logger::instance() << "Read waves: " << _wavesNum << "\n";

        // so far only up to 3 waves are supported
        assert(_wavesNum > 0 && _wavesNum < 4);

        for (int i=0; i<_wavesNum; i++) {
            EnemyWave w;
            std::getline(f, line);
            std::istringstream issl(line);
            issl >> dummy >> w.delay >> dummy >> w.timeout >> dummy >> w.quantity;
            Logger::instance() << "wave " << i << " delay " << w.delay << ", timeout " << w.timeout << " q " << w.quantity << "\n";
            assert(w.delay>=0 && w.delay<600);
            assert(w.timeout>0 && w.timeout<60);
            assert(w.quantity>0 && w.quantity<60);
            _waves.push_back(w);
        }
    }



    // read the rest of the file
    while (std::getline(f, line))
    {
        mapString.append(line);
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


