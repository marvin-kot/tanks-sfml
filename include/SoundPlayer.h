#pragma once


#include "Logger.h"


#include <SFML/Audio.hpp>

#include <queue>

class SoundPlayer
{
public:
    enum SoundType
    {
        NoSound,
        TankStand,
        TankMove,
        Shoot,
        bulletHitWall,
        smallExplosion,
        bigExplosion,
        bonusAppear,
        bonusCollect,
        iceSkid,
        pause,
        xpCollect,
        tick,
        win,
        fail,
        debuff
    };
private:
    sf::SoundBuffer tankStandBuffer;
    sf::Sound       tankStandSound;

    sf::SoundBuffer tankMoveBuffer;
    sf::Sound       tankMoveSound;

    sf::SoundBuffer shootBuffer;
    sf::Sound       shootSound;

    sf::SoundBuffer bulletHitWallBuffer;
    sf::Sound       bulletHitWallSound;

    sf::SoundBuffer smallExplosionBuffer;
    sf::Sound       smallExplosionSound;

    sf::SoundBuffer bigExplosionBuffer;
    sf::Sound       bigExplosionSound;

    sf::SoundBuffer bonusAppearBuffer;
    sf::Sound       bonusAppearSound;

    sf::SoundBuffer bonusCollectBuffer;
    sf::Sound       bonusCollectSound;

    sf::SoundBuffer iceSkidBuffer;
    sf::Sound       iceSkidSound;

    sf::SoundBuffer pauseBuffer;
    sf::Sound       pauseSound;

    sf::SoundBuffer xpCollectBuffer;
    sf::Sound       xpCollectSound;

    sf::SoundBuffer tickBuffer;
    sf::Sound       tickSound;

    sf::SoundBuffer winBuffer;
    sf::Sound       winSound;

    sf::SoundBuffer failBuffer;
    sf::Sound       failSound;

    sf::SoundBuffer debuffBuffer;
    sf::Sound       debuffSound;

    std::queue<SoundType> _sounds_to_stop;
    std::queue<SoundType> _sounds_to_play;

public:
    bool gameOver = false;
private:
    SoundPlayer();


    SoundPlayer(SoundPlayer &) = delete;
    void operator=(const SoundPlayer&) = delete;


public:
    static SoundPlayer& instance();

    void playSound(SoundType);
    void stopSound(SoundType);

    int loadSounds();
    void stopAllSounds();
    void playTankStandSound();
    void playTankMoveSound();
    void playShootSound();
    void playBulletHitWallSound();
    void playSmallExplosionSound();
    void playBigExplosionSound();
    void playBonusAppearSound();
    void playBonusCollectSound();
    void playIceSkidSound();
    void playPauseSound();
    void playXpCollectSound();
    void playTickSound();
    void playWinJingle();
    void playFailJingle();
    void playDebuffSound();

    void enqueueSound(SoundType, bool);

    std::queue<SoundType>& getSoundsQueue(bool);

    void processQueuedSounds();
};