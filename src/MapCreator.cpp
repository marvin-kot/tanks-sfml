#include "MapCreator.h"
#include "EagleController.h"
#include "GameObject.h"
#include "Controller.h"
#include "Shootable.h"
#include "Damageable.h"
#include "ObjectsPool.h"
#include "GlobalConst.h"
#include "SpriteRenderer.h"
#include "Utils.h"

#include <string>
#include <set>

namespace Level {
    std::map<WinCondition, const char *> winDescriptionsMap = {
        { SurviveTime, "Survive %d minutes" },
        { KillEmAll, "Kill all enemies" },
        { ObtainFlag, "Obtain the flag" }
    };

    std::map<FailCondition, const char *> failDescriptionsMap = {
        { LoseBase, "Lose the base or all your tanks" },
        { ExpireTime, "Fail to complete your task in %d minutes" }
    };

}



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
    object->setRenderer(new LoopAnimationSpriteRenderer(object, "spark"), 4);

    SpawnController *controller = new SpawnController(object, data.type, data.delay, data.timeout, data.quantity);
    object->setController(controller);

    return object;
}

GameObject *MapCreator::buildObject(std::string type)
{
    if (type.empty())
        return nullptr;

    if (type == "spawner_player") {
        assert(ObjectsPool::playerSpawnerObject == nullptr);
        GameObject *spawner = new GameObject("spawner_player");
        spawner->setFlags(GameObject::PlayerSpawner | GameObject::TankPassable | GameObject::BulletPassable);
        spawner->setRenderer(new LoopAnimationSpriteRenderer(spawner, "spark"), 4);
        spawner->setController(new PlayerSpawnController(spawner, globalVars::player1Lives, globalVars::player1PowerLevel));

        return spawner;
    }

    if (type == "eagle") {
        assert(ObjectsPool::eagleObject == nullptr);
        GameObject *eagle = new GameObject("eagle");
        eagle->setFlags(GameObject::Eagle | GameObject::BulletKillable);
        eagle->setRenderer(new SpriteRenderer(eagle), 2);
        eagle->setDamageable(new Damageable(eagle, globalConst::DefaultBaseProtection));
        eagle->setController(new EagleController(eagle));

        return eagle;
    }

    if (type == "brickWall") {
        GameObject *wall = new GameObject("brickWall");
        wall->setFlags(GameObject::BulletKillable | GameObject::Static);
        wall->setRenderer(new SpriteRenderer(wall), 3);
        wall->setDamageable(new Damageable(wall, 1));

        return wall;
    }
    // wall parts
    if (type.rfind("brickWall", 0, 9) != std::string::npos) {
    //if (type == "brickWall1x1" || type == "brickWall2x1" || type == "brickWall1x2" || type == "brickWall2x2") {
        GameObject *wall = new GameObject(type);
        wall->setFlags(GameObject::BulletKillable | GameObject::Static);
        wall->setRenderer(new SpriteRenderer(wall), 3);
        wall->setDamageable(new Damageable(wall, 1));

        return wall;
    }

    if (type == "concreteWall") {
        GameObject *wall = new GameObject("concreteWall");
        wall->setFlags(GameObject::Static);
        wall->setRenderer(new SpriteRenderer(wall), 3);
        auto damageable = new Damageable(wall, 0);
        damageable->makeInvincible(true);
        wall->setDamageable(damageable);

        return wall;
    }

    if (type == "tree") {
        GameObject *tree = new GameObject("tree");
        tree->setFlags(GameObject::TankPassable | GameObject::BulletPassable | GameObject::Static);
        tree->setRenderer(new SpriteRenderer(tree), 3);

        return tree;
    }

    if (type == "concrete_floor") {
        GameObject *floor = new GameObject("concrete_floor");
        floor->setFlags(GameObject::TankPassable | GameObject::BulletPassable | GameObject::Static);
        floor->setRenderer(new SpriteRenderer(floor), 0);

        return floor;
    }

    if (type == "underbrick_floor") {
        GameObject *floor = new GameObject("underbrick_floor");
        floor->setFlags(GameObject::TankPassable | GameObject::BulletPassable | GameObject::Static);
        floor->setRenderer(new SpriteRenderer(floor), 0);

        return floor;
    }

    if (type == "water") {
        GameObject *water = new GameObject("water");
        water->setFlags(GameObject::BulletPassable | GameObject::Static);
        water->setRenderer(new SpriteRenderer(water), 0);
        water->setCurrentAnimation("default");

        return water;
    }

    if (type.rfind("water-border", 0, 12) != std::string::npos) {
        GameObject *water = new GameObject(type);
        water->setFlags(GameObject::TankPassable | GameObject::BulletPassable | GameObject::Static);
        water->setRenderer(new SpriteRenderer(water), 1);
        water->setCurrentAnimation("default");

        return water;
    }

    if (type.rfind("bridge", 0, 6) != std::string::npos) {
        GameObject *bridge = new GameObject("bridge");
        bridge->setFlags(GameObject::TankPassable | GameObject::BulletPassable | GameObject::Static);
        bridge->setRenderer(new SpriteRenderer(bridge), 0);
        bridge->setCurrentAnimation("default");

        if (type == "bridge-horizontal")
            bridge->setCurrentDirection(globalTypes::Direction::Left);

        return bridge;
    }


    if (type == "ice") {
        GameObject *ice = new GameObject("ice");
        ice->setFlags(GameObject::BulletPassable | GameObject::TankPassable | GameObject::Static | GameObject::Ice);
        ice->setRenderer(new SpriteRenderer(ice), 0);
        ice->setCurrentAnimation("default");

        return ice;
    }

    if (type == "jezek") {
        GameObject *jezek = new GameObject("enemyJezek");
        jezek->setFlags(GameObject::BulletPassable | GameObject::TankPassable | GameObject::Static);
        jezek->setController(new JezekController(jezek, false));
        jezek->setRenderer(new SpriteRenderer(jezek), 1);
        jezek->setCurrentAnimation("default");

        return jezek;
    }

    if (type.rfind("car-", 0, 4) != std::string::npos) {
        GameObject *car = new GameObject(type);
        car->setFlags(GameObject::BulletKillable | GameObject::Static);
        car->setController(new StaticCarController(car));
        car->setRenderer(new SpriteRenderer(car), 2);
        car->setDamageable(new Damageable(car, 0));
        car->setCurrentAnimation("default");
        return car;
    }

    if (type.rfind("debris-", 0, 7) != std::string::npos) {
        GameObject *car = new GameObject(type);
        car->setFlags(GameObject::TankPassable | GameObject::BulletPassable | GameObject::Static);
        car->setRenderer(new SpriteRenderer(car), 1);
        car->setCurrentAnimation("default");
        return car;
    }

    if (type.rfind("road_t_cross_", 0, 13) != std::string::npos) {
        GameObject *road = new GameObject("road_t_cross");
        road->setFlags(GameObject::TankPassable | GameObject::BulletPassable | GameObject::Static);
        road->setRenderer(new SpriteRenderer(road), 0);
        road->setCurrentAnimation("default");

        std::string dir = type.substr(13);
        if (dir == "left")
            road->setCurrentDirection(globalTypes::Direction::Left);
        else if (dir == "right")
            road->setCurrentDirection(globalTypes::Direction::Right);
        else if (dir == "down")
            road->setCurrentDirection(globalTypes::Direction::Down);

        return road;
    }



    // else - consider it's floor
    GameObject *floor = new GameObject(type);
    floor->setFlags(GameObject::TankPassable | GameObject::BulletPassable | GameObject::Static);
    floor->setRenderer(new SpriteRenderer(floor), 0);

    return floor;

}

