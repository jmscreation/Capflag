#include "includes.h"

using namespace std;
using namespace Engine;

MapObj_Wall::MapObj_Wall(int x, int y) : CFGameObject(x, y, type()) {
    InitMaskRectangle();
    depth(0.3);
}
MapObj_Wall::~MapObj_Wall() {
}


MapObj_Base::MapObj_Base(int x, int y, int t) : CFGameObject(x, y, type()) {
    InitMaskCircle();
    depth(0.2);
    sprite.frame(t);
    team = t;

}
MapObj_Base::~MapObj_Base() {
}


MapObj_Flag::MapObj_Flag(int x, int y, int t) : CFGameObject(x, y, type()) {
    InitMaskCircle();
    depth(0.4);
    sprite.frame(t);
    sprite.frameDelay(20);
    team = t;
    mplayMoveUpdate = false;
    followObject = NULL;
    xStart = x;
    yStart = y;
}
MapObj_Flag::~MapObj_Flag() {}
void MapObj_Flag::step(sf::Time &delta){

    if(GameController::current().isHost) {
        float vx, vy, tx, ty, tdir, dist;
        if(followObject != nullptr){
            tx = followObject->x();
            ty = followObject->y();
            if(followObject->type() == GAMEOBJ_PLAYER && ((MapObj_Player*)followObject)->isDead()) followObject = nullptr;
        } else {
            tx = xStart;
            ty = yStart;
        }
        dist = sqrt((tx - x())*(tx - x()) + (ty - y())*(ty - y()));

        if(abs(x() - tx) > 16 || abs(y() - ty) > 16){
            tdir = MyGame::point_direction(x(), y(), tx, ty);
            if(abs(dir - tdir) > 3) dir += MyGame::direction_sign_toward(dir, tdir) * 3;

            MyGame::lengthdir(min(5.f, dist / 50), dir, vx, vy);
            move(vx, vy);
            mplayMoveUpdate = true;
        }

        beginCollision();
        while(BaseCollidable* o = iterateCollidable()){
            if(o->mask == nullptr || !mask->collidesWith(*o->mask)) continue;
            collide((CFGameObject*)o);
        }
    }
    CFGameObject::step(delta);  //inherit
}
void MapObj_Flag::collide(CFGameObject* o) {
    MapObj_Player& oplayer = *GameController::current().getControlPlayer();
    switch(o->type()){
        case GAMEOBJ_PLAYER:{
            MapObj_Player& other = *(MapObj_Player*)o;
            if(pauseFollow.getElapsedTime().asSeconds() < 8) return;
            if(other.isDead() || other.team == team) return;
            if(followObject != o){
                if(other.team != oplayer.team){
                    playSound(AUDIO_ALERT);
                }
                MplayController::flagWarn(other.team);
                followObject = o;
            }
            break;}
        case GAMEOBJ_BASE:{
            MapObj_Base& other = *(MapObj_Base*)o;
            if(other.team == team || followObject == nullptr) return;
            if(other.team == oplayer.team){
                playSound(AUDIO_POINT);
            } else {
                playSound(AUDIO_LOSEFLAG);
            }
            MplayController::flagCapture(other.team);
            GameController::current().scoreTeam(other.team);
            followObject = nullptr;
            pauseFollow.restart();
            break;}
        default:
            return;
    }
}


MapObj_Pickup::MapObj_Pickup(int x, int y) : CFGameObject(x, y, type()) {
    InitMaskCircle();
    depth(0.4);
    dead = false;
    alpha = 255;
    bonusType = 1 + rand() % BONUS_TYPE_COUNT;
}
MapObj_Pickup::~MapObj_Pickup() {
}
void MapObj_Pickup::step(sf::Time& delta){
    if(dead){
        if(alpha < 0){
            destroy();
        } else {
            sprite.setColor(sf::Color(255, 255, 255, alpha));
            alpha -= 4;
        }
    }
    CFGameObject::step(delta); // inherited event
}

