#include "includes.h"

using namespace std;
using namespace Engine;

TextureResources* MyGame::texPack = nullptr;
AudioResources* MyGame::audioPack = nullptr;
FontResources* MyGame::fontPack = nullptr;
CFGameAnimations* MyGame::gameAnimations = nullptr;

const sf::Clock MyGame::gameRuntime;

std::vector<std::string> MyGame::teamNames = {
    "Blue",
    "Red"
};


MyGame::MyGame(): App(1, 1, "", sf::Style::None){
    setWindowTransparency(0);

    ///Load Game Resources
    texPack = new TextureResources({_PNG_GAME_SPRITES, _PNG_BACKGROUND});
    if(!texPack->successful()){
        throw runtime_error("Texture Resources failed to load");
        exit(-1);
    }
    audioPack = new AudioResources({_OGG_MENU_MUSIC, _OGG_GAME_MUSIC,
                                   _OGG_POINT, _OGG_GUN, _OGG_ALERT, _OGG_LOSEFLAG, _OGG_HURT,
                                   _OGG_DIE1, _OGG_DIE2, _OGG_DIE3,
                                   _OGG_MISS1, _OGG_MISS2, _OGG_MISS3, _OGG_MISS4, _OGG_MISS5,
                                   _OGG_MISS6, _OGG_MISS7, _OGG_MISS8, _OGG_MISS9, _OGG_MISS10});

    fontPack = new FontResources({_FONT_MAIN, _FONT_MENU, _FONT_ERROR});
    if(!fontPack->successful()){
        throw runtime_error("Font Resources failed to load");
        exit(-1);
    }
    gameAnimations = new CFGameAnimations;  //Init game animations

    memset(&GameController::settings, 0, sizeof(GameSettings)); // zero fill game settings

    new MainMenu;
    setWindowTransparency(255);

}

MyGame::~MyGame(){
    delete gameAnimations;
    delete texPack;
    delete audioPack;
    delete fontPack;

    MainMenu::freeMemory(); // deletes statically allocated memory
}


void MyGame::stepAfter(sf::Time& delta) {
    static bool exitSwitch = false;
    static sf::Clock exitTimer;
    if(!window().isOpen() && !exitSwitch) {
        exitTimer.restart();
        exitSwitch = true;
    }
    if(exitSwitch && exitTimer.getElapsedTime().asSeconds() > 2) exit(0);
}

float MyGame::point_direction(float x1, float y1, float x2, float y2){
    return atan2(y2 - y1, x2 - x1) * RAD2DEG;
}

void MyGame::lengthdir(float dist, float dir, float &x, float &y) {
    dir *= DEG2RAD;
    x = cos(dir) * dist;
    y = sin(dir) * dist;
}

int MyGame::direction_sign_toward(float dir, float to) {
    float d = direction_loop(to) - direction_loop(dir);
    if(d < 0) d += 360;
    return 1+(d > 180)*-2-!d;
}

float MyGame::direction_loop(float dir) {
    return fmod(fmod(dir, 360) + 360, 360);
}

bool lineRectIntersect(const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::FloatRect&& rect) {
    if( (p1.x < rect.left && p2.x < rect.left)  ||
        (p1.y < rect.top && p2.y < rect.top)    ||
        (p1.x > rect.left + rect.width && p2.x > rect.left + rect.width)  ||
        (p1.y > rect.top + rect.height && p2.y > rect.top + rect.height)) return false;
    if(rect.contains(p1) || rect.contains(p2)) return true;
    // Find min and max X for the segment
    auto minX = std::min(p1.x, p2.x);
    auto maxX = std::max(p1.x, p2.x);
    // Find the intersection of the segment's and rectangle's x-projections
    if (maxX > rect.left + rect.width) {
        maxX = rect.left + rect.width;
    }
    if (minX < rect.left) {
        minX = rect.left;
    }
    // If Y-sf::Vector2fprojections do not intersect then there's no intersection
    if (minX > maxX) { return false; }
    // Find corresponding min and max Y for min and max X we found before
    auto minY = p1.y;
    auto maxY = p2.y;
    auto dx = p2.x - p1.x;
    if (std::abs(dx) > 0.001f) {
        auto k = (p2.y - p1.y) / dx;
        auto b = p1.y - k * p1.x;
        minY = k * minX + b;
        maxY = k * maxX + b;
    }
    if (minY > maxY) std::swap(minY, maxY);
    // Find the intersection of the segment's and rectangle's y-projections
    if (maxY > rect.top + rect.height) maxY = rect.top + rect.height;
    if (minY < rect.top) minY = rect.top;
    // If Y-projections do not intersect then there's no intersection
    if (minY > maxY) return false;
    return true;
}

