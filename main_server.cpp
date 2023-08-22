

#include "GameServer.h"
#include "Logger.h"


int main()
{
    using namespace server;
    GameServer game;
    Logger::instance().setFilename("logfile_server.txt");
    Logger::instance() << "Loading assets...";
    if (!game.loadAssets())
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

    return 0;
}
