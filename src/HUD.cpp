#include "EagleController.h"
#include "Damageable.h"
#include "GameObject.h"
#include "GlobalConst.h"
#include "HUD.h"
#include "ObjectsPool.h"
#include "PlayerUpgrade.h"
#include "PlayerController.h"
#include "Shootable.h"
#include "SoundPlayer.h"

#include "Utils.h"
#include "UiUtils.h"

bool base_blink = false;

HUD::HUD()
{
    _baseDamageClock.reset(false);
}

HUD& HUD::instance()
{
    static HUD _instance;
    return _instance;
}

void HUD::draw()
{
    checkForGamePause();
    //if (_pause) return;
    drawPlayerXP(28);
    drawTankLives(64);
    drawTankUpgrades(112);
    drawBaseLives(112+64);
    drawBaseUpgrades(112+64+48);
    drawPerks(28);

    drawBullets(globalConst::screen_h - 96);
    drawArmor(globalConst::screen_h - 230);

    drawGlobalTimer();

    if (_showWin)
        drawWinScreen();

    if (_showFail)
        drawFailScreen();

    if (_baseDamageClock.isRunning()) {
        if (_baseDamageClock.getElapsedTime() < sf::seconds(4)) {
            if (_blinkClock.getElapsedTime() > sf::seconds(0.5)) {
                    _blinkClock.reset(true);
                    base_blink = !base_blink;

                    if (base_blink)
                        SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::Alarm, true);
            }
            if (base_blink) {
                UiUtils::instance().drawText("base damaged!", 50, globalConst::screen_w/2, 72, false, sf::Color::Red);
            }
        } else {
            _baseDamageClock.reset(false);
            base_blink = false;
        }
    }
}

void HUD::drawGlobalTimer()
{
    sf::Time time = globalVars::globalChronometer.getElapsedTime();
    int minutes = (int)time.asSeconds() / 60;
    int seconds = (int)time.asSeconds() % 60;

    std::string timeStr = std::to_string(minutes) + ":" + std::to_string(seconds);
    UiUtils::instance().drawText(timeStr, 30, globalConst::screen_w/2, 32);
}

void HUD::drawPlayerXP(int baseY)
{
    const int fontSize = 28;
    std::string levelAndXp = std::to_string(globalVars::player1Level) + "/" + std::to_string(globalVars::player1XP);
    UiUtils::instance().drawText(levelAndXp, fontSize, 8, baseY, true);
}

void HUD::drawTankLives(int baseY)
{
    constexpr int iconWidth = 11;
    constexpr int iconHeight = 14;
    sf::IntRect rect = sf::IntRect(163, 17, iconWidth, iconHeight);

    int baseX = 17;

    for (int i=0; i < globalVars::player1Lives; i++) {
        UiUtils::instance().drawMiniIcon( rect, baseX + i * (iconWidth + 18), baseY );
    }
}

void HUD::drawTankUpgrades(int baseY)
{
    if (ObjectsPool::playerObject == nullptr)
        return;

    auto pController = ObjectsPool::playerObject->getComponent<PlayerController>();

    const int upgradesNumber = pController->numberOfUpgrades();

    int baseX = 32;

    for (int i=0; i<upgradesNumber; i++) {
        UiUtils::instance().drawMiniIcon(
            pController->getUpgrade(i)->iconRect(),
            baseX + i * (32 + 16),
            baseY
        );

        UiUtils::instance().drawText(
            std::to_string(pController->getUpgrade(i)->currentLevel()+1),
            24,
            baseX + i * (32 + 16),
            baseY + 24
        );
    }
}

void HUD::drawBaseLives(int baseY)
{
    sf::IntRect eagleRect = sf::IntRect(176, 17, 16, 14);
    UiUtils::instance().drawMiniIcon(eagleRect, 20, baseY);
}

void HUD::drawBaseUpgrades(int baseY)
{

    if (ObjectsPool::eagleObject == nullptr)
        return;

    auto bController = ObjectsPool::eagleObject->getComponent<EagleController>();

    const int upgradesNumber = bController->numberOfUpgrades();

    int baseX = 32;

    for (int i=0; i<upgradesNumber; i++) {
        UiUtils::instance().drawMiniIcon(
            bController->getUpgrade(i)->iconRect(),
            baseX + i * (32 + 16),
            baseY
        );

        UiUtils::instance().drawText(
            std::to_string(bController->getUpgrade(i)->currentLevel()+1),
            24,
            baseX + i * (32 + 16),
            baseY + 24
        );
    }
}

