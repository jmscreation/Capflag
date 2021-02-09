#ifndef _GAME_OBJECTS_
#define _GAME_OBJECTS_


class MapObj_Wall : public CFGameObject {
public:
    MapObj_Wall(int, int);
    virtual ~MapObj_Wall();
    //void draw(sf::RenderWindow& win, sf::Time&delta);

    inline virtual int type() { return GAMEOBJ_WALL; }
};

class MapObj_Base : public CFGameObject {
public:
    MapObj_Base(int, int, int);
    virtual ~MapObj_Base();
    inline virtual int type() { return GAMEOBJ_BASE; }

    int team;
};

class MapObj_Flag : public CFGameObject {
public:
    MapObj_Flag(int, int, int);
    virtual ~MapObj_Flag();
    inline virtual int type() { return GAMEOBJ_FLAG; }

    inline void returnFlag() { followObject = nullptr; }

    int team;
    bool mplayMoveUpdate;
    void collide(CFGameObject*);

protected:
    float xStart, yStart, dir;
    CFGameObject* followObject;
    sf::Clock pauseFollow;

    virtual void step(sf::Time&);
};

class MapObj_Pickup : public CFGameObject {
public:
    MapObj_Pickup(int, int);
    virtual ~MapObj_Pickup();
    inline virtual int type() { return GAMEOBJ_PICKUP; }

    bool dead;
    int bonusType;
protected:
    float alpha;
    virtual void step(sf::Time&);
};

class MapObj_Bullet : public CFGameObject {
public:
    MapObj_Bullet(int, int, int, float dir, float dmg=5);
    virtual ~MapObj_Bullet();
    inline virtual int type() { return GAMEOBJ_BULLET; }

    void collide(CFGameObject*);

protected:
    float damage, speed;
    unsigned int myGenerator;

    virtual void step(sf::Time&);
};

class MapObj_BulletImpact : public CFGameObject {
public:
    MapObj_BulletImpact(int, int, float, int, int=200);
    virtual ~MapObj_BulletImpact();
    inline virtual int type() { return GAMEOBJ_BULLET_IMPACT; }
protected:
    sf::Clock die;
    int life;

    virtual void step(sf::Time&);
};

class MapObj_Player : public CFGameObject {
public:
    MapObj_Player(int, int, int);
    virtual ~MapObj_Player();
    inline virtual int type() { return GAMEOBJ_PLAYER; }

    unsigned int life, ammo, bonusType;
    bool isAi;
    int team;
    float dir, alpha;
    PathGrid::PathFinder* myPath;

    bool mplayUpdated, mplayFired, mplayRespawn;
    unsigned int mplayID, mplayBonus, mplayBonusReset;

    inline bool isDead() { return !canMove; }
    bool isMe() const;

    virtual void moveLeft();
    virtual void moveRight();
    virtual void moveUp();
    virtual void moveDown();
    virtual void shootGun();
    virtual void clearBonus();
    virtual void setDirection(float dir);
    virtual void activateBonus();
    virtual void hitPlayer(float damage);
    virtual void killMe();
    virtual void respawnMe();
    virtual void resetBonusStatus(int bonus);

    void collide(CFGameObject* other);

protected:
    unsigned int magSize;

    bool bonusActive[BONUS_TYPE_COUNT];
    sf::Clock bonusExpires[BONUS_TYPE_COUNT];

    float speed, moveSpeed;
    bool canMove, invisible;
    sf::Clock canShoot, respawn, reload, updatePath;

    virtual void step(sf::Time& delta);
};

class MapObj_AI : public MapObj_Player , public Engine::Input::Mouse {
public:
    MapObj_AI(int, int, int);
    virtual ~MapObj_AI();
    inline virtual int type() { return GAMEOBJ_AI; }

    virtual void toggleFollowMe(MapObj_Player* me, bool direct=false);

protected:
    static unsigned int AI_ID_Counter;
    MapObj_Player *follow, *fireAt;
    PathGrid::PathFinder* path;
    float xto, yto;

    sf::Clock motionStart, motionStop, motionDir, searchEnemy;

    virtual void step(sf::Time& delta);
    virtual void killMe();
    virtual void mouseButtonPress(sf::Event::MouseButtonEvent &evt);

    friend class MapObj_Player;
    friend class GameController;
};

class MapObj_DeadPlayer : public CFGameObject {
public:
    MapObj_DeadPlayer(int, int, int);
    virtual ~MapObj_DeadPlayer();
    inline virtual int type() { return GAMEOBJ_DEAD_BODY; }

protected:
    sf::Clock fade;
    float alpha;
    Engine::AnimatedSprite border;

    void step(sf::Time&);
    void draw(sf::RenderWindow &win, sf::Time &delta);
};


/*
class MapObj_ : public CFGameObject {
public:
    MapObj_(int, int);
    virtual ~MapObj_();
    inline virtual int type() { return GAMEOBJ_; }
};
*/


#endif // _GAME_OBJECTS_