MapObj_Player::MapObj_Player(int x, int y, int t) : CFGameObject(x, y, type()) {
    InitMaskCircle();
    depth(0.8);
    sprite.frame(t);
    team = t;

    magSize = 20; // magazine size
    moveSpeed = 2.1; // default speed player should move at
    life = 100; // health
    alpha = 255; // visibility
    ammo = magSize; // current ammo in magazine
    bonusType = BONUS_NONE; // holding bonus
    bonusActive = false; // bonus currently active

    dir = 0;
    speed = moveSpeed; // current moving speed of player
    canMove = true;
    invisible = false; // player disappears - bonus
    isAi = false;

    mplayUpdated = true;
    mplayBonusReset = false;
    mplayRespawn = false;
    mplayFired = false;
    mplayID = 0;

    xprev = x;
    yprev = y;

    myPath = grid->generatePath(x/16, y/16);
}
MapObj_Player::~MapObj_Player() {
    delete myPath;
    GameController& cont = GameController::current();
    if(cont.getControlPlayer() == this) {
        cont.setControlPlayer(nullptr);
    }
}

bool MapObj_Player::isMe() const {
    return (GameController::current().getControlPlayer() == this);
}

void MapObj_Player::step(sf::Time &delta) {
    if(canMove){
        float dist = fabs(MyGame::direction_loop(direction) - dir);
        if(dist > 2.f) {
            direction += MyGame::direction_sign_toward(direction, dir) * fmin(1.f + (dist / 8.f), 15.f);
        }
        beginCollision();
        while(BaseCollidable* o = iterateCollidable()){
            if(o->mask == nullptr || !mask->collidesWith(*o->mask)) continue;
            collide((CFGameObject*)o);
        }
        if(invisible) {
            if(alpha > (isMe() ? 75 : 0) ) sprite.setColor(sf::Color(255, 255, 255, --alpha));
        } else {
            if(alpha < 255) sprite.setColor(sf::Color(255, 255, 255, ++alpha));
        }
    } else {
        if(isMe() && respawn.getElapsedTime().asSeconds() > RESPAWN_TIME){
            respawnMe();
        }
    }
    if(updatePath.getElapsedTime().asSeconds() > 2){
        int _xx, _yy;
        myPath->destination(x() / 8, y() / 8, true, -1);
        updatePath.restart();
    }

    if(isMe() || (isAi && isHost) ){
        if(!ammo && reload.getElapsedTime().asSeconds() > 4) {
            ammo = magSize;
        }
        if(bonusActive && bonusExpires.getElapsedTime().asSeconds() > 14){
            bonusActive = false;
            resetBonusStatus();
        }
    }
    CFGameObject::step(delta);  //inherit
}
void MapObj_Player::moveLeft() {
    if(!canMove) return;
    move(-speed, 0);
    mplayUpdated = true;
}
void MapObj_Player::moveRight() {
    if(!canMove) return;
    move(speed, 0);
    mplayUpdated = true;
}
void MapObj_Player::moveUp() {
    if(!canMove) return;
    move(0,-speed);
    mplayUpdated = true;
}
void MapObj_Player::moveDown() {
    if(!canMove) return;
    move(0, speed);
    mplayUpdated = true;
}
void MapObj_Player::setDirection(float d) {
    if(!canMove) return;
    dir = MyGame::direction_loop(d);
    mplayUpdated = true;
}
void MapObj_Player::shootGun() {
    if(!canMove) return;
    if(isMe() && !ammo) return;
    if(canShoot.getElapsedTime().asSeconds() > 0.25) {
        float xx, yy;
        MyGame::lengthdir(16, direction, xx, yy);
        playSoundDist(AUDIO_GUN);
        new MapObj_Bullet(x() + xx, y() + yy, index, direction, 6 + rand()%5);
        new MapObj_BulletImpact(x() + xx, y() + yy, direction, BULLET_IMPACT_SPARK, 30);
        if(invisible) alpha += 24; // slightly show when firing
        if(isMe() && !--ammo){
            reload.restart();
        }
        canShoot.restart();
    }
    mplayFired = true;
    mplayUpdated = true;
}
void MapObj_Player::hitPlayer(float damage) {
    if(!canMove) return;
    if((isAi && !isHost) || (!isAi && !isMe())) return;

    damage = round(damage);
    if(life < damage){
        killMe();
    } else {
        life -= damage;
    }
    mplayUpdated = true;
}
void MapObj_Player::killMe() {
    if(!canMove) return;
    canMove = false;
    sprite.setColor(sf::Color::Transparent);
    new MapObj_DeadPlayer(x(), y(), team);
    unsigned int snd[] = {AUDIO_DIE1, AUDIO_DIE2, AUDIO_DIE3};
    playSoundDist(snd[rand() % 3]);

    for(CFGameObject* o : CFGameObject::gameObjects){ // remove pointers from AI
        if(o == nullptr || o->type() != GAMEOBJ_AI) continue;
        MapObj_AI &p = *(MapObj_AI*)o;
        if(p.follow == this) p.follow = nullptr;
        if(p.fireAt == this) p.fireAt = nullptr;
    }

    if(isMe() || (isHost && isAi)){
        life = 0;
        mplayUpdated = true;
        respawn.restart();
    }
}
void MapObj_Player::respawnMe() {
    sprite.setColor(sf::Color::White);
    canMove = true;
    if(isMe()){
        #ifdef DEBUG_MODE
        bonusType = BONUS_SPAWNAI;
        #else
        bonusType = BONUS_NONE;
        #endif
        life = 100;
        bonusActive = false;
        GameController& cont = GameController::current();
        sf::Vector2f pos = cont.getBasePos(team);
        setPosition(pos.x, pos.y);
        xprev = pos.x;
        yprev = pos.y;
        mplayRespawn = true;
        mplayUpdated = true;
    }
}
void MapObj_Player::activateBonus() {
    if(!canMove) return;
    if(bonusType == BONUS_NONE) return;

    GameController& cont = GameController::current();

    if(bonusActive){
        resetBonusStatus();
    }

    switch(bonusType){
        case BONUS_SPEED:
            speed = moveSpeed + 1.4;
        break;
        case BONUS_INVISIBLE:
            invisible = true;
        break;
        case BONUS_WARPBASE:
            if(isMe()){
                sf::Vector2f pos(cont.getBasePos(team));
                setPosition(pos.x, pos.y);
            }
        break;
        case BONUS_RETURNFLAG:
            if(isHost){
                MapObj_Flag* myFlag;
                if(MplayController::getFlagByTeam(team, myFlag)){
                    myFlag->returnFlag();
                }
            }
        break;
        case BONUS_GAINHEALTH:
            if(isMe()){
                life = 100;
            }
        break;
        case BONUS_SPAWNAI:
            if(isHost){
                sf::Vector2f pos(cont.getBasePos(team));
                MapObj_AI* p = new MapObj_AI(pos.x, pos.y, team);
                MplayController::spawnPlayer(p);
                p->follow = this;
            }
            break;
    }

    cout << "Bonus Activated: " << bonusType << endl;

    if(isMe()){
        mplayBonus = bonusType;
        mplayUpdated = true;
    }

    bonusActive = true;
    bonusType = BONUS_NONE;
    bonusExpires.restart();
}
void MapObj_Player::resetBonusStatus() {
    speed = moveSpeed;
    invisible = false;

    mplayBonusReset = true;
    mplayUpdated = true;
}
void MapObj_Player::collide(CFGameObject* o) {
    switch(o->type()){
        case GAMEOBJ_BASE:{
            MapObj_Base& other = *(MapObj_Base*)o;
            break;}
        case GAMEOBJ_WALL:{
            MapObj_Wall& other = *(MapObj_Wall*)o;

            if(type() == GAMEOBJ_PLAYER){
                MyGame::collision_with_object(this, o, speed);
            } else {
                //MyGame::collision_with_object(this, o, speed * 3);
            }
            break;}
        case GAMEOBJ_AI:
        case GAMEOBJ_PLAYER:{ // Collide with either a player OR AI
            if(type() == GAMEOBJ_AI){ // I am AI
                MapObj_AI& other = *(MapObj_AI*)o;
                MapObj_AI& me = *(MapObj_AI*)this;

                if(other.isDead()) break;

                if((me.xto != -1 && (me.xto > x()) ^ (x() < other.x())) || (me.yto != -1 && (me.yto > y()) ^ (y() < other.y())) ) break;

                float dir = MyGame::point_direction(other.x(), other.y(), x(), y()),
                      vx, vy;
                int cx, cy;
                MyGame::lengthdir(speed*6,dir, vx, vy);

                cx = round((x() + vx) / 8.f);
                cy = round((y() + vy) / 8.f);
                if(!grid->getStatic(cx, cy)) {
                    setPosition(cx * 8, cy * 8);
                }
            }

            if(type() == GAMEOBJ_PLAYER){ // I am Player
                MapObj_Player& other = *(MapObj_Player*)o;
                if(other.isDead()) break;
                MyGame::collision_with_object(this, o, speed);
            }
            break;}
        case GAMEOBJ_FLAG:{
            MapObj_Flag& other = *(MapObj_Flag*)o;
            break;}
        case GAMEOBJ_PICKUP:{
            MapObj_Pickup& other = *(MapObj_Pickup*)o;
            if(other.dead || bonusType != BONUS_NONE || isAi) break;
            other.dead = true;
            bonusType = other.bonusType;
            cout << "Collected bonus: " << bonusType << endl;
            break;}
    }
}

