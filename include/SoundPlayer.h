#pragma once


#include "Logger.h"


#include <SFML/Audio.hpp>

class SoundPlayer
{
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

    sf::SoundBuffer bonusAppearBuffer;
    sf::Sound       bonusAppearSound;

    sf::SoundBuffer bonusCollectBuffer;
    sf::Sound       bonusCollectSound;

public:
    bool gameOver = false;
private:
    SoundPlayer();


    SoundPlayer(SoundPlayer &) = delete;
    void operator=(const SoundPlayer&) = delete;


public:
    static SoundPlayer& instance();

    int loadSounds();
    void stopAllSounds();
    void playTankStandSound();
    void playTankMoveSound();
    void playShootSound();
    void playBulletHitWallSound();
    void playSmallExplosionSound();
    void playBonusAppearSound();
    void playBonusCollectSound();

};