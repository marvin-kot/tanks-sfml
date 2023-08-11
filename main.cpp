

#include "Game.h"
#include "Logger.h"


int main()
{

    Game game;
    Logger::instance() << "Loading assets...";
    if (!game.loadAssets())
        return -1;

    Logger::instance() << "Initializing window...";
    if (!game.initializeWindow())
        return -1;

    Logger::instance() << "Starting the Game...";
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
