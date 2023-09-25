#include "Shootable.h"
#include "SoundPlayer.h"

class SecondShootable : public Shootable
{
public:
    SecondShootable(GameObject *parent);
};


class LandmineSetter : public SecondShootable
{
    int _limit;

protected:
    bool isShootingProhibited() override;
public:
    LandmineSetter(GameObject *parent);
    bool shoot(globalTypes::Direction dir) override;

    SoundPlayer::SoundType usageSound() const;

    void setLimit(int val) { _limit = val; }
};


class TurretSetter : public SecondShootable
{
    int _limit;
    int _prot;

protected:
    bool isShootingProhibited() override;
public:
    TurretSetter(GameObject *parent);
    bool shoot(globalTypes::Direction dir) override;

    SoundPlayer::SoundType usageSound() const;

    void setLimit(int val) { _limit = val; }
    void setProtection(int val) { _prot = val; }
};

class BlockageSetter : public SecondShootable
{
    int _limit;
    int _prot;

protected:
    bool isShootingProhibited() override;
public:
    BlockageSetter(GameObject *parent);
    bool shoot(globalTypes::Direction dir) override;

    SoundPlayer::SoundType usageSound() const;

    void setLimit(int val) { _limit = val; }
    void setProtection(int val) { _prot = val; }
};


class JezekSetter : public SecondShootable
{
    int _limit;
    int _paralyzeTime;

protected:
    bool isShootingProhibited() override;
public:
    JezekSetter(GameObject *parent);
    bool shoot(globalTypes::Direction dir) override;

    void setLimit(int val) { _limit = val; }
    void setParalyzeTime(int val) { _paralyzeTime = val; }
};