void MapCreator::setupScreenBordersBasedOnMapSize()
{
    assert(map_w > 0 && map_h > 0);
    globalVars::borderWidth = (globalConst::screen_w - globalVars::mapViewPort.width)/2;
    globalVars::borderHeight = (globalConst::screen_h - globalVars::mapViewPort.height)/2;
    globalVars::gameViewPort = sf::IntRect(globalVars::borderWidth, globalVars::borderHeight, globalVars::mapViewPort.width, globalVars::mapViewPort.height);

    // WARNING: side effect - save map size globally
    globalVars::mapSize = sf::Vector2i(mapWidth() * globalConst::spriteOriginalSizeX, mapHeight() * globalConst::spriteOriginalSizeX);
}


// ############ Custom Matrix Text ###################

MapCreator::MapCreator()
{
    charMap = {
            {'@', "spawner_player"},
            {'.', "concrete_floor"},
            {',', "grass_floor"},
            {'#', "brickWall"},
            {'*', "concreteWall"},
            {'!', "eagle"},
            {'&', "tree"},
            {'~', "water"},
            {'_', "ice"},
            {'x', "jezek"},
            {'-', "road"},
            {'+', "bridge"},
            /*{'-', "road_horizontal"},
            {'\"', "road_horizontal_broken"},
            {'|', "road_vertical"},
            {':', "road_vertical_broken"},
            {'+', "road_cross"},
            {'T', "road_t_cross_down"},
            {'<', "road_t_cross_left"},
            {'>', "road_t_cross_right"},
            {'1', "road_t_cross_up"},
            {'r', "road_top_left"},
            {'7', "road_top_right"},
            {'L', "road_bottom_left"},
            {'j', "road_bottom_right"},*/
            {'=', "carsHorizontal"},
            {'H', "carsVertical"},
            };
}

