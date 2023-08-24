#include "PersistentGameData.h"
#include "Logger.h"

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

const std::string DefaultSaveFilePath = "playerdata_decoded.json";

PersistentGameData::PersistentGameData() : _xpDeposit(0)
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
        _xpDeposit = data["deposit"];
    }
    catch (json::parse_error& ex)
    {
        Logger::instance() << "file not found\n";
        _xpDeposit = 0;
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

    std::ofstream file(_filePath);
    file << data;
}