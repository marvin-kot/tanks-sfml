#pragma once

#include <string>

class PersistentGameData
{
private:
    PersistentGameData();
    PersistentGameData(PersistentGameData &) = delete;
    void operator=(const PersistentGameData&) = delete;

    const std::string _defaultSaveFilePath;
    std::string _filePath;

    long int _xpDeposit;
    bool _shopUnlocked;
public:
    static PersistentGameData& instance();

    void setFilePath(std::string val) { _filePath = val; }
    int loadDataFromDisk();
    int saveDataToDisk();

    int xpDeposit() const { return _xpDeposit; }
    void addToXpDeposit(int val) { _xpDeposit += val; }
    void subtractFromDeposit(int val) { _xpDeposit -= val; }

    bool isShopUnlocked() const { return _shopUnlocked; }
    void unlockShop() { _shopUnlocked = true; }
};