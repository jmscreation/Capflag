#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#define GAMEOBJ_WALL            7

#define GAMEOBJ_PLAYER          1
#define GAMEOBJ_AI              13
#define GAMEOBJ_PICKUP          18

#define GAMEOBJ_BASE_A          27
#define GAMEOBJ_BASE_B          29

#define GAMEOBJ_FLAG_A          30
#define GAMEOBJ_FLAG_B          31

/*Updated Object IDs*/
#define GAMEOBJ_BASE            50
#define GAMEOBJ_FLAG            51

#define GAMEOBJ_BULLET          64
#define GAMEOBJ_BULLET_IMPACT   65
#define GAMEOBJ_DEAD_BODY       66

#define BULLET_IMPACT_WALL      0
#define BULLET_IMPACT_HUMAN     1
#define BULLET_IMPACT_SPARK     2


#define BONUS_TYPE_COUNT        6
#define BONUS_NONE              0
/// Bonus Types:
#define BONUS_SPEED             1
#define BONUS_INVISIBLE         2
#define BONUS_RETURNFLAG        3
#define BONUS_WARPBASE          4
#define BONUS_GAINHEALTH        5
#define BONUS_SPAWNAI           6



typedef struct  {
    unsigned int x, y, type, prop;
} GameObjectType;

class CFGameObject : public Engine::GameObject , public Engine::Active , public Engine::ActiveEnd {
    public:
        CFGameObject(int x, int y, int, int=0);
        virtual ~CFGameObject();
        virtual int type() = 0;

        const unsigned int& index;

        static std::vector<CFGameObject*> gameObjects;  //Contains all game objects in the game
        static unsigned int counter;
        static bool isHost;
        static PathGrid* grid;
        static Engine::SoundInstance* playSound(unsigned int snd);
        Engine::SoundInstance* playSoundDist(unsigned int snd);

        virtual void step(sf::Time &delta);
        virtual void stepEnd(sf::Time &delta);

        float direction, xprev, yprev;
        void destroy() { destroyMe = true; }

    protected:
        static Engine::Animation* getAnimation(int, int);
        void InitMaskCircle(float=16);
        void InitMaskRectangle(float=32, float=32);

    private:
        unsigned int _ind;
        bool destroyMe;
};

#endif // GAMEOBJECT_H
