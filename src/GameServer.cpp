#include "GameServer.h"

#include "AssetManager.h"
#include "GameObject.h"
#include "GlobalConst.h"
#include "HUD.h"
#include "LevelUpPopupMenu.h"
#include "ObjectsPool.h"
#include "PlayerController.h"
#include "Logger.h"
#include "SoundPlayer.h"
#include "Utils.h"
#include "UiUtils.h"

#include <SFML/Graphics.hpp>

#include <format>
#include <algorithm>


namespace server
{

std::vector<std::string> levelMaps = {
    "assets/testmap4.txt",
    //"assets/testmap.txt",
    //"assets/testmap2.txt"
};


GameServer::GameServer() {}

bool GameServer::loadAssets()
{
    if (!AssetManager::instance().loadSpriteSheet("spriteSheet16x16")) {
        Logger::instance() << "[ERROR] Could not open assets/spriteSheet16x16.png";
        return false;
    }


    if (!AssetManager::instance().loadFont("assets/joystix.otf")) {
        Logger::instance() << "[ERROR] fonts not loaded";
        return false;
    }

    return true;
}


void GameServer::initializeVariables()
{
    Utils::window.create(sf::VideoMode(640, 480), "[SERVER] Retro Tank Massacre");
    Utils::window.setFramerateLimit(globalConst::FixedFrameRate);

    currentLevel = 0;
    framesToDie = -1;
    framesToWin = -1;
    gameState = WaitingForStartRequest;

    _recvSocket.bind(net::CLIENT_SEND_PORT);
    _sendSocket.setBlocking(false);
    _recvSocket.setBlocking(false);
}

bool GameServer::isWindowOpen() const
{
    return Utils::window.isOpen();
}

void GameServer::processWindowEvents()
{
    sf::Event event;
    while (Utils::window.pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                Utils::window.close();
                break;
            case sf::Event::KeyPressed:
                if (event.key.scancode == sf::Keyboard::Scan::Escape)
                    Utils::window.close();
                break;
        }
    }

}

bool GameServer::update()
{
    updateFrameClock();

    // receive commands from client
    processWindowEvents();


    int stateResult = processStateChange();
    if (stateResult == -1) {
        return false;
    } else if (stateResult == 0) {
        return true;
    }
    // else if 1 - proceed

    updateAllObjectControllers();

    if (!globalVars::gameIsPaused) {
        processDeletedObjects();
    }

    if (!globalVars::gameIsPaused)
        recalculateViewPort();

    // populate thin object container
    networkDraw();
    // send world state to client
    _lastFrame.frame_num = Utils::currentFrame;

    int i = 0;
    for (auto it = ObjectsPool::thinGameObjects.cbegin(); it != ObjectsPool::thinGameObjects.cend(); ++it) {

        net::ThinGameObject thin = (*it).second;
        _lastFrame.objects[i] = thin;
        i++;
    }

    _lastFrame.num_objects = i;

    auto& sounds_to_stop = SoundPlayer::instance().getSoundsQueue(false);
    i=0;
    while (!sounds_to_stop.empty()) {
        _lastFrame.sounds_stop[i] = sounds_to_stop.front();
        sounds_to_stop.pop();
        i++;
    }
    _lastFrame.sstop_num = i;

    auto& sounds_to_play = SoundPlayer::instance().getSoundsQueue(true);
    i=0;
    while (!sounds_to_play.empty()) {
        Logger::instance() << "    Play sound " << (int)sounds_to_play.front() << "\n";
        _lastFrame.sounds_play[i] = sounds_to_play.front();
        sounds_to_play.pop();
        i++;
    }
    _lastFrame.splay_num = i;

    // send
    Logger::instance() << "send frame " << _lastFrame.frame_num << "\n";
    auto status = _sendSocket.send((void *)&_lastFrame, sizeof(net::FrameDetails), "127.0.0.1", net::SERVER_SEND_PORT);

    if (status != sf::Socket::Done)
        Logger::instance() << "UDP send Error" << status << "\n";


    /*if (LevelUpPopupMenu::instance().isOpen()) {
        SoundPlayer::instance().stopAllSounds();
        LevelUpPopupMenu::instance().draw();
    }*/

    // send sound commands if any

    if (!globalVars::gameIsPaused)
        checkStatePostFrame();

    // TEMP for debug screen
    drawPlayingScreen();
    Utils::window.display();
    return true;
}

