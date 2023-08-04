#include "SoundPlayer.h"


SoundPlayer::SoundPlayer()
{
    if (-1 == loadSounds())
        Logger::instance() << "[ERROR] Cannot load sounds";
}


SoundPlayer& SoundPlayer::instance()
{
    static SoundPlayer _instance;
    return _instance;
}


int SoundPlayer::loadSounds()
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

void SoundPlayer::stopAllSounds()
{
    tankMoveSound.stop();
    tankStandSound.stop();
}


void SoundPlayer::playTankStandSound()
{
    sf::SoundSource::Status standStatus = tankStandSound.getStatus();
    sf::SoundSource::Status moveStatus = tankMoveSound.getStatus();

    if (moveStatus == sf::SoundSource::Status::Playing)
        tankMoveSound.pause();

    if (gameOver) {
        tankStandSound.stop();
        return;
    }

    if (standStatus != sf::SoundSource::Status::Playing) {
        tankStandSound.setLoop(true);
        tankStandSound.play();
    }
}

void SoundPlayer::playTankMoveSound()
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


void SoundPlayer::playShootSound()
{
    shootSound.play();
}

void SoundPlayer::playBulletHitWallSound()
{
    bulletHitWallSound.play();
}

void SoundPlayer::playSmallExplosionSound()
{
    smallExplosionSound.play();
}

