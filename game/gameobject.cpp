#include "includes.h"

using namespace Engine;
using namespace std;

vector<CFGameObject*> CFGameObject::gameObjects = {};
unsigned int CFGameObject::counter = 0;
bool CFGameObject::isHost = MplayServer::isRunning();
PathGrid* CFGameObject::grid = nullptr;

CFGameObject::CFGameObject(int x, int y, int ty, int xtra): GameObject(getAnimation(ty, xtra), x, y), index(_ind) {
    _ind = counter++;
    gameObjects.push_back(this);

    sprite.setOrigin(16, 16);
    direction = 0;
    destroyMe = false;
}

CFGameObject::~CFGameObject() {
    if(gameObjects.empty()) return;
    auto ind = find(gameObjects.begin(), gameObjects.end(), this);
    if(ind != gameObjects.end()){
        gameObjects.erase(ind);
    } else {
        cout << "Warning: Can't find instance in array" << endl;
    }
}

Animation* CFGameObject::getAnimation(int ty, int xtra){
    switch(ty){
        case GAMEOBJ_BULLET: return &MyGame::gameAnimations->Bullet;
        case GAMEOBJ_BULLET_IMPACT:{
            switch(xtra) {
                case BULLET_IMPACT_HUMAN: return &MyGame::gameAnimations->Impact;
                case BULLET_IMPACT_WALL: return &MyGame::gameAnimations->WallFlesh;
                case BULLET_IMPACT_SPARK: return &MyGame::gameAnimations->BulletSpark;
            }
            throw runtime_error((string("Cannot load animation for non-existent impact type [") + to_string(xtra) + "]").data());
            return nullptr;}
        case GAMEOBJ_WALL: return &MyGame::gameAnimations->Wall;
        case GAMEOBJ_PLAYER: case GAMEOBJ_AI: return &MyGame::gameAnimations->Player;
        case GAMEOBJ_BASE: return &MyGame::gameAnimations->Base;
        case GAMEOBJ_FLAG: return &MyGame::gameAnimations->Flag;
        case GAMEOBJ_PICKUP: return &MyGame::gameAnimations->Pickup;
        case GAMEOBJ_DEAD_BODY: return &MyGame::gameAnimations->DeadBody;
        default:
            throw runtime_error((string("Cannot load animation for non-existent object type [") + to_string(ty) + "]").data());
            return nullptr;
    }
}

SoundInstance* CFGameObject::playSound(unsigned int snd) {
    SoundBuffer* sound = MyGame::audioPack->getSound(snd);
    if(sound != nullptr){
        return sound->play(true);
    }
    return nullptr;
}

SoundInstance* CFGameObject::playSoundDist(unsigned int snd) {
    SoundInstance* ii = playSound(snd);
    if(ii != nullptr){
        sf::Vector2f center = MyGame::current().view().getCenter();
        float xx = x() - center.x, yy = y() - center.y,
              dist = pow(xx*xx + yy*yy, 0.5);
        ii->volume(fmin(1, fmax(0, 1.0 - dist / 350)));
    }
    return ii;
}

void CFGameObject::InitMaskCircle(float rad) {
    if(mask != nullptr) delete mask;
    mask = new Mask(MASK_CIRCLE, x(), y());
    mask->initCircle(rad, rad, rad);
}

void CFGameObject::InitMaskRectangle(float w, float h){
    if(mask != nullptr) delete mask;
    mask = new Mask(MASK_RECT, x(), y());
    mask->initRect(w, h, w/2, h/2);
}

void CFGameObject::step(sf::Time &delta) {
    sprite.setRotation(direction);
    xprev = x();
    yprev = y();

    if(destroyMe){
        delete this;
        return;
    }
}

void CFGameObject::stepEnd(sf::Time &delta) {
}