int MapCreator::parseMapFile(std::string fileName)
{
    // file format:
    // line 1: width, height
    // next lines: map

    std::ifstream f(fileName);
    char cstr[256];

    strcpy(cstr, fileName.c_str());
    std::string line;
    // line 0: map name
    {
        std::getline(f, _currLevelProperies.name);
        assert(_currLevelProperies.name.length() > 0 && _currLevelProperies.name.length() < 100);
    }

    // line 1: win conditions
    {
        std::getline(f, line);
        std::istringstream iss(line);
        std::string cond;
        int  par;

        iss >> cond >> par;
        assert(Level::winMeaningsMap.find(cond) != Level::winMeaningsMap.end());
        _currLevelProperies.win = Level::winMeaningsMap.at(cond);
        _currLevelProperies.winParam = par;
    }
    // line 2: fail conditions
    {
        std::getline(f, line);
        std::istringstream iss(line);
        std::string cond;
        int  par;

        iss >> cond >> par;
        assert(Level::failMeaningsMap.find(cond) != Level::failMeaningsMap.end());
        _currLevelProperies.fail = Level::failMeaningsMap.at(cond);
        _currLevelProperies.failParam = par;
    }

    // line 3 num of briefing lines
    {
        std::getline(f, line);
        int num;
        std::istringstream iss(line);
        iss >> num;
        assert(num>=0 && num < 10);
        // read briefing line by line
        for (int i = 0; i < num; i++) {
            std::getline(f, line);
            _currLevelProperies.briefing.push_back(line);
        }
    }

    // map size
    {
        std::string dummy;
        std::getline(f, line);
        std::istringstream iss(line);
        iss >> dummy >> map_w >> map_h;
        assert(dummy == "map");
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
        assert(sd.delay >= 0 && sd.delay < 1000);
        assert(sd.timeout >= 0 && sd.timeout < 120);
        assert(sd.quantity > 0 && sd.quantity < 100);

        _spawners.push_back(sd);
    }

    return 0;
}

void MapCreator::placeBrickWall(int x, int y)
{
    constexpr int basicTileSize = globalConst::spriteOriginalSizeX;
    constexpr int tileCenter = basicTileSize / 2;
    constexpr int subtileSize = basicTileSize / 2;
    constexpr int subtileCenter = subtileSize / 2;
    using namespace std;

    vector<string> parts = {"brickWall1x1", "brickWall2x1", "brickWall1x2", "brickWall2x2"};
    int i=0;
    for (auto part : parts) {
        GameObject *object = MapCreator::buildObject(part);
        assert(object != nullptr);
        object->setSize(subtileSize, subtileSize);
        object->setPosition(
            x + subtileCenter + (i%2)*subtileSize,
            y + subtileCenter + (i/2)*subtileSize);
        ObjectsPool::addObject(object);
        i++;
    }

    // put floor under brick
    GameObject *object = MapCreator::buildObject("underbrick_floor");
    object->setSize(basicTileSize, basicTileSize);
    object->setPosition(x + tileCenter, y + tileCenter);
    ObjectsPool::addObject(object);

}

