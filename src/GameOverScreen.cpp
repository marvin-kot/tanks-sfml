#include "GameOverScreen.h"

#include "MissionSelectScreen.h"
#include "PlayerUpgrade.h"
#include "PersistentGameData.h"
#include "SoundPlayer.h"
#include "UiUtils.h"
#include "Utils.h"

#include <SFML/System/Time.hpp>

#include <cassert>
#include <format>

GameOverScreen& GameOverScreen::instance()
{
    static GameOverScreen _instance;
    return _instance;
}

GameOverScreen::GameOverScreen() : _state(Start) {}


void GameOverScreen::setMissionOutcome(bool won, int index, int ek, int ss)
{
    _won = won;
    _missionIndex = index;
    _enemiesKilled = ek;
    _secondsSurvived = ss;
}

globalTypes::GameState GameOverScreen::draw()
{
    using namespace globalConst;
    using namespace globalVars;
    using namespace globalTypes;

    constexpr int screenCenterX = screen_w / 2;
    constexpr int screenCenterY = screen_h / 2;

    constexpr int screenQuarterY = screen_h / 4;

    const sf::Color greyColor = sf::Color(150, 150, 150);

    // draw frame
    sf::RectangleShape blackRect(sf::Vector2f(screen_w, screen_h));
    blackRect.setOrigin(blackRect.getSize().x/2, blackRect.getSize().y/2);
    blackRect.setPosition(sf::Vector2f(screenCenterX, screenCenterY));
    blackRect.setFillColor(sf::Color(0, 0, 0));
    Utils::window.draw(blackRect);

    const int menuWidth = blackRect.getSize().x;
    const int menuHeight = blackRect.getSize().y;

    int currentStringY = screenCenterY - blackRect.getSize().y/2 - 32;

    // draw title
    const int titleFontSize = 52;
    currentStringY += titleFontSize * 2;

    std::string title = "Mission Outcome";
    UiUtils::instance().drawText( title, titleFontSize, screenCenterX, currentStringY);

    currentStringY += titleFontSize + 32;
    const int lineSize = 34;

    UiUtils::instance().drawHorizontalLine(screenCenterX, currentStringY, globalConst::screen_w-64, greyColor);


    switch (_state)
    {
        case Start:
            _delayClock.restart();
            _state = DelayAfterStart;
            _countKills = _enemiesKilled;
            _countSec = _secondsSurvived;
            break;
        case DelayAfterStart:
            if (_delayClock.getElapsedTime() > sf::seconds(1)) {
                _state = CountSeconds;
                _clock.restart();
            }
            break;
        case CountSeconds: {
                float tickTime = _faster ? 0.02 : 0.06;
                if (_clock.getElapsedTime() > sf::seconds(tickTime)) {
                    if (_countSec == 0) {
                        _state = CountKills;
                        _clock.restart();
                        break;
                    }
                    SoundPlayer::instance().playSound(SoundPlayer::tick);
                    _countSec -= 10; if (_countSec<0) _countSec = 0;
                    _countMoney += 100;
                    _clock.restart();
                }
            }
            break;
        case CountKills:
            {
                float tickTime = _faster ? 0.01 : 0.08;
                if (_clock.getElapsedTime() > sf::seconds(tickTime)) {
                    if (_countKills == 0) {
                        _state = DelayAfterCount;
                        _delayClock.restart();
                        break;
                    }
                    SoundPlayer::instance().playSound(SoundPlayer::tick);
                    _countKills--;
                    _countMoney += 50;
                    _clock.restart();
                }
            }
            break;
        case DelayAfterCount:
            if (_delayClock.getElapsedTime() > sf::seconds(0.5)) {
                _state = AddingToDeposit;
                _delayClock.restart();
            }
        case AddingToDeposit:
            PersistentGameData::instance().addToXpDeposit(_countMoney);
            SoundPlayer::instance().playSound(SoundPlayer::bonusAppear);
            _delayClock.restart();
            _state = DelayAfterAdding;
            break;
        case DelayAfterAdding:
            if (_delayClock.getElapsedTime() > sf::seconds(1)) {
                _state = UnlockShop;
            }
            break;
        case UnlockShop:
            if (PersistentGameData::instance().isShopUnlocked() == false
                && PersistentGameData::instance().xpDeposit() > PlayerUpgrade::minPerkPrice) {
                    // TODO get rid of magic numbers
                    PersistentGameData::instance().unlockShop();
                    SoundPlayer::instance().playSound(SoundPlayer::bonusCollect);
                    _delayClock.restart();
                    _shopJustUnlocked = true;
                }

            if (_delayClock.getElapsedTime() > sf::seconds(_faster ? 1.5 : 3)) {
                _delayClock.restart();
                _state = UnlockNewMission;
            }
            break;
        case UnlockNewMission: {
            const int unlockedMissions = PersistentGameData::instance().unlockedLevels();
            if (_won && !_newMissionJustUnlocked && _missionIndex==unlockedMissions && unlockedMissions+1 < MissionSelectScreen::instance().totalMissions()) {
                PersistentGameData::instance().unlockNewLevel();
                SoundPlayer::instance().playSound(SoundPlayer::bonusCollect);
                _delayClock.restart();
                _newMissionJustUnlocked = true;
            }
            if (_delayClock.getElapsedTime() > sf::seconds(_faster ? 1.5 : 3))
                _state = Exit;
        }
            break;
        case Exit:
            _state = Start; // for the next time
            _shopJustUnlocked = false;
            _newMissionJustUnlocked = false;
            _faster = false;
            if (PersistentGameData::instance().isShopUnlocked())
                return GameState::BonusShop; // BonusShop
            else
                return GameState::SelectLevel; // SelectLevel
    }

    // Draw rest
    currentStringY += titleFontSize*2 + 32;

    if (_state >= CountSeconds) {
        int minutes = _countSec / 60;
        int seconds = _countSec % 60;
        std::string line = std::format("Time survived:      {}:{}", minutes, seconds);
        UiUtils::instance().drawText( line, lineSize, menuWidth/5-64, currentStringY, true, _state==CountSeconds ? sf::Color::White : greyColor) ;
    }

    currentStringY += lineSize*2;
    if (_state >= CountKills) {
        std::string line = std::format("Enemies destroyed:  {}", _countKills);
        UiUtils::instance().drawText( line, lineSize, menuWidth/5-64, currentStringY, true, _state==CountKills ? sf::Color::White : greyColor) ;
    }

    currentStringY += lineSize*2;

    if (_state >= CountSeconds) {
        std::string line = std::format("Bonus earned:       {}", _countMoney);
        UiUtils::instance().drawText( line, lineSize, menuWidth/5-64, currentStringY, true, _state<AddingToDeposit ? sf::Color::White : greyColor) ;
    }

    currentStringY += lineSize*2 + 32;
    UiUtils::instance().drawHorizontalLine(screenCenterX, currentStringY, globalConst::screen_w-64, greyColor);
    currentStringY += lineSize*2 + 32;
    if (_state < AddingToDeposit) {
        std::string line = std::format("Deposit:            {} + {}", PersistentGameData::instance().xpDeposit(), _countMoney);
        UiUtils::instance().drawText( line, lineSize, menuWidth/5-64, currentStringY, true, sf::Color::White) ;
    } else {
        std::string line = std::format("Deposit:            {}", PersistentGameData::instance().xpDeposit());
        UiUtils::instance().drawText( line, lineSize, menuWidth/5-64, currentStringY, true, sf::Color::White);
    }

    currentStringY += lineSize*2 + titleFontSize;

    if (_state >= UnlockShop && _shopJustUnlocked) {
        static bool blink = false;
        if (_state == UnlockShop && _clock.getElapsedTime() > sf::seconds(0.3)) {
            // blink only when appeared
            _clock.restart();
            blink = !blink;
        }

        if (blink || _state > UnlockShop) {

            std::string line = std::format("Perk shop unlocked!");
            UiUtils::instance().drawText( line, titleFontSize, screenCenterX, currentStringY, false, _state > UnlockShop ? greyColor : sf::Color::Green);
        }
    }

    currentStringY += lineSize*2 + titleFontSize;

    if (_state >= UnlockNewMission && _newMissionJustUnlocked) {
        static bool blink = false;
        if (_clock.getElapsedTime() > sf::seconds(0.3)) {
            _clock.restart();
            blink = !blink;
        }
        if (blink) {
            std::string line = std::format("New Mission unlocked!");
            UiUtils::instance().drawText( line, titleFontSize, screenCenterX, currentStringY, false, sf::Color::Green);
        }
    }

    Utils::window.display();
    return GameState::GameOverScreen;
}