unsigned int MapObj_AI::AI_ID_Counter = 2048; // MPLAY ID for AI ONLY - Forever Counter
MapObj_AI::MapObj_AI(int x, int y, int team) : MapObj_Player(x, y, team) {
    mplayID = AI_ID_Counter++;
    isAi = true;
    xto = -1;
    yto = -1;

    follow = nullptr;
    fireAt = nullptr;
    path = nullptr;
}

MapObj_AI::~MapObj_AI() {
    for(CFGameObject* o : CFGameObject::gameObjects){
        if(o == nullptr || o->type() != GAMEOBJ_AI) continue;
        MapObj_AI &p = *(MapObj_AI*)o;
        if(p.follow == this) p.follow = nullptr;
        if(p.fireAt == this) p.fireAt = nullptr;
    }
}

void MapObj_AI::mouseButtonPress(sf::Event::MouseButtonEvent &evt){
    MapObj_Player* me = GameController::current().getControlPlayer();
    if(me->team != team) return;

    if(evt.button == sf::Mouse::Right){
        toggleFollowMe(me);
    }
}

void MapObj_AI::step(sf::Time& delta){
    if(isHost){
        if(searchEnemy.getElapsedTime().asSeconds() > (fireAt==nullptr ? 1 : 3) ){
            vector<MapObj_Player*> found;
            for(CFGameObject* o : CFGameObject::gameObjects){
                if(o == nullptr || o == this || (o->type() != GAMEOBJ_AI && o->type() != GAMEOBJ_PLAYER)) continue;
                MapObj_Player* p = (MapObj_Player*)o;
                if(p->team == team || p->alpha < 10) continue; // same team or player is invisible
                if(MyGame::collision_line(x(), y(), p->x(), p->y(), {GAMEOBJ_WALL})) continue;
                float xd = x()-p->x(), yd = y()-p->y(), dist = pow(xd*xd+yd*yd, 0.5);
                if(dist > 260) continue;
                found.push_back(p);
            }
            fireAt = found.size() ? found[rand() % found.size()] : nullptr;
            searchEnemy.restart();
        }

        if(fireAt != nullptr){
            if(motionDir.getElapsedTime().asSeconds() > 0.175){
                setDirection(MyGame::point_direction(x(), y(), fireAt->x(), fireAt->y()) + (rand()%160 - 80)/10.f);
                motionDir.restart();
            }
            shootGun();

            if(follow == nullptr){
                if(path == nullptr) path = fireAt->myPath;
            }
        }

        if(follow != nullptr){
            if(path == nullptr) path = follow->myPath;
            if(fireAt == nullptr && motionDir.getElapsedTime().asSeconds() > 5){
                float pxd = fabs(follow->x() - x()), pyd = fabs(follow->y() - y()), pDist = pow(pxd*pxd+pyd*pyd, 0.5);
                if(pDist < 164){
                    setDirection(MyGame::point_direction(x(), y(), follow->x(), follow->y()) + (rand()%320 - 160)/10.f);
                } else {
                    setDirection((rand()%3600)/10.f);
                }
                motionDir.restart();
            }
        } else {
            if(motionDir.getElapsedTime().asSeconds() > 5){
                setDirection((rand()%3600)/10.f);
                motionDir.restart();
            }
        }

        if(xto == -1 && yto == -1){
            int bx, by, _xx, _yy;
            bx = round((x())/8.f);
            by = round((y())/8.f);
            setPosition(bx * 8, by * 8);

            if(path != nullptr){
                int xdest, ydest; path->getDestination(xdest, ydest);
                float xd = (x() - xdest*8), yd = (y() - ydest*8), dist = pow(xd*xd+yd*yd, 0.5);
                if(dist > 80){
                    PathGrid::Step s = path->nextStep(bx, by, _xx, _yy, 16);
                    switch(s){
                        case PathGrid::Move:
                            xto = _xx * 8;
                            yto = _yy * 8;
                            break;
                        default:
                            xto = -1;
                            yto = -1;
                            break;
                    }
                } else {
                    path = nullptr;
                }
            }
        }

        float xd = fabs(xto - x()), yd = fabs(yto - y());
        if(xto != -1) {if(xd > speed*1.1) {if(xto < x()) moveLeft(); else moveRight();} else xto = -1;}
        if(yto != -1) {if(yd > speed*1.1) {if(yto < y()) moveUp(); else moveDown();} else yto = -1;}

        if(motionStop.getElapsedTime().asSeconds() > 3){
            path = nullptr;
            motionStop.restart();
        }
    }

    if(isDead()){
        delete this;
        return;
    }
    MapObj_Player::step(delta); // inherited
}

