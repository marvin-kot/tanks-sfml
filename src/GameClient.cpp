#include "GameClient.h"

#include "AssetManager.h"
#include "GameObject.h"
#include "GlobalConst.h"
#include "HUD.h"
#include "LevelUpPopupMenu.h"
#include "ObjectsPool.h"

#include "Logger.h"
#include "SoundPlayer.h"
#include "Utils.h"
#include "UiUtils.h"
#include "ClientSpriteRenderer.h"

#include <SFML/Graphics.hpp>

#include <format>

#include "PlayerControllerClient.h"


#define MAX_PAYLOAD 50
#define DEFAULT_KEEP_ALIVE 60


//////////SOCKET STUFF///////
//#include <arpa/inet.h>
//#include <sys/socket.h>

//int status, valread, client_fd;
//struct sockaddr_in serv_addr;

namespace client
{

GameClient::GameClient()
{
}


GameClient::~GameClient() {
}


void GameClient::on_connect(int rc)
{}

void GameClient::on_message(const struct mosquitto_message *message)
{

}
void GameClient::on_subscribe(int mid, int qos_count, const int *granted_qos)
{}

bool GameClient::loadAssets()
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

bool GameClient::initializeWindow()
{
    using namespace globalConst;
    Utils::window.create(sf::VideoMode(screen_w, screen_h), "Retro Tank Massacre SFML");
    //window.setVerticalSyncEnabled(true);
    //Utils::window.setFramerateLimit(30);
    //Utils::window.setVerticalSyncEnabled(true);
    return true;
}

void GameClient::initializeVariables()
{
    currentLevel = 0;
    if (recvSocket.bind(net::SERVER_SEND_PORT) != sf::Socket::Done) {
        Logger::instance() << "Cannot bing to port\n";
    }
    sendSocket.setBlocking(false);
    recvSocket.setBlocking(false);
    gameState = TitleScreen;

}

bool GameClient::update()
{
    memset(&_lastPlayerInput, 0, sizeof(net::PlayerInput));

    updateFrameClock();

    // step 1 read inputs
    processWindowEvents();
    int stateResult = processStateChange();
    if (stateResult == -1) {
        return false;
    } else if (stateResult == 0) {
        return true;
    }
    // else if 1 - proceed

    updateAllObjectControllers();

    // step 2: send inputs
    sendSocket.send((void *)&_lastPlayerInput, sizeof(net::PlayerInput), "127.0.0.1", net::CLIENT_SEND_PORT);

    // step 3: read world state
    uint16_t port;
    uint16_t msg;
    sf::IpAddress sender;
    char buf[sizeof(net::FrameDetails)];
    int bytesRead = 0;
    sf::Socket::Status status = sf::Socket::NotReady;
    std::size_t received = 0;
    int attempts = 100;
    while (status != sf::Socket::Done && --attempts > 0) {
        status = recvSocket.receive((void *)(buf+received), sizeof(net::FrameDetails)-received, received, sender, port);
        bytesRead += received;
        sf::sleep(sf::milliseconds(1));
    }

    if (attempts = 0) {
        Logger::instance() << "[NETWORK ERROR] update loop\n";
        return false;
    }

    if (bytesRead < sizeof(net::FrameDetails))
    {
        Logger::instance() << "WTF too little data\n";
        //updateDisplay();
        return true;
    }

    memcpy((void *)&_lastFrame, buf, sizeof(net::FrameDetails));

    if (_prevFrameNum > _lastFrame.frame_num) {
        // ignore frame
        //updateDisplay();
        return true;
    }


    for (int i = 0; i < _lastFrame.sstop_num; i++) {
        int sid = _lastFrame.sounds_stop[i];
        SoundPlayer::SoundType sound = static_cast<SoundPlayer::SoundType>(sid);
        SoundPlayer::instance().enqueueSound(sound, false);
    }

    for (int i = 0; i < _lastFrame.splay_num; i++) {
        int sid = _lastFrame.sounds_play[i];
        SoundPlayer::SoundType sound = static_cast<SoundPlayer::SoundType>(sid);
        SoundPlayer::instance().enqueueSound(sound, true);
    }

    _prevFrameNum = _lastFrame.frame_num;

    // step 4: play sounds if needed
    SoundPlayer::instance().processQueuedSounds();

    // step 5: draw all objects
    drawGameScreen();
    ClientSpriteRenderer::drawAllObjects(_lastFrame.objects, _lastFrame.num_objects);


    // TODO: read upgrades info
    //HUD::instance().draw();

    if (LevelUpPopupMenu::instance().isOpen()) {
        SoundPlayer::instance().stopAllSounds();
        LevelUpPopupMenu::instance().draw();
    }

    updateDisplay();

    //if (!globalVars::gameIsPaused)
    //checkStatePostFrame();

    return true;
}

bool GameClient::isWindowOpen() const
{
    return Utils::window.isOpen();
}

void GameClient::updateFrameClock()
{
    Utils::currentFrame++;
    Utils::lastFrameTime = Utils::refreshClock.getElapsedTime();
    Utils::refreshClock.restart();
}

void GameClient::processWindowEvents()
{
    sf::Event event;
    while (Utils::window.pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                Utils::window.close();
                break;
            case sf::Event::KeyPressed:
                if (gameState == TitleScreen) {
                    if (event.key.scancode == sf::Keyboard::Scan::Escape)
                        Utils::window.close();
                    else if (event.key.scancode == sf::Keyboard::Scan::Space) {
                        currentLevel = 0;
                        gameState = ConnectToHost;
                    }
                } else if (gameState == StartLevelScreen) {
                    if (event.key.scancode == sf::Keyboard::Scan::Space)
                        gameState = StartLevel;
                    else if (event.key.scancode == sf::Keyboard::Scan::Escape) {
                        _lastPlayerInput.exit_request = true;
                        //gameState = GameOver;
                    }
                } else if (gameState == PlayingLevel) {
                    if (LevelUpPopupMenu::instance().isOpen()) {
                        if (event.key.scancode == sf::Keyboard::Scan::Enter) {
                            _lastPlayerInput.isUpgradeChoice = true;
                            _lastPlayerInput.upgradeChosen = LevelUpPopupMenu::instance().cursorPosition();
                            //LevelUpPopupMenu::instance().getSelectedUpgrade();
                            LevelUpPopupMenu::instance().close();
                        } else if (event.key.scancode == sf::Keyboard::Scan::Left)
                            LevelUpPopupMenu::instance().moveCursorLeft();
                        if (event.key.scancode == sf::Keyboard::Scan::Right)
                            LevelUpPopupMenu::instance().moveCursorRight();
                        break; // do not process further!
                    }

                    if (event.key.scancode == sf::Keyboard::Scan::Escape) {
                        // TODO: ask player confirmation
                        gameState = GameOver;
                    } else if (event.key.scancode == sf::Keyboard::Scan::Pause || event.key.scancode == sf::Keyboard::Scan::P) {
                        _lastPlayerInput.pause_request = true;
                    }
                }
                break;
            case sf::Event::Resized:
                std::cout << "new width: " << event.size.width << std::endl;
                std::cout << "new height: " << event.size.height << std::endl;
                break;
            default:
                break;
        }
    }
}