void HUD::drawPerks(int baseY)
{
    int baseX = globalConst::screen_w - 64;

    for (auto obj : PlayerUpgrade::availablePerkObjects) {
        if (PlayerUpgrade::playerOwnedPerks.contains(obj->type())) {
            UiUtils::instance().drawMiniIcon( obj->iconRect(), baseX, baseY );
            baseX -= 64;
        }
    }
}

void HUD::drawBullets(int baseY)
{
    int baseX = 48;

    if (ObjectsPool::playerObject == nullptr) return;

    auto shootable = ObjectsPool::playerObject->getComponent<Shootable>();
    assert(shootable != nullptr);

    for (int i=0; i<shootable->maxBullets(); i++) {

        auto anim = shootable->bullets() <= i ? "empty" : "default";
        auto rect = AssetManager::instance().getAnimationFrame("bulletIcon", anim, 0).rect;
        UiUtils::instance().drawIcon(rect, baseX-16, baseY);
        baseX += 40;
    }

    for (int j=0; j<shootable->tempBullets(); j++) {
        auto rect = AssetManager::instance().getAnimationFrame("bulletIcon", "extra", 0).rect;
        UiUtils::instance().drawIcon(rect, baseX-16, baseY);
        baseX += 40;
    }
}

void HUD::drawArmor(int baseY)
{
    int baseX = 56;

    if (ObjectsPool::playerObject == nullptr) return;

    auto shootable = ObjectsPool::playerObject->getComponent<Shootable>();
    assert(shootable != nullptr);

    auto damageable = ObjectsPool::playerObject->getComponent<Damageable>();

    for (int i=0; i<damageable->maxDefence(); i++) {
        auto anim = damageable->defence() <= i ? "broken" : "default";
        auto rect = AssetManager::instance().getAnimationFrame("shieldIcon", anim, 0).rect;
        UiUtils::instance().drawIcon(rect, baseX, baseY);
        baseX += 92;
    }
}

void HUD::drawWinScreen()
{
    using namespace globalConst;
    UiUtils::instance().drawText(
        "CONGRATULATIONS!" , 90,
        screen_w/2, screen_h/2 - 90, false,
        sf::Color::Green);

    UiUtils::instance().drawText(
        "Good job, commander!" , 50,
        screen_w/2, screen_h/2 + 50, false,
        sf::Color::White);
}

void HUD::drawFailScreen()
{
    using namespace globalConst;
    UiUtils::instance().drawText(
        "GAME OVER" , 90,
        screen_w/2, screen_h/2 - 90, false,
        sf::Color::Red);

    UiUtils::instance().drawText(
        "You survived for " + _surviveTimeStr , 50,
        screen_w/2, screen_h/2 + 50, false,
        sf::Color::White);
}

void HUD::showWin(bool val)
{
    _showWin = val;
}
void HUD::showFail(bool val)
{
    _showFail = val;

    if (_showFail) {
        sf::Time time = globalVars::globalChronometer.getElapsedTime();
        int minutes = (int)time.asSeconds() / 60;
        int seconds = (int)time.asSeconds() % 60;

        _surviveTimeStr = std::to_string(minutes) + " minutes " + std::to_string(seconds) + " seconds";
    }
}


void HUD::checkForGamePause()
{
    if (!_pause && globalVars::gameIsPaused) {
        if (_baseDamageClock.isRunning()) {
            _blinkClock.pause();
            _baseDamageClock.pause();
        }
        _pause = true;
    } else if (_pause && ! globalVars::gameIsPaused) {
        if (_baseDamageClock.isPaused()) {
            _blinkClock.resume();
            _baseDamageClock.resume();
        }
        _pause = false;
    }
}

void HUD::onBaseDamaged()
{
    base_blink = true;
    _baseDamageClock.reset(true);
    _blinkClock.reset(true);
    SoundPlayer::instance().enqueueSound(SoundPlayer::SoundType::Alarm, true);
}