void MapCreator::placeStaticCars(int x, int y, bool vert)
{
    constexpr int basicTileSize = globalConst::spriteOriginalSizeX;
    constexpr int tileCenter = basicTileSize / 2;
    constexpr int subtileSize = basicTileSize / 2;
    constexpr int subtileCenter = subtileSize / 2;
    using namespace std;

    vector<string> parts;

    vector<string> carSprite;
    vector<string> debrisSprite;


    std::uniform_int_distribution<int> type_distr(0, 3);

    for (int i=0; i<2; i++) {
        int carIndex = type_distr(Utils::generator);

        switch (carIndex) {
            case 0:
                carSprite.push_back("car-1-left");
                debrisSprite.push_back("debris-car-1");
                break;
            case 1:
                carSprite.push_back("car-1-right");
                debrisSprite.push_back("debris-car-1");
                break;
            case 2:
                carSprite.push_back("car-2-left");
                debrisSprite.push_back("debris-car-2");
                break;
            case 3:
                carSprite.push_back("car-2-right");
                debrisSprite.push_back("debris-car-2");
                break;
        }
    }

    const int centerX = x + tileCenter;
    const int centerY = y + tileCenter;

    for (int i=0; i<2; i++) {
        GameObject *car = MapCreator::buildObject(carSprite.at(i));
        assert(car != nullptr);
        int posX = vert ? (centerX - subtileCenter + i*subtileSize) : centerX;
        int posY = vert ? centerY : (centerY - subtileCenter + i*subtileSize);
        car->setPosition(posX, posY);

        bool randMirror = (type_distr(Utils::generator) < 2);

        if (vert)
            car->setCurrentDirection(randMirror ? globalTypes::Direction::Down : globalTypes::Direction::Up);
        else
            car->setCurrentDirection(randMirror ? globalTypes::Direction::Left : globalTypes::Direction::Right);

        ObjectsPool::addObject(car);

        // set debris
        GameObject *debris = MapCreator::buildObject(debrisSprite.at(i));
        debris->setPosition(posX, posY);
        ObjectsPool::addObject(debris);

        if (vert)
            debris->setCurrentDirection(randMirror ? globalTypes::Direction::Down : globalTypes::Direction::Up);
        else
            debris->setCurrentDirection(randMirror ? globalTypes::Direction::Left : globalTypes::Direction::Right);

    }

    // put floor under cars
    GameObject *object = MapCreator::buildObject("concrete_floor");
    object->setSize(basicTileSize, basicTileSize);
    object->setPosition(centerX, centerY);
    ObjectsPool::addObject(object);

}

void MapCreator::placeWater(int xTile, int yTile)
{
    constexpr int basicTileSize = globalConst::spriteOriginalSizeX;
    constexpr int tileCenter = basicTileSize / 2;
    constexpr int subtileSize = basicTileSize / 2;
    constexpr int subtileCenter = subtileSize / 2;

    GameObject *water = MapCreator::buildObject("water");
    water->setSize(basicTileSize, basicTileSize);
    water->setPosition(xTile*basicTileSize + tileCenter, yTile*basicTileSize + tileCenter);
    ObjectsPool::addObject(water);

    std::set<char> waterBodies = {'~', '+'};

    // add borders
    if (xTile>0 && !waterBodies.contains(charFromMap(xTile-1, yTile))) {
        GameObject *border = MapCreator::buildObject("water-border-left");
        border->setPosition(xTile*basicTileSize + 1, yTile*basicTileSize + tileCenter);
        ObjectsPool::addObject(border);
    }

    if (xTile<(map_w-1) && !waterBodies.contains(charFromMap(xTile+1, yTile))) {
        GameObject *border = MapCreator::buildObject("water-border-right");
        border->setPosition(xTile*basicTileSize + basicTileSize - 2, yTile*basicTileSize + tileCenter);
        ObjectsPool::addObject(border);
    }

    if (yTile>0 && !waterBodies.contains(charFromMap(xTile, yTile-1))) {
        GameObject *border = MapCreator::buildObject("water-border-top");
        border->setPosition(xTile*basicTileSize + tileCenter, yTile*basicTileSize + 1);
        ObjectsPool::addObject(border);
    }

    if (yTile<(map_h-1) && !waterBodies.contains(charFromMap(xTile, yTile+1))) {
        GameObject *border = MapCreator::buildObject("water-border-bottom");
        border->setPosition(xTile*basicTileSize + tileCenter, yTile*basicTileSize + basicTileSize - 2);
        ObjectsPool::addObject(border);
    }
}

