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


private:
    SoundPlayer()
    {
        if (-1 == loadSounds())
            Logger::instance() << "[ERROR] Cannot load sounds";
    }

    SoundPlayer(SoundPlayer &) = delete;
    void operator=(const SoundPlayer&) = delete;

public:
    static SoundPlayer& instance()
    {
        static SoundPlayer _instance;
        return _instance;
    }


    int loadSounds()
    {
        if (!tankStandBuffer.loadFromFile("assets/audio/tankStanding.wav"))
            return -1;
        tankStandSound.setBuffer(tankStandBuffer);

        if (!tankMoveBuffer.loadFromFile("assets/audio/tankMoving.wav"))
            return -1;
        tankMoveSound.setBuffer(tankMoveBuffer);

        if (!shootBuffer.loadFromFile("assets/audio/shot.wav"))
            return -1;
        shootSound.setBuffer(shootBuffer);

        if (!bulletHitWallBuffer.loadFromFile("assets/audio/bulletHitWall.wav"))
            return -1;
        bulletHitWallSound.setBuffer(bulletHitWallBuffer);

        if (!smallExplosionBuffer.loadFromFile("assets/audio/smallExplosion.wav"))
            return -1;
        smallExplosionSound.setBuffer(smallExplosionBuffer);

        return 0;
    }

    void stopAllSounds()
    {
        tankMoveSound.stop();
        tankStandSound.stop();
    }


    void playTankStandSound()
    {
        sf::SoundSource::Status standStatus = tankStandSound.getStatus();
        sf::SoundSource::Status moveStatus = tankMoveSound.getStatus();

        if (moveStatus == sf::SoundSource::Status::Playing)
            tankMoveSound.pause();

        if (standStatus != sf::SoundSource::Status::Playing) {
            tankStandSound.setLoop(true);
            tankStandSound.play();
        }
    }

    void playTankMoveSound()
    {
        sf::SoundSource::Status standStatus = tankStandSound.getStatus();
        sf::SoundSource::Status moveStatus = tankMoveSound.getStatus();

        if (standStatus == sf::SoundSource::Status::Playing)
            tankStandSound.pause();

        if (moveStatus != sf::SoundSource::Status::Playing) {
            tankMoveSound.setLoop(true);
            tankMoveSound.play();
        }
    }

    void playShootSound()
    {
        shootSound.play();
    }
};