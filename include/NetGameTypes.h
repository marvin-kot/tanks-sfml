#pragma once

#include "GlobalConst.h"

#include <SFML/Network.hpp>
#include <SFML/Network/Packet.hpp>

#include <cstdint>
//#include <string>


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

struct ThinGameObject
{
    sf::Uint16  id; // 2
    sf::Uint16 x, y; // 4
    sf::Uint16 spr_left, spr_top, spr_w, spr_h;  // 8
    sf::Uint16  flags; // 10
    sf::Uint8 zorder; // 11
    sf::Uint8 dummy;

    //friend std::ostream& operator<< (std::ostream& os, const ThinGameObject& o);

};

/*std::ostream& operator<< (std::ostream& os, const ThinGameObject& o) {
        os << "obj " << o.id << " | " << o.x << " " << "y |" << o.spr_left << " " << o.spr_top << " " << o.spr_w << " " << o.spr_h;
        return os;
    }*/

struct UpgradeInfo
{
    bool isBaseUpgrade;
    sf::Uint8 spr_left, spr_top, spr_w, spr_h;  // 8
    sf::Uint8 level;
    char description[64];
};

struct PlayerInfo
{
    uint8_t id;
    uint8_t level;
    uint32_t xp;
    uint8_t lives;
    UpgradeInfo upgrades[4];
};

//extern sf::Packet& operator <<(sf::Packet& packet, const ThinGameObject& o);
//extern sf::Packet& operator >>(sf::Packet& packet, ThinGameObject& o);

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
};

//extern sf::Packet& operator <<(sf::Packet& packet, const FrameDetails& f);
//extern sf::Packet& operator >>(sf::Packet& packet, FrameDetails& f);

struct PlayerInput
{
    sf::Uint8 direction_request;
    bool shoot_request;
    bool pause_request;
    bool exit_request;
    bool isUpgradeChoice;
    sf::Uint8 upgradeChosen;
};

}