void MapCreator::placeRoad(int xTile, int yTile)
{
    constexpr int basicTileSize = globalConst::spriteOriginalSizeX;
    constexpr int tileCenter = basicTileSize / 2;
    constexpr int subtileSize = basicTileSize / 2;
    constexpr int subtileCenter = subtileSize / 2;

    bool onLeft = xTile>0 && charFromMap(xTile-1, yTile) == '-';
    bool onRight = xTile<(map_w-1) && charFromMap(xTile+1, yTile) == '-';
    bool onTop = yTile>0 && charFromMap(xTile, yTile-1) == '-';
    bool onBottom = yTile<(map_h-1) && charFromMap(xTile, yTile+1) == '-';



    std::string roadSprite;

    std::uniform_int_distribution<int> type_distr(0, 3);
    int roadIndex = type_distr(Utils::generator);

    if (!onLeft && !onRight) {
        roadSprite = roadIndex > 0 ? "road_vertical" : "road_vertical_broken";
    }
    else if (!onTop && !onBottom) {
        roadSprite = roadIndex > 0 ? "road_horizontal" : "road_horizontal_broken";
    }
    else if (onLeft && onRight && onTop && !onBottom)
        roadSprite = "road_t_cross_up";
    else if (onLeft && onRight && !onTop && onBottom)
        roadSprite = "road_t_cross_down";
    else if (!onLeft && onRight && onTop && onBottom)
        roadSprite = "road_t_cross_right";
    else if (onLeft && !onRight && onTop && onBottom)
        roadSprite = "road_t_cross_left";
    else if (!onLeft && onRight && !onTop && onBottom)
        roadSprite = "road_top_left";
    else if (onLeft && !onRight && !onTop && onBottom)
        roadSprite = "road_top_right";
    else if (onLeft && !onRight && onTop && !onBottom)
        roadSprite = "road_bottom_right";
    else if (!onLeft && onRight && onTop && !onBottom)
        roadSprite = "road_bottom_left";
    else
        roadSprite = "road_cross";

    GameObject *road = MapCreator::buildObject(roadSprite);

    const int x = xTile*basicTileSize + tileCenter;
    const int y = yTile*basicTileSize + tileCenter;
    road->setPosition(x, y);
    ObjectsPool::addObject(road);

}


void MapCreator::placeBridge(int xTile, int yTile)
{
    constexpr int basicTileSize = globalConst::spriteOriginalSizeX;
    constexpr int tileCenter = basicTileSize / 2;
    constexpr int subtileSize = basicTileSize / 2;
    constexpr int subtileCenter = subtileSize / 2;

    bool onLeft = xTile>0 && charFromMap(xTile-1, yTile) == '~';
    bool onRight = xTile<(map_w-1) && charFromMap(xTile+1, yTile) == '~';
    bool onTop = yTile>0 && charFromMap(xTile, yTile-1) == '~';
    bool onBottom = yTile<(map_h-1) && charFromMap(xTile, yTile+1) == '~';

    std::string roadSprite;

    if (onTop && onBottom)
        roadSprite = "bridge-horizontal";
    else
        roadSprite = "bridge-vertical";

    GameObject *road = MapCreator::buildObject(roadSprite);

    const int x = xTile*basicTileSize + tileCenter;
    const int y = yTile*basicTileSize + tileCenter;
    road->setPosition(x, y);
    ObjectsPool::addObject(road);

}


Level::Properties MapCreator::buildMapFromData()
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
                    placeBrickWall(x*basicTileSize, y*basicTileSize);
                    continue;
                }

                if (objType == "water") {
                    placeWater(x, y);
                    continue;
                }

                if (objType == "bridge") {
                    placeBridge(x, y);
                    continue;
                }

                if (objType == "road") {
                    placeRoad(x, y);
                    continue;
                }

                if (objType == "carsHorizontal") {
                    placeStaticCars(x*basicTileSize, y*basicTileSize, false);
                    continue;
                }
                if (objType == "carsVertical") {
                    placeStaticCars(x*basicTileSize, y*basicTileSize, true);
                    continue;
                }
                GameObject *object = MapCreator::buildObject(objType);
                if (object != nullptr) {
                    /*if (object->type() == "enemyJezek")
                        object->setSize(subtileSize, subtileSize);
                    else
                        object->setSize(basicTileSize, basicTileSize);*/
                    object->setPosition(x*basicTileSize + tileCenter, y*basicTileSize + tileCenter);
                    ObjectsPool::addObject(object);
                    bool placeFloorUnderObject = false;
                    if (object->type() == "spawner_player") {
                        ObjectsPool::playerSpawnerObject = object;
                        placeFloorUnderObject = true;
                        playerCreated = true;
                    }

                    if (object->type() == "eagle") {
                        ObjectsPool::eagleObject = object;
                        placeFloorUnderObject = true;
                    }

                    if (object->type() == "tree" || object->type() == "enemyJezek") {
                        placeFloorUnderObject = true;
                    }

                    if (placeFloorUnderObject) {
                        {
                            GameObject *floor = MapCreator::buildObject("concrete_floor");
                            //floor->setSize(basicTileSize, basicTileSize);
                            floor->setPosition(x*basicTileSize + tileCenter, y*basicTileSize + tileCenter);
                            ObjectsPool::addObject(floor);
                        }
                    }
                }
            }
        }
    }

    if (!playerCreated) {
        Logger::instance() << "[Error] No player created";
        _currLevelProperies.failedToLoad = true;
    }

    return _currLevelProperies;
}



