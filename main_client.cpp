

#include "GameClient.h"
#include "Logger.h"


int main()
{
    using namespace client;
    GameClient game;
    Logger::instance().setFilename("logfile_client.txt");
    Logger::instance() << "Loading assets...";
    if (!game.loadAssets())
        return -1;

    Logger::instance() << "Initializing window...";
    if (!game.initializeWindow())
        return -1;

    Logger::instance() << "Starting the Game...";
    game.initializeVariables();
    // main loop
    while (game.isWindowOpen())
    {
        if (!game.update()) {
            Logger::instance() << "[ERROR] during update";
            return -1;
        }
    }

    Logger::instance() << "Game window is closed";

    return 0;
}