void GameServer::updateFrameClock()
{
    sf::Time now = Utils::refreshClock.getElapsedTime();

    auto diff = now - Utils::lastFrameTime;
    auto fixedLength = sf::seconds(globalConst::FixedFrameLength);
    if (diff < fixedLength) {
        sf::sleep(fixedLength - diff);
    }

    Utils::lastFrameTime = now;

    Utils::refreshClock.restart();
    Utils::currentFrame++;
}


void GameServer::pause(bool p)
{
    using namespace globalVars;
    gameIsPaused = p;
    if (p) {
        SoundPlayer::instance().stopAllSounds();
        SoundPlayer::instance().playPauseSound();
        globalChronometer.pause();
        globalFreezeChronometer.pause();
    } else {
        globalChronometer.resume();
        globalFreezeChronometer.resume();
    }
}

int GameServer::processStateChange()
{
    if (gameState == WaitingForStartRequest) {
        sf::IpAddress sender;
        std::size_t received = 0;
        uint16_t port;
        uint16_t msg;
        _recvSocket.receive((void *)&msg, sizeof(msg), received, sender, port);
        if (received == sizeof(msg) && msg == net::SpecialMessage::StartGameRequest)
            gameState = LoadNextLevel;

        drawConnectionScreen();
        return 0;
    } else if (gameState == LoadNextLevel) {
        if (currentLevel >= levelMaps.size()) {
            Logger::instance() << "Load next level but it's over\n";
            gameState = WaitingForStartRequest;
            return 0;
        }

        framesToWin = -1; framesToDie = -1;
        SoundPlayer::instance().stopAllSounds();
        ObjectsPool::clearEverything();

        globalVars::player1Lives = globalConst::InitialLives;
        globalVars::player1PowerLevel = globalConst::InitialPowerLevel;

        Logger::instance() << "Building map..." << levelMaps[currentLevel] << "\n";
        if (!buildLevelMap(levelMaps[currentLevel])) {
            Logger::instance() << "Failed to build map\n";
            return -1;
        }
        Logger::instance() << "Level map is built\n";

        _sendSocket.send((void *)&_map, sizeof(net::MapDetails), "127.0.0.1", net::SERVER_SEND_PORT);
        gameState = WaitingForLevelStartRequest;
        return 0;
    } else if (gameState == WaitingForLevelStartRequest) {
        sf::IpAddress sender;
        std::size_t received = 0;
        uint16_t port;
        uint16_t msg;
        _recvSocket.receive((void *)&msg, sizeof(msg), received, sender, port);
        if (received == sizeof(msg) && msg == net::SpecialMessage::StartLevelRequest) {
            gameState = StartLevel;
        }
        drawStartingScreen();
        return 0;
    }
    else if (gameState == StartLevel) {
        Logger::instance() << "Starting level...\n";
        SoundPlayer::instance().gameOver = false;
        HUD::instance().showFail(false);
        HUD::instance().showWin(false);
        globalVars::globalChronometer.reset(true);
        gameState = PlayingLevel;
    }
    else if (gameState == GameOver) {
        SoundPlayer::instance().stopAllSounds();
        ObjectsPool::clearEverything();
        gameState = WaitingForStartRequest;
        return 0;
    } //else if (gameState == PlayingLevel)

    return 1;
}