void GameClient::pause(bool p)
{
    using namespace globalVars;
    gameIsPaused = p;
    if (p) {
        SoundPlayer::instance().stopAllSounds();
        SoundPlayer::instance().playSound(SoundPlayer::SoundType::pause);
        globalChronometer.pause();
        globalFreezeChronometer.pause();
    } else {
        globalChronometer.resume();
        globalFreezeChronometer.resume();
    }
}

int GameClient::readMapDetails()
{
    return 0;
}

void GameClient::setMapVariables()
{
    globalVars::borderWidth = (globalConst::screen_w - globalVars::mapViewPort.width)/2;
    globalVars::borderHeight = (globalConst::screen_h - globalVars::mapViewPort.height)/2;
    globalVars::gameViewPort = sf::IntRect(globalVars::borderWidth, globalVars::borderHeight, globalVars::mapViewPort.width, globalVars::mapViewPort.height);

    // WARNING: side effect - save map size globally
    globalVars::mapSize = sf::Vector2i(_map.w * globalConst::spriteOriginalSizeX, _map.h * globalConst::spriteOriginalSizeX);
}

int GameClient::processStateChange()
{
    if (gameState == TitleScreen) {
        drawTitleScreen();
        return 0;
    } else if (gameState == ConnectToHost) {
        SoundPlayer::instance().stopAllSounds();
        ObjectsPool::clearEverything();

        // send game start request
        ipAddress = "127.0.0.1";
        uint16_t msg = net::SpecialMessage::StartGameRequest;
        sendSocket.send((void *)&msg, sizeof(msg), ipAddress, net::CLIENT_SEND_PORT);
        gameState = WaitForLevelData;
        return 0;
    } else if (gameState == WaitForLevelData) {
        sf::IpAddress sender;
        // wait for server response

        std::size_t received = 0;
        uint16_t port;

        auto status = recvSocket.receive((void *)&_map, sizeof(net::MapDetails), received, sender, port);
        if ( status == sf::Socket::Done) {
            Logger::instance() << "Level reaad OK\n";
        } else if (status == sf::Socket::Partial) {
            Logger::instance() << "UDP Partial\n";
        }

        if (received == sizeof(net::MapDetails)) {
            setMapVariables();
            gameState = StartLevelScreen;
        }

        drawConnectionScreen();

        return 0;
    }

    else if (gameState == StartLevelScreen) {
        drawStartLevelScreen();
        return 0;
    }
    else if (gameState == StartLevel) {
        SoundPlayer::instance().gameOver = false;
        HUD::instance().showFail(false);
        HUD::instance().showWin(false);

         // send game start request
        ipAddress = "127.0.0.1";
        uint16_t msg = net::SpecialMessage::StartLevelRequest;
        sendSocket.send((void *)&msg, sizeof(msg), ipAddress, net::CLIENT_SEND_PORT);
        gameState = PlayingLevel;
    }
    else if (gameState == GameOver) {
        SoundPlayer::instance().stopAllSounds();
        ObjectsPool::clearEverything();
        gameState = TitleScreen;
        return 0;
    }

    return 1;
}

