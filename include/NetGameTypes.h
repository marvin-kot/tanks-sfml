#pragma once

#include "GlobalConst.h"

#include <SFML/Network.hpp>
#include <SFML/Network/Packet.hpp>

#include <cstdint>


namespace net
{

enum SpecialMessage
{
    NullMessage = 0,
    StartGameRequest = 0xBEEF,
    StartLevelRequest = 0xBAAD,
    PauseRequest = 0x5109,
    PauseResponse = 0x9015,
    GameOverConfirm = 0xDEAD
};

constexpr uint16_t CLIENT_SEND_PORT = 5501;
constexpr uint16_t SERVER_SEND_PORT = 5502;

constexpr uint8_t MapDetails_frameBegin = 9;
constexpr uint8_t FrameDetails_frameBegin = 13;

struct MapDetails
{
    sf::Uint16  id;
    sf::Uint8  w, h;
    char title[64];
    sf::Uint8 brief_lines;
    char briefing[6][128];
    char win[32];
    char fail[32];
};

#define FRAMEPROP_BIT_MIRROR 0
#define FRAMEPROP_BIT_ROTATE 1

struct ThinGameObject
{
    sf::Uint16  id; // 2
    sf::Uint16 x, y; // 4
    sf::Uint8 frame_id; // 5
    sf::Uint8 frape_prop; // 6
    sf::Uint16  flags; // 8
    sf::Uint8 zorder; // 9
    sf::Uint8 dummy;
};

struct UpgradeInfo
{
    bool isBaseUpgrade;
    sf::Uint8 frame_id;
    sf::Uint8 level;
    char description[64];
};

struct PlayerInfo
{
    uint8_t id;
    uint8_t level;
    uint32_t xp;
    uint8_t lives;
    UpgradeInfo upgrades[globalConst::PlayerUpgradesLimit];
};

constexpr int MaxVisibleObjects = globalConst::viewPortWidthTiles * globalConst::viewPortHeightTiles * 4;

struct FrameDetails
{
    sf::Uint16 frame_num;
    PlayerInfo player;
    sf::Uint8 splay_num;
    sf::Uint8 sounds_play[4];
    sf::Uint8 sstop_num;
    sf::Uint8 sounds_stop[2];
    sf::Uint16 num_objects;
    ThinGameObject objects[MaxVisibleObjects];
    sf::Uint8 pause;
    sf::Uint8 upgradeIds[globalConst::NumOfUpgradesOnLevelup];
};

struct PlayerInput
{
    sf::Uint8 direction_request;
    bool shoot_request;
    sf::Uint8 shoot_direction_request;
    bool weapon2_request;
    bool pause_request;
    bool exit_request;
    bool isUpgradeChoice;
    sf::Uint8 upgradeChosen;
};

}