bool GameServer::buildLevelMap(std::string fileName)
{
    MapCreator mapBuilder;
    mapBuilder.parseMapFile(fileName);
    _currentLevelProperties = mapBuilder.buildMapFromData();
    if (_currentLevelProperties.failedToLoad)
        return false;

    if (mapBuilder.mapWidth() > globalConst::maxFieldWidth || mapBuilder.mapHeight() > globalConst::maxFieldHeight) {
        Logger::instance() << "[ERROR] the map size exceeds the limits of the screen. Aborting game..." << fileName;
        return false;
    }

    mapBuilder.placeSpawnerObjects();

    _surviveTimeoutSec = 60 * 5;

    strcpy(_map.title, _currentLevelProperties.name.c_str());
    int briefingLines = std::min((int)(_currentLevelProperties.briefing.size()), 4); // TODO magin number
    _map.brief_lines = briefingLines;
    for (int i=0; i < briefingLines; i++) {
        strcpy(_map.briefing[i], _currentLevelProperties.briefing[i].c_str());
    }

    const Level::WinCondition win = _currentLevelProperties.win;
    const int winParam = _currentLevelProperties.winParam;
    {
        const char *placeholder = Level::winDescriptionsMap.at(win);
        sprintf(_map.win, placeholder, winParam);
    }
    const Level::FailCondition fail = _currentLevelProperties.fail;
    const int failParam = _currentLevelProperties.failParam;
    {
        const char *placeholder = Level::failDescriptionsMap.at(fail);
        sprintf(_map.fail, placeholder, winParam);
    }

    _map.w = mapBuilder.mapWidth();
    _map.h = mapBuilder.mapHeight();

    return true;
}

void GameServer::updateAllObjectControllers()
{
    auto &allObjects = ObjectsPool::getAllObjects();
    // update object states
    for (GameObject *obj : allObjects) {
        if (!obj->isFlagSet(GameObject::Static))
            obj->update();
    }
}

void GameServer::processDeletedObjects()
{
    auto &allObjects = ObjectsPool::getAllObjects();
    std::vector<GameObject *> objectsToAdd;

        // delete objects marked for deletion on previous step
        for (auto it = allObjects.begin(); it != allObjects.end(); ) {
            GameObject *obj = *it;
            if (obj->mustBeDeleted()) {
                if (obj->isFlagSet(GameObject::Player)) {
                    globalVars::player1Lives--;
                    ObjectsPool::playerObject = nullptr;
                }

                if (obj->isFlagSet(GameObject::Eagle)) {
                    ObjectsPool::eagleObject = nullptr;
                    // do not break here - we still need to create explosion few lines later!
                }

                if (obj->isFlagSet(GameObject::PlayerSpawner))
                    ObjectsPool::playerSpawnerObject = nullptr;

                if (obj->isFlagSet(GameObject::Bullet)) {
                    GameObject *explosion = new GameObject("smallExplosion");
                    explosion->setRenderer(new OneShotAnimationRenderer(explosion));
                    explosion->setFlags(GameObject::BulletPassable | GameObject::TankPassable);
                    explosion->copyParentPosition(obj);

                    objectsToAdd.push_back(explosion);

                    if (obj->getParentObject()->isFlagSet(GameObject::Player))
                        SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::bulletHitWall, true);
                }

                if (obj->isFlagSet(GameObject::Player | GameObject::NPC | GameObject::Eagle)) {
                    GameObject *explosion = new GameObject("bigExplosion");
                    explosion->setRenderer(new OneShotAnimationRenderer(explosion));
                    explosion->setFlags(GameObject::BulletPassable | GameObject::TankPassable);
                    explosion->copyParentPosition(obj);

                    objectsToAdd.push_back(explosion);

                    if (obj->isFlagSet(GameObject::Player | GameObject::Eagle))
                        SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::bigExplosion, true);
                    else
                        SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::smallExplosion, true);
                }

                if (obj->isFlagSet(GameObject::BonusOnHit)) {
                    obj->generateDrop();
                    SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::bonusAppear, true);
                } else
                    obj->dropXp();

                it = ObjectsPool::kill(it);
                delete obj;
            } else ++it;
        }

        // temporary explosion effects
        for (auto obj : objectsToAdd) {
            ObjectsPool::addObject(obj);
        }
}