void MapObj_AI::toggleFollowMe(MapObj_Player* me, bool direct) {
    if(!direct){
        View& v = MyGame::current().view();
        Mask point(MASK_CIRCLE, v.viewMouseX(), v.viewMouseY());
        point.initCircle(8);
        if(!point.collidesWith(*mask)) return;
    }
    if(isHost){
        follow = (follow == nullptr ? me : nullptr);
    } else {
        MplayController::aiToggleFollow(this, me);
    }
}

void MapObj_AI::killMe() {
    canMove = false;
    new MapObj_DeadPlayer(x(), y(), team);
    unsigned int snd[] = {AUDIO_DIE1, AUDIO_DIE2, AUDIO_DIE3};
    playSoundDist(snd[rand() % 3]);
    if(isHost){
        mplayUpdated = true;
    }
}

MapObj_Bullet::MapObj_Bullet(int x, int y, int gen, float dir, float dmg) : CFGameObject(x, y, type()) {
    depth(0.4);
    direction = dir + float(-4 + rand() % 8);    //bullet direction of fire
    myGenerator = gen;  //bullet came from this index
    damage = dmg; //bullet damage dealing
    speed = 30 + float(rand() % 4);  //bullet flight speed

    InitMaskCircle(10);

    beginCollision();
    while(BaseCollidable* o = iterateCollidable()) {
        if(o->mask == nullptr || !mask->collidesWith(*o->mask)) continue;
        if(o->type() == GAMEOBJ_WALL) speed = 4;
    }
    InitMaskCircle(2);
}
MapObj_Bullet::~MapObj_Bullet() {
}
void MapObj_Bullet::step(sf::Time &delta) {
    float vx, vy;
    MyGame::lengthdir(speed, direction, vx, vy);
    move(vx, vy);

    beginCollision();
    while(BaseCollidable* o = iterateCollidable()) {
        if(o->mask == nullptr || !mask->collidesWith(*o->mask)) continue;
        collide((CFGameObject*)o);
    }

    if(speed > 12) speed /= 1.008; else {
        destroy();
        new MapObj_BulletImpact(x(), y(), direction, BULLET_IMPACT_WALL);
    }

    CFGameObject::step(delta); //inherit
}
void MapObj_Bullet::collide(CFGameObject* o) {
    switch(o->type()){
        case GAMEOBJ_AI:
        case GAMEOBJ_PLAYER:{
            MapObj_Player& other = *(MapObj_Player*)o;
            if(other.index == myGenerator || other.isDead()) return;
            other.hitPlayer(damage);
            playSoundDist(AUDIO_HURT);
            new MapObj_BulletImpact(x(), y(), direction, BULLET_IMPACT_HUMAN);
            break;}
        case GAMEOBJ_WALL:{
            float xdist = abs(xprev - o->x()), ydist = abs(yprev - o->y()),
                    xx = o->x(), yy = o->y();
            if(xdist < ydist){
                xx = xprev;
                if(yprev < o->y()) yy -= 20; else yy += 20;
            } else {
                yy = yprev;
                if(xprev < o->x()) xx -= 20; else xx += 20;
            }

            unsigned int snd[] = {AUDIO_MISS1, AUDIO_MISS2, AUDIO_MISS3, AUDIO_MISS4, AUDIO_MISS5,
                                  AUDIO_MISS6, AUDIO_MISS7, AUDIO_MISS8, AUDIO_MISS9, AUDIO_MISS10};
            playSoundDist(snd[rand() % 10]);
            new MapObj_BulletImpact(xx, yy, direction, BULLET_IMPACT_WALL);
            break;}
        default:
            return;
    }
    destroy();
}



