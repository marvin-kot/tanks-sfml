#include "PersistentGameData.h"
#include "Logger.h"

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

const std::string DefaultSaveFilePath = "playerdata_decoded.json";

PersistentGameData::PersistentGameData() : _xpDeposit(0), _filePath(DefaultSaveFilePath)
{
}

PersistentGameData& PersistentGameData::instance()
{
    static PersistentGameData _instance;
    return _instance;
}

int PersistentGameData::loadDataFromDisk()
{
    std::ifstream f(_filePath);


    try {
        json data = json::parse(f);
        if (data.contains("deposit"))
            _xpDeposit = data["deposit"];
        else
            _xpDeposit = 0;

        if (data.contains("shopUnlocked"))
            _shopUnlocked = data["shopUnlocked"];
        else
            _shopUnlocked = false;
    }
    catch (json::parse_error& ex) {
        Logger::instance() << "file not found\n";
        _xpDeposit = 0;
        _shopUnlocked = false;
    } catch (...) {
        Logger::instance() << "unknown exception\n";
        _xpDeposit = 0;
        _shopUnlocked = false;
    }


    /*for (json::iterator it = data.begin(); it != data.end(); ++it) {
        json mmap = *it;

    }*/

    return 0;
}

int PersistentGameData::saveDataToDisk()
{
    json data;

    data["deposit"] = _xpDeposit;
    data["shopUnlocked"] = _shopUnlocked;
    Logger::instance() << "save to file: " << _filePath;
    std::ofstream file(_filePath);
    file << data;
    file.close();

    return 0;
}