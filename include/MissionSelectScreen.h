#pragma once

#include <string>
#include <vector>


class MissionSelectScreen
{
private:

    int _cursorPos = 0;

    MissionSelectScreen(MissionSelectScreen&) = delete;
    void operator=(const MissionSelectScreen&) = delete;

    MissionSelectScreen();
    ~MissionSelectScreen();

    struct MapInfo
    {
        std::string filename;
        std::string title;
        std::string briefDescription;
    };

    std::vector<MapInfo> _missions;
    int maxUnlockedMissions = 0;
public:
    static MissionSelectScreen& instance();

    void open();
    void close();


    int draw();

    void drawCursor();
    void moveCursorUp();
    void moveCursorDown();
    void selectLevel();
    int cursorPosition();

    std::string getSelectedFilename() const { return _missions[_cursorPos].filename; }
    int getSelectedIndex() const {return _cursorPos; }

    int totalMissions() const { return _missions.size(); }
};