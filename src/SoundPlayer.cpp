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

    if (!bigExplosionBuffer.loadFromFile("assets/audio/bigExplosion.wav"))
        return -1;
    bigExplosionSound.setBuffer(bigExplosionBuffer);

    if (!bonusAppearBuffer.loadFromFile("assets/audio/bonusAppear.wav"))
        return -1;
    bonusAppearSound.setBuffer(bonusAppearBuffer);

    if (!bonusCollectBuffer.loadFromFile("assets/audio/bonus.wav"))
        return -1;
    bonusCollectSound.setBuffer(bonusCollectBuffer);

    if (!iceSkidBuffer.loadFromFile("assets/audio/ice.wav"))
        return -1;
    iceSkidSound.setBuffer(iceSkidBuffer);

    if (!pauseBuffer.loadFromFile("assets/audio/pause.wav"))
        return -1;
    pauseSound.setBuffer(pauseBuffer);

    if (!xpCollectBuffer.loadFromFile("assets/audio/xpCollect.wav"))
        return -1;
    xpCollectSound.setBuffer(xpCollectBuffer);

    if (!tickBuffer.loadFromFile("assets/audio/tick.wav"))
        return -1;
    tickSound.setBuffer(tickBuffer);

    if (!winBuffer.loadFromFile("assets/audio/8_Bit_Dendy_Battle_City.wav"))
        return -1;
    winSound.setBuffer(winBuffer);

    if (!failBuffer.loadFromFile("assets/audio/game_over.wav"))
        return -1;
    failSound.setBuffer(failBuffer);

    if (!debuffBuffer.loadFromFile("assets/audio/debuff.wav"))
        return -1;
    debuffSound.setBuffer(debuffBuffer);

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

void SoundPlayer::playBigExplosionSound()
{
    bigExplosionSound.play();
}


void SoundPlayer::playBonusAppearSound()
{
    bonusAppearSound.play();
}

void SoundPlayer::playBonusCollectSound()
{
    bonusCollectSound.play();
}

void SoundPlayer::playIceSkidSound()
{
    sf::SoundSource::Status skidStatus = iceSkidSound.getStatus();
    if (skidStatus != sf::SoundSource::Status::Playing)
        iceSkidSound.play();
}

void SoundPlayer::playPauseSound()
{
    pauseSound.play();
}

void SoundPlayer::playXpCollectSound()
{
    xpCollectSound.play();
}

void SoundPlayer::playTickSound()
{
    tickSound.play();
}

void SoundPlayer::playWinJingle()
{
    winSound.play();
}

void SoundPlayer::playFailJingle()
{
    failSound.play();
}

void SoundPlayer::playDebuffSound()
{
    debuffSound.play();
}