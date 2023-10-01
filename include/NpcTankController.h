#include "Controller.h"
#include "GlobalConst.h"

#include <stack>

class NpcTankController : public Controller
{
    bool _blink = false;
protected:
    const sf::Time _actionTimeout;
    bool freezeIfGlobalFreezeActive();
    void blinkIfParalyzed();

    virtual bool decideIfToShoot(globalTypes::Direction oldDir) const = 0;
    virtual bool tryToMove() = 0;
    int trySqueeze();

    bool tryMoveToOneOfDirections(std::vector<globalTypes::Direction>&);

    bool _triggered = false; // needed only for kamikaze tank
public:
    NpcTankController(GameObject *parent, int spd, float timeoutSec);
    void update() override;
    void onDamaged() override;
    GameObject *onDestroyed() override;
};

class TankRandomController : public NpcTankController
{
    bool decideIfToShoot(globalTypes::Direction oldDir) const override;

    bool tryToMove() override;
public:
    TankRandomController(GameObject *parent, int spd, float timeoutSec);
};

class TankKamikazeController : public NpcTankController
{
    bool decideIfToShoot(globalTypes::Direction oldDir) const override;
    bool tryToMove() override;

public:
    TankKamikazeController(GameObject *parent, int spd, float timeoutSec);
    GameObject *onDestroyed() override;
};


class TankBossController : public NpcTankController
{
    bool decideIfToShoot(globalTypes::Direction oldDir) const override;
    bool tryToMove() override;

    //std::stack<globalTypes::Direction> _nextDirection;
    globalTypes::Direction _nextDirection;

public:
    TankBossController(GameObject *parent, int spd, float timeoutSec);
    ~TankBossController();
    void onDamaged() override;
    GameObject *onDestroyed() override;
};