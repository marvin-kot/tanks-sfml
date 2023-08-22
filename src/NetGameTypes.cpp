#include "NetGameTypes.h"


using namespace net;

/*sf::Packet& operator <<(sf::Packet& packet, const ThinGameObject& o)
{
    return packet << o.id << o.x << o.y << o.spr_left << o.spr_top << o.spr_w << o.spr_h << o.flags << o.zorder;
}

sf::Packet& operator >>(sf::Packet& packet, ThinGameObject& o)
{
    return packet >> o.id >> o.x >> o.y >> o.spr_left >> o.spr_top >> o.spr_w >> o.spr_h >> o.flags >> o.zorder;
}

sf::Packet& operator <<(sf::Packet& packet, const FrameDetails& f)
{
    packet << f.frame_num << f.sound_play << f.sound_stop << f.num_objects;
    for (int i=0; i<f.num_objects; i++)
        packet << f.objects[i];

    return packet;
}

sf::Packet& operator >>(sf::Packet& packet, FrameDetails& f)
{
    packet >> f.frame_num >> f.sound_play >> f.sound_stop >> f.num_objects;
    for (int i=0; i<f.num_objects; i++)
        packet >> f.objects[i];

    return packet;
}*/