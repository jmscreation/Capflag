#ifndef __ANIMATIONS__
#define __ANIMATIONS__


struct CFGameAnimations {

    CFGameAnimations();
    virtual ~CFGameAnimations();

    Engine::Animation Wall, WallFlesh, Player, Base, Pickup, Impact, Flag, DeadBody, BulletSpark, Bullet, Background;
};


#endif // __ANIMATIONS__