void GameServer::recalculateViewPort()
{
    GameObject *centerObject = nullptr;

    if (ObjectsPool::playerObject)
        centerObject = ObjectsPool::playerObject;
    else if (ObjectsPool::playerSpawnerObject)
        centerObject = ObjectsPool::playerSpawnerObject;
    else
        return;

    using namespace globalConst;
    using namespace globalVars;
    // get (mapped) player coordinates
    auto ppos = centerObject->position();
    int mpX = ppos.x * spriteScaleX;
    int mpY = ppos.y * spriteScaleY;

    mapViewPort.left = mpX - mapViewPort.width/2;
    mapViewPort.top = mpY - mapViewPort.height/2;

    if (mapViewPort.left < 0)
        mapViewPort.left = 0;
    if (mapViewPort.top < 0)
        mapViewPort.top = 0;

    int realMapSizeX = mapSize.x * globalConst::spriteScaleX;
    int realMapSizeY = mapSize.y * globalConst::spriteScaleY;
    if (mapViewPort.left + mapViewPort.width > realMapSizeX)
        mapViewPort.left = realMapSizeX - mapViewPort.width;
    if (mapViewPort.top + mapViewPort.height > realMapSizeY)
        mapViewPort.top = realMapSizeY - mapViewPort.height;
}


void GameServer::networkDraw()
{
    ObjectsPool::thinGameObjects.clear();

    Logger::instance() << "Draw objects: \n";
    using namespace net;
    ThinGameObject o;
    auto objectsToDraw = ObjectsPool::getAllObjects();
    auto objectActor = [&o](GameObject *obj) {
        // TODO: populate the array of thin objecta
            o.id = obj->id();
            if (obj->networkDraw(o)) {
                //Logger::instance() << "    " << "obj " << o.id << " | " << (int)o.x << " " << (int)o.y << " |" << (int)o.spr_left << " " << (int)o.spr_top << " " << (int)o.spr_w << " " << (int)o.spr_h << "\n";
                ObjectsPool::thinGameObjects[o.id] = o;
            }
        };


    std::for_each(objectsToDraw.cbegin(), objectsToDraw.cend(), objectActor);

    return;
    // 1. draw ice and water

    o.zorder = 1;
    auto objectsToDrawFirst = ObjectsPool::getObjectsByTypes({"ice", "water"});
    std::for_each(objectsToDrawFirst.cbegin(), objectsToDrawFirst.cend(), objectActor);

    o.zorder = 2;
    // 2. draw tanks and bullets
    std::unordered_set<GameObject *> objectsToDrawSecond = ObjectsPool::getObjectsByTypes({"player", "eagle", "npcBaseTank", "npcFastTank", "npcPowerTank", "npcArmorTank", "bullet"});
    std::for_each(objectsToDrawSecond.begin(), objectsToDrawSecond.end(), objectActor);

    o.zorder = 3;
    // 3. draw walls and trees
    auto objectsToDrawThird = ObjectsPool::getObjectsByTypes({
                "brickWall", "brickWall1x1", "brickWall2x1", "brickWall1x2", "brickWall2x2",
                "concreteWall", "tree"});
    std::for_each(objectsToDrawThird.cbegin(), objectsToDrawThird.cend(), objectActor);

    o.zorder = 4;
    // 4. visual effects
    auto objectsToDrawFourth = ObjectsPool::getObjectsByTypes({
        "spawner_player", "spawner_npcBaseTank", "spawner_npcFastTank", "spawner_npcPowerTank", "spawner_npcArmorTank",
        "100xp", "200xp", "300xp", "400xp", "500xp",
        "smallExplosion", "bigExplosion"
        });
    std::for_each(objectsToDrawFourth.cbegin(), objectsToDrawFourth.cend(), objectActor);

    // last step - send everything to client
    // TODO
}