void GameClient::updateAllObjectControllers()
{
    if (gameState == PlayingLevel)
        PlayerControllerClient::instance().update(&_lastPlayerInput);
}

void GameClient::drawGameScreen()
{
    Utils::window.clear();
    // draw objects (order matter)
    // draw border
    using namespace globalConst;
    sf::RectangleShape greyRect(sf::Vector2f(screen_w, screen_h));
    greyRect.setFillColor(sf::Color(102, 102, 102));
    Utils::window.draw(greyRect);

    // draw view port

    int w = std::min<int>(globalVars::mapSize.x*globalConst::spriteDisplaySizeX, globalVars::gameViewPort.width);
    int h = std::min<int>(globalVars::mapSize.y*globalConst::spriteDisplaySizeY, globalVars::gameViewPort.height);
    sf::RectangleShape blackRect(sf::Vector2f(w, h));
    blackRect.setPosition(sf::Vector2f(globalVars::gameViewPort.left, globalVars::gameViewPort.top));
    blackRect.setFillColor(sf::Color(50, 0, 0));
    Utils::window.draw(blackRect);
}

void GameClient::drawObjects()
{
    // 1. draw ice and water
    auto objectsToDrawFirst = ObjectsPool::getObjectsByDrawOrder(1);
    std::for_each(objectsToDrawFirst.cbegin(), objectsToDrawFirst.cend(), [](GameObject *obj) { obj->draw(); });

    // 2. draw tanks and bullets
    std::unordered_set<GameObject *> objectsToDrawSecond = ObjectsPool::getObjectsByDrawOrder(2);

    std::for_each(objectsToDrawSecond.begin(), objectsToDrawSecond.end(), [&](GameObject *obj) { if (obj) obj->draw(); });

    // 3. draw walls and trees
    auto objectsToDrawThird = ObjectsPool::getObjectsByDrawOrder(3);
    std::for_each(objectsToDrawThird.cbegin(), objectsToDrawThird.cend(), [](GameObject *obj) { obj->draw(); });

    // 4. visual effects
    auto objectsToDrawFourth = ObjectsPool::getObjectsByDrawOrder(4);

    std::for_each(objectsToDrawFourth.cbegin(), objectsToDrawFourth.cend(), [&](GameObject *obj) { obj->draw(); });
}

void GameClient::updateDisplay()
{
    Utils::window.display();
}

