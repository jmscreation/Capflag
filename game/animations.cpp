
#include "includes.h"

using namespace Engine;

CFGameAnimations::CFGameAnimations():
        Wall(MyGame::texPack->getTexture(TEX_GAME), std::vector<int>{1}, 0, 0, 32, 32, 32),
        Player(MyGame::texPack->getTexture(TEX_GAME), std::vector<int>{4}, 32, 0, 32, 32, 32),
        Base(MyGame::texPack->getTexture(TEX_GAME), std::vector<int>{2}, 160, 0, 32, 32, 32),
        Pickup(MyGame::texPack->getTexture(TEX_GAME), std::vector<int>{1}, 224, 0, 32, 32, 32),
        Impact(MyGame::texPack->getTexture(TEX_GAME), std::vector<int>{6}, 256, 0, 32, 32, 32),
        Flag(MyGame::texPack->getTexture(TEX_GAME), std::vector<int>{2}, 448, 0, 32, 32, 32),
        DeadBody(MyGame::texPack->getTexture(TEX_GAME), std::vector<int>{4}, 512, 0, 32, 32, 32),
        BulletSpark(MyGame::texPack->getTexture(TEX_GAME), std::vector<int>{3}, 640, 0, 32, 32, 32),
        Bullet(MyGame::texPack->getTexture(TEX_GAME), std::vector<int>{1}, 736, 0, 32, 32, 32),
        WallFlesh(MyGame::texPack->getTexture(TEX_GAME), std::vector<int>{4}, 768, 0, 32, 32, 32),


        Background(MyGame::texPack->getTexture(TEX_BACKGROUND), std::vector<int>{1}, 0, 0, 32, 32, 1)
{}

CFGameAnimations::~CFGameAnimations(){
}