void GameServer::checkStatePostFrame()
{
    if (gameState != PlayingLevel) return;
    if (framesToDie > -1) {
        if (--framesToDie <= 0)
            gameState = GameOver;
    }

    if (framesToWin > -1) {
        if (--framesToWin <= 0) {
            gameState =  (++currentLevel < levelMaps.size()) ? LoadNextLevel : GameOver;
        }
    }

    // as the game just cleared all objects marked for deletion, all the existing enemies must be alive at this point
    if (framesToWin == -1 && framesToDie == -1 && winConditionsMet()) {
        // instant kill all enemies
        auto enemiesAlive = ObjectsPool::getObjectsByTypes({
            "npcBaseTank", "npcFastTank", "npcPowerTank", "npcArmorTank",
            "spawner_npcBaseTank", "spawner_npcFastTank", "spawner_npcPowerTank", "spawner_npcArmorTank"});
        for (auto enemy : enemiesAlive) enemy->markForDeletion();

        framesToWin = globalConst::MaxFramesToWin;
        SoundPlayer::instance().stopAllSounds();
        SoundPlayer::instance().gameOver = true;
        SoundPlayer::instance().playWinJingle();
        HUD::instance().showWin(true);
    }

    if (framesToWin == -1 && framesToDie == -1 && failConditionsMet()) {
        framesToDie = globalConst::MaxFramesToDie;
        SoundPlayer::instance().stopAllSounds();
        SoundPlayer::instance().gameOver = true;
        SoundPlayer::instance().playFailJingle();
        HUD::instance().showFail(true);
    }
}

bool GameServer::winConditionsMet() const
{
    if (_currentLevelProperties.win == Level::KillEmAll) {
        int countEnemiesAlive = ObjectsPool::countObjectsByTypes({
            "npcBaseTank", "npcFastTank", "npcPowerTank", "npcArmorTank",
            "spawner_npcBaseTank", "spawner_npcFastTank", "spawner_npcPowerTank", "spawner_npcArmorTank"});
        if (countEnemiesAlive < 1)
            return true;

    } else if (_currentLevelProperties.win == Level::SurviveTime) {
        const int secondsToWin = _currentLevelProperties.winParam * 60;
        if (globalVars::globalChronometer.getElapsedTime() >= sf::seconds(secondsToWin))
            return true;
    }

    return false;
}

bool GameServer::failConditionsMet() const
{
    int countPlayerObjects = ObjectsPool::countObjectsByTypes({"player", "spawner_player"});
    if (countPlayerObjects < 1 )
        return true;

    if (_currentLevelProperties.fail == Level::LoseBase) {
        if (ObjectsPool::eagleObject == nullptr)
            return true;
    }

    return false;
}


void GameServer::drawConnectionScreen()
{
    using namespace globalConst;

    constexpr int screenCenterX = 640 / 2;
    constexpr int screenCenterY = 480 / 2;

    // draw black rect
    UiUtils::instance().drawRect(sf::IntRect(0, 0, screen_w, screen_h), sf::Color(0, 0, 0));

    // string
    constexpr int titleFontSize = 24;
    UiUtils::instance().drawText( "waiting for a handshake from client", titleFontSize, screenCenterX, screenCenterY - titleFontSize, false, sf::Color::Cyan);

    Utils::window.display();
}


void GameServer::drawStartingScreen()
{
    using namespace globalConst;

    constexpr int screenCenterX = 640 / 2;
    constexpr int screenCenterY = 480 / 2;

    // draw black rect
    UiUtils::instance().drawRect(sf::IntRect(0, 0, screen_w, screen_h), sf::Color(0, 0, 0));

    // string
    constexpr int titleFontSize = 24;
    UiUtils::instance().drawText( "waiting for starting level request", titleFontSize, screenCenterX, screenCenterY - titleFontSize, false, sf::Color::Yellow);

    Utils::window.display();
}

void GameServer::drawPlayingScreen()
{
    using namespace globalConst;

    constexpr int screenCenterX = 640 / 2;
    constexpr int screenCenterY = 480 / 2;

    // draw black rect
    UiUtils::instance().drawRect(sf::IntRect(0, 0, screen_w, screen_h), sf::Color(0, 0, 0));

    // string
    constexpr int titleFontSize = 24;
    UiUtils::instance().drawText( "Running the game... Client should show something now", titleFontSize, screenCenterX, screenCenterY - titleFontSize, false, sf::Color::Green);

    Utils::window.display();
}

} // namespace