void GameClient::checkStatePostFrame()
{
    if (framesToDie > -1) {
        if (--framesToDie <= 0)
            gameState = GameOver;
    }

    if (framesToWin > -1) {
        if (--framesToWin <= 0) {
            gameState = GameOver;
        }
    }
}

bool GameClient::winConditionsMet() const
{
    // received from server

    return false;
}

bool GameClient::failConditionsMet() const
{
    // received from server

    return false;
}

void GameClient::drawTitleScreen()
{
    using namespace globalConst;

    constexpr int screenCenterX = screen_w / 2;
    constexpr int screenCenterY = screen_h / 2;

    // draw black rect
    UiUtils::instance().drawRect(sf::IntRect(0, 0, screen_w, screen_h), sf::Color(0, 0, 0));

    // game title
    constexpr int titleFontSize = 96;
    UiUtils::instance().drawText( "RETRO TANK MASSACRE", titleFontSize, screenCenterX, screenCenterY - titleFontSize);

    // game version
    constexpr int versionFontSize = titleFontSize / 6;
    static std::string version = std::format("version {}.{}.{}", GameMajorVersion, GameMinorVersion, GameReleaseVersion);
    UiUtils::instance().drawText(version, versionFontSize, screenCenterX, screenCenterY + versionFontSize);

    // prompt
    constexpr int promptFontSize = titleFontSize / 4;
    UiUtils::instance().drawText( "Press [space] to start the game", promptFontSize,
        screenCenterX, screenCenterY + versionFontSize + titleFontSize + promptFontSize,
        false, sf::Color::Yellow );


    Utils::window.display();
}

void GameClient::drawConnectionScreen()
{
    using namespace globalConst;

    constexpr int screenCenterX = screen_w / 2;
    constexpr int screenCenterY = screen_h / 2;

    // draw black rect
    UiUtils::instance().drawRect(sf::IntRect(0, 0, screen_w, screen_h), sf::Color(0, 0, 0));

    // string
    constexpr int titleFontSize = 48;
    UiUtils::instance().drawText( "connecting to host ...", titleFontSize, screenCenterX, screenCenterY - titleFontSize);

    Utils::window.display();
}

void GameClient::drawStartLevelScreen()
{
    using namespace globalConst;
    constexpr int screenCenterX = screen_w / 2;
    constexpr int screenCenterY = screen_h / 2;

    constexpr int screenQuarterY = screen_h / 4;

    // grey background
    UiUtils::instance().drawRect(sf::IntRect(0, 0, screen_w, screen_h), sf::Color(102, 102, 102));

    int currentStringY = screenQuarterY - 64;
    // level name
    UiUtils::instance().drawText(
        std::string(_map.title), 48,
        screenCenterX, currentStringY, false,
        sf::Color::White);

    currentStringY += 80;

    for (auto brief : _map.briefing) {
        UiUtils::instance().drawText( brief, 23, screenCenterX, currentStringY, false, sf::Color::White);
        currentStringY += 25;
    }

    // level goal
    currentStringY += 75;
    UiUtils::instance().drawText(
        "Win", 32,
        screenCenterX, currentStringY, false,
        sf::Color::Green);

    currentStringY += 32;

    UiUtils::instance().drawText(
        std::string(_map.win), 24, screenCenterX, currentStringY, false, sf::Color::White);

    // fail condition
    currentStringY += 60;
    UiUtils::instance().drawText(
        "Fail", 32,
        screenCenterX, currentStringY, false,
        sf::Color::Red);

    currentStringY += 32;
    UiUtils::instance().drawText(
        std::string(_map.fail), 24, screenCenterX, currentStringY, false, sf::Color::White);

    // prompt
    currentStringY += 150;
    UiUtils::instance().drawText(
        "Press [space] to start", 24,
        screenCenterX, currentStringY, false,
        sf::Color::Yellow);
    Utils::window.display();
}

void GameClient::drawGameOverScreen()
{
    using namespace globalConst;
    // grey background
    UiUtils::instance().drawRect(sf::IntRect(0, 0, screen_w, screen_h), sf::Color(102, 102, 102));

    UiUtils::instance().drawText(
        "GAME OVER" , 48,
        screen_w/2, screen_h/2 - 64, false,
        sf::Color::White);

    Utils::window.display();
}

}