MapObj_BulletImpact::MapObj_BulletImpact(int x, int y, float dir, int ty, int lf) : CFGameObject(x, y, type(), ty) {
    depth(0.6);
    sprite.frameDelay(life/100);
    direction = dir;
    life = lf;
}
MapObj_BulletImpact::~MapObj_BulletImpact() {
}
void MapObj_BulletImpact::step(sf::Time &delta) {
    sprite.next();
    if(die.getElapsedTime().asMilliseconds() > life) destroy();
    CFGameObject::step(delta);  //inherit
}



MapObj_DeadPlayer::MapObj_DeadPlayer(int x, int y, int t) : CFGameObject(x, y, type()), border(&MyGame::gameAnimations->DeadBody) {
    depth(0.25);
    direction = float(rand() % 720)/2;
    alpha = 510;
    sprite.frame(t);
    border.frame(3);
    border.setRotation(direction);
    border.setOrigin(sprite.getOrigin());
    border.setPosition(sprite.getPosition());
}
MapObj_DeadPlayer::~MapObj_DeadPlayer() {
}
void MapObj_DeadPlayer::step(sf::Time &delta) {
    if(fade.getElapsedTime().asMilliseconds() > 40){
        fade.restart();
        if(alpha > 255){
            sprite.setColor(sf::Color(255, 255, 255, (alpha--) - 255));
        } else {
            border.setColor(sf::Color(255, 255, 255, alpha--));
        }
    }
    if(alpha < 0){
        destroy();
    }

    CFGameObject::step(delta);  //inherit
}
void MapObj_DeadPlayer::draw(sf::RenderWindow &win, sf::Time &delta) {
    win.draw(border);
    CFGameObject::draw(win, delta); //inherited
}


/*
MapObj_::MapObj_(int x, int y) : CFGameObject(x, y, type()) {
}

MapObj_::~MapObj_() {
}
*/