CFGameObject* MyGame::collision_line(float x1, float y1, float x2, float y2, vector<int>objectTypes) {
    for(CFGameObject* o : CFGameObject::gameObjects){
        if(o == nullptr || o->mask == nullptr || o->mask->type != MASK_RECT) continue;
        if(find(objectTypes.begin(), objectTypes.end(), o->type()) == objectTypes.end()) continue;
        //Engine::Mask::
        if(lineRectIntersect({x1, y1}, {x2, y2},
            (sf::FloatRect){o->x() - o->mask->xorig, o->y() - o->mask->yorig, (o->mask->rect).w, (o->mask->rect).h}))
            return o;
    }
    return nullptr;
}

void MyGame::collision_with_object(CFGameObject* m, CFGameObject* o, float speed, float strength) {
    CFGameObject &me = *m, &other = *o;
    if(other.mask != nullptr){
        if(other.mask->type == MASK_CIRCLE){
            sf::Vector2f ray = {me.x() - other.x(), me.y() - other.y()};
            float len = sqrt(ray.x * ray.x + ray.y * ray.y);
            if(len == 0) return;

            sf::Vector2f norm = {ray.x / len, ray.y / len};

            float overlap = (me.mask->circle.radius + other.mask->circle.radius - len) * strength;

            if(overlap > 0) { // collision
                me.setPosition(me.x() + overlap * norm.x, me.y() + overlap * norm.y);
            }
        }
        if(other.mask->type == MASK_RECT){
            float nx = std::max(other.mask->x - other.mask->xorig, std::min(other.mask->x - other.mask->xorig + other.mask->rect.w, me.x()));
            float ny = std::max(other.mask->y - other.mask->yorig, std::min(other.mask->y - other.mask->yorig + other.mask->rect.h, me.y()));

            sf::Vector2f ray = {me.x() - nx, me.y() - ny};
            float len = sqrt(ray.x * ray.x + ray.y * ray.y);
            
            if(len == 0) return;

            sf::Vector2f norm = {ray.x / len, ray.y / len};

            float overlap = (me.mask->circle.radius - len) * strength;

            if(overlap > 0) { // collision
                me.setPosition(me.x() + overlap * norm.x, me.y() + overlap * norm.y);
            }
        }
    }
    /*
    speed*=0.5;
    float xdist = abs(other.x() - me.x()), ydist = abs(other.y() - me.y());
    if(xdist < ydist){
        me.y(me.yprev);
        if(ydist < 30){
            if(me.y() > other.y()){
                me.move(0, speed);
            } else {
                me.move(0, -speed);
            }
        }
    } else {
        me.x(me.xprev);
        if(xdist < 30){
            if(me.x() > other.x()){
                me.move(speed, 0);
            } else {
                me.move(-speed, 0);
            }
        }
    }
    */
}

sf::Packet& operator<<(sf::Packet& out, const GameSettings& in) {
    out << sf::Uint32(in.magSize) << sf::Uint32(in.gameTime) << sf::Uint32(in.bonusSpawnTime) << sf::Uint32(in.respawnTime);
    for(int i=0; i<BONUS_TYPE_COUNT; ++i) out << sf::Uint32(in.bonusDuration[i]);

    return out;
}

sf::Packet& operator>>(sf::Packet& in, GameSettings& out) {
    sf::Uint32 mSize, gTime, bsTime, rspTime, bTime;

    in >> mSize >> gTime >> bsTime >> rspTime;
    for(int i=0; i<BONUS_TYPE_COUNT; ++i){
        in >> bTime;
        out.bonusDuration[i] = bTime;
    }

    out.magSize = mSize;
    out.gameTime = gTime;
    out.bonusSpawnTime = bsTime;
    out.respawnTime = rspTime;

    return in;
}
