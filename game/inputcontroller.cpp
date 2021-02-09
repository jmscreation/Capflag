#include "includes.h"

using namespace std;
using namespace Engine;

InputController::InputController(): moveDir(0) {
    myPlayer = NULL;
}

InputController::~InputController() {
}


void InputController::mouseButtonPress(sf::Event::MouseButtonEvent &evt){
    switch(evt.button){
        case sf::Mouse::Left: moveDir |= 32; break;
    }
}

void InputController::mouseButtonRelease(sf::Event::MouseButtonEvent &evt){
    switch(evt.button){
        case sf::Mouse::Left: moveDir &= ~32; break;
    }
}

void InputController::keyPress(sf::Event::KeyEvent &evt){
    switch(evt.code){
        case sf::Keyboard::A: moveDir |= 1; break;
        case sf::Keyboard::S: moveDir |= 2; break;
        case sf::Keyboard::D: moveDir |= 4; break;
        case sf::Keyboard::W: moveDir |= 8; break;
        case sf::Keyboard::E: moveDir |= 16; break;
        case sf::Keyboard::Space: moveDir |= 32; break;
        case sf::Keyboard::Q: moveDir |= 64; break;
    }
}

void InputController::keyRelease(sf::Event::KeyEvent &evt){
    switch(evt.code){
        case sf::Keyboard::A: moveDir &= ~1; break;
        case sf::Keyboard::S: moveDir &= ~2; break;
        case sf::Keyboard::D: moveDir &= ~4; break;
        case sf::Keyboard::W: moveDir &= ~8; break;
        case sf::Keyboard::E: moveDir &= ~16; break;
        case sf::Keyboard::Space: moveDir &= ~32; break;
        case sf::Keyboard::Q: moveDir &= ~64; break;
    }
}

void InputController::step(sf::Time &delta){
    if(myPlayer == NULL) return;
    MapObj_Player &p = *(MapObj_Player*)myPlayer;
    View &v = MyGame::current().view();

    if(moveDir & 1){    //Left
        p.moveLeft();
    }
    if(moveDir & 2){    //Down
        p.moveDown();
    }
    if(moveDir & 4){    //Right
        p.moveRight();
    }
    if(moveDir & 8){    //Up
        p.moveUp();
    }
    if(moveDir & 16){    //Bonus
        p.activateBonus();
    }
    if(moveDir & 32){   //Shoot
        p.shootGun();
    }
    if(moveDir & 64){   //Clear Bonus
        p.clearBonus();
    }

    p.setDirection(MyGame::point_direction(p.x(), p.y(), v.viewMouseX(), v.viewMouseY()));

}
