#pragma once

#include "MapCreator.h"
#include "NetGameTypes.h"


#include <string>
#include <stack>


namespace client
{

class GameClient// : public mosqpp::mosquittopp
{
private:
    enum GameState
    {
        TitleScreen = 0,
        ConnectToHost,
        WaitForLevelData,
        StartLevelScreen,
        StartLevel,
        PlayingLevel,
        GameOver
    };

    GameState gameState = TitleScreen;
    int currentLevel;
    Level::Properties _currentLevelProperties;
    int _surviveTimeoutSec;

    int framesToDie, framesToWin;

    net::MapDetails _map;
    std::stack<int> _objectIdsToUpdateThisFrame;
    std::unordered_map<int, net::ThinGameObject> _objects;

    net::PlayerInput _lastPlayerInput;
    net::FrameDetails _lastFrame;
    int _prevFrameNum = 0;

    sf::UdpSocket sendSocket;
    sf::UdpSocket recvSocket;
    std::string ipAddress;

public:
    // MQTT functions
    void on_connect(int rc);
    void on_message(const struct mosquitto_message *message);
    void on_subscribe(int mid, int qos_count, const int *granted_qos);

public:
    GameClient();
    virtual ~GameClient();

    int readMapDetails();

    bool loadAssets();
    bool initializeWindow();
    void initializeVariables();
    bool update();

    bool isWindowOpen() const;

private:
    void updateFrameClock();
    void processWindowEvents();
    int processStateChange();
    void updateAllObjectControllers();

    void drawGameScreen();
    void recalculateViewPort();
    void drawObjects();
    void updateDisplay();
    void checkStatePostFrame();
    void pause(bool);

    void drawTitleScreen();
    void drawConnectionScreen();
    void drawStartLevelScreen();
    void drawGameOverScreen();

    bool winConditionsMet() const;
    bool failConditionsMet() const;

    void setMapVariables();
};

}