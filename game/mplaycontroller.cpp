#include "includes.h"

using namespace std;
using namespace Engine;

MplayController* MplayController::currentController = nullptr;

MplayController& MplayController::mplayBegin(BufferContext ctx) {
    if(currentController == nullptr) currentController = new MplayController;
    currentController->bufCtx = ctx;
    return *currentController;
}

void MplayController::mplayEnd() {
    if(currentController != nullptr){
        *currentController->bufCtx.out << sf::Uint8(CMD_MESSAGE) << sf::Uint8(MSG_EXIT);
        delete currentController;
    }
}

MplayController::MplayController() {
    isHost = MplayServer::isRunning();
}

MplayController::~MplayController() {
    currentController = nullptr;
}

bool MplayController::getPlayerById(unsigned int findID, MapObj_Player*& player) {
    for(CFGameObject* o : CFGameObject::gameObjects) {
        switch(o->type()){
            case GAMEOBJ_PLAYER:
            case GAMEOBJ_AI:{
                MapObj_Player& obj = *(MapObj_Player*)o;
                if(obj.mplayID == findID){
                    player = (MapObj_Player*)o;
                    return true;
                }
            }
        }
    }
    return false;
}

bool MplayController::getFlagByTeam(unsigned int findTeam, MapObj_Flag*& flag) {
    for(CFGameObject* o : CFGameObject::gameObjects) {
        if(o->type() == GAMEOBJ_FLAG){
            MapObj_Flag& obj = *(MapObj_Flag*)o;
            if(obj.team == findTeam){
                flag = (MapObj_Flag*)o;
                return true;
            }
        }
    }
    return false;
}


void MplayController::spawnPlayer(MapObj_Player* player) {
    if(currentController == nullptr) return;
    sf::Packet& buf = *currentController->bufCtx.out;

    buf << sf::Uint8(CMD_GAME_DATA) << sf::Uint8(GAME_DATA_SPAWN_PLAYER) << sf::Uint32(player->mplayID) << sf::Int16(player->x()) << sf::Int16(player->y()) << sf::Uint8(player->team) << sf::Uint8(player->isAi);
}

void MplayController::spawnBonus(float xx, float yy, int ty){
    if(currentController == nullptr) return;
    sf::Packet& buf = *currentController->bufCtx.out;

    buf << sf::Uint8(CMD_GAME_DATA) << sf::Uint8(GAME_DATA_SPAWN_BONUS) << sf::Int16(xx) << sf::Int16(yy) << sf::Uint8(ty);
}

void MplayController::flagCapture(int team){
    if(currentController == nullptr) return;
    sf::Packet& buf = *currentController->bufCtx.out;

    buf << sf::Uint8(CMD_GAME_DATA) << sf::Uint8(GAME_DATA_FLAG_STATE) << sf::Uint8(team) << sf::Uint8(0);
}

void MplayController::flagWarn(int team){
    if(currentController == nullptr) return;
    sf::Packet& buf = *currentController->bufCtx.out;

    buf << sf::Uint8(CMD_GAME_DATA) << sf::Uint8(GAME_DATA_FLAG_STATE) << sf::Uint8(team) << sf::Uint8(1);
}

void MplayController::aiToggleFollow(MapObj_AI* ai, MapObj_Player* player){
    if(currentController == nullptr) return;
    sf::Packet& buf = *currentController->bufCtx.out;

    buf << sf::Uint8(CMD_GAME_DATA) << sf::Uint8(GAME_DATA_AI_TOGGLE_FOLLOW) << sf::Uint32(ai->mplayID) << sf::Uint32(player->mplayID);
}

void MplayController::updateWriteStep(sf::Packet& buf) {
    {
        static sf::Clock holdUpdate;
        if(holdUpdate.getElapsedTime().asMilliseconds() < 15) return;
        holdUpdate.restart();
    }

    {
        static sf::Clock syncUpdate;
        if(syncUpdate.getElapsedTime().asSeconds() > 1){
            syncUpdate.restart();
            buf << sf::Uint8(CMD_GAME_DATA) << sf::Uint8(GAME_DATA_SYNC) << GameController::current().gameSync;
        }
    }

    if(GameController::current().getControlPlayer() != nullptr) {   //Update my player data
        MapObj_Player& obj = *GameController::current().getControlPlayer();
        if(obj.mplayUpdated){
            sf::Uint8 flags;
            flags = obj.mplayFired*1 + obj.isDead()*2 + obj.mplayBonusReset*4 + obj.mplayRespawn*8;
            buf << sf::Uint8(CMD_GAME_DATA) << sf::Uint8(GAME_DATA_UPDATE_PLAYER) << sf::Uint32(obj.mplayID) << sf::Int16(obj.x()) << sf::Int16(obj.y()) << obj.direction << sf::Uint8(obj.mplayBonus) << flags;

            obj.mplayUpdated = false;
            obj.mplayFired = false;
            obj.mplayBonusReset = false;
            obj.mplayRespawn = false;
            obj.mplayBonus = BONUS_NONE;
        }
    }

    if(isHost){
        for(CFGameObject* o : CFGameObject::gameObjects) {
            switch(o->type()){
                case GAMEOBJ_WALL:
                case GAMEOBJ_BASE:
                case GAMEOBJ_BULLET:
                case GAMEOBJ_BULLET_IMPACT:
                case GAMEOBJ_DEAD_BODY:
                case GAMEOBJ_PICKUP:
                case GAMEOBJ_PLAYER:
                    continue;
                case GAMEOBJ_AI:{
                    //Update all AI players data
                    MapObj_Player& obj = *(MapObj_Player*)o;
                    if(obj.mplayUpdated){
                        sf::Uint8 flags, bonus;
                        flags = obj.mplayFired*1 + obj.isDead()*2; // 4, 8 are not allowed for AI
                        buf << sf::Uint8(CMD_GAME_DATA) << sf::Uint8(GAME_DATA_UPDATE_PLAYER) << sf::Uint32(obj.mplayID) << sf::Int16(obj.x()) << sf::Int16(obj.y()) << obj.direction << bonus << flags;
                        obj.mplayUpdated = false;
                        obj.mplayFired = false;
                    }
                    continue;
                }
                case GAMEOBJ_FLAG:{
                    MapObj_Flag& obj = *(MapObj_Flag*)o;
                    if(obj.mplayMoveUpdate){
                        buf << sf::Uint8(CMD_GAME_DATA) << sf::Uint8(GAME_DATA_UPDATE_FLAG) << sf::Uint8(obj.team) << sf::Int16(obj.x()) << sf::Int16(obj.y());
                        obj.mplayMoveUpdate = false;
                    }
                    continue;
                }
            }
        }

        static sf::Clock sendScore;
        if(sendScore.getElapsedTime().asSeconds() > 1){
            sendScore.restart();
            buf << sf::Uint8(CMD_GAME_DATA) << sf::Uint8(GAME_DATA_UPDATE_SCORES);
            for(int s : GameController::current().gameScores) buf << sf::Uint16(s);
        }
    }

}

bool MplayController::updateReadStep(sf::Packet& buf) {
    sf::Uint8 cmd;
    if(!(buf >> cmd)) return false;
    switch(cmd){
        case GAME_DATA_SYNC:{
            sf::Uint64 sync;
            if(!(buf >> sync)) return false;
            //cout << "other system sync: " << sync << "\nmy system sync: " << GameController::current().gameSync << endl;
            if(sync > GameController::current().gameSync){
                size_t offset = sync - GameController::current().gameSync;
                //cout << "offset: " << offset << endl;
                if(offset > 20){
                    cout << "System is slow by " << offset << " steps" << endl;
                    GameController::current().gameSync += offset;
                    buf.clear(); // skip the buffer to keep up
                }
            }

            break;
        }
        case GAME_DATA_UPDATE_PLAYER:{
            sf::Uint32 id;
            sf::Int16 xx, yy;
            float dir;
            sf::Uint8 flags, bonus;

            if( ! (buf >> id >> xx >> yy >> dir >> bonus >> flags) ) return false;

            bool fired, dead, bonusReset, respawn;

            MapObj_Player* o;
            if(getPlayerById(id, o)){
                o->setPosition(xx, yy);
                o->setDirection(dir);

                fired = flags & 1;
                dead = flags & 2;
                bonusReset = flags & 4;
                respawn = flags & 8;

                if(fired) o->shootGun(); // player fire gun
                if(dead) o->killMe(); // kill player
                if(respawn) o->respawnMe(); // respawn player
                if(bonusReset) o->resetBonusStatus(); // reset bonus state

                if(bonus != BONUS_NONE){ // activate bonus state
                    o->bonusType = bonus;
                    o->activateBonus();
                }
            }

            break;
        }
        case GAME_DATA_UPDATE_FLAG:{
            sf::Uint8 team;
            sf::Int16 xx, yy;

            if( !(buf >> team >> xx >> yy) ) return false;

            MapObj_Flag* o;
            if(getFlagByTeam(team, o)){
                o->setPosition(xx, yy);
            }

            break;
        }
        case GAME_DATA_FLAG_STATE:{
            sf::Uint8 team, ty;

            if( !(buf >> team >> ty) ) return false;

            MapObj_Player& oplayer = *GameController::current().getControlPlayer();
            if(team != oplayer.team){
                if(ty) CFGameObject::playSound(AUDIO_ALERT); // warn
                else   CFGameObject::playSound(AUDIO_LOSEFLAG); // captured
            } else {
                if(!ty) CFGameObject::playSound(AUDIO_POINT); // captured
            }
            break;
        }
        case GAME_DATA_AI_TOGGLE_FOLLOW:{
            sf::Uint32 aiID, playerID;

            if( !(buf >> aiID >> playerID) ) return false;

            if(!isHost) break;

            MapObj_Player *_ai, *follow;
            if(!getPlayerById(aiID, _ai) || !getPlayerById(playerID, follow)) break;
            MapObj_AI *ai = dynamic_cast<MapObj_AI*>(_ai);
            if(ai == nullptr){
                cout << "dynamic_cast<MapObj_AI*> failed!" << endl;
                break;
            }

            ai->toggleFollowMe(follow, true);
            break;
        }
        case GAME_DATA_SPAWN_PLAYER:{
            sf::Uint32 id;
            sf::Int16 xx, yy;
            sf::Uint8 team, ai;

            if( !(buf >> id >> xx >> yy >> team >> ai) ) return false;

            GameController::current().spawnNewPlayer(xx, yy, team, ai, id);

            break;
        }
        case GAME_DATA_UPDATE_SCORES:{
            for(int& s : GameController::current().gameScores){
                sf::Uint16 score;
                if( !(buf >> score) ) return false;
                s = score;
            }
            break;
        }
        case GAME_DATA_SPAWN_BONUS:{
            sf::Int16 xx, yy;
            sf::Uint8 bonusTy;

            if( !(buf >> xx >> yy >> bonusTy) ) return false;

            GameController::current().spawnBonus(xx, yy, bonusTy);

            break;
        }
        default:
            return false;
    }
    return true;
}

void MplayController::step(sf::Time& delta){
    sf::Packet &inData = *bufCtx.in, &outData = *bufCtx.out;
    static sf::Clock testClock;
    if(testClock.getElapsedTime().asSeconds() > 5){
        testClock.restart();
        if(isHost) {
            outData << sf::Uint8(CMD_ACTIVE) << string("Server Runtime " + to_string(floor(MyGame::gameRuntime.getElapsedTime().asSeconds())) );
        }
    }
    static sf::Clock timeout;
    timeout.restart();
    if(inData.getDataSize())
        while(!inData.endOfPacket()){
            if(timeout.getElapsedTime().asSeconds() > 5) { inData.clear(); break;} // timeout clears buffer to prevent bad buffer
            sf::Uint8 cmd;
            inData >> cmd;
            if(cmd == CMD_GAME_DATA){   //Data that corresponds to the game map objects
                if(!updateReadStep(inData)){
                    cout << "Warning: game command failed - buffer cleared" << endl;
                    break;
                }
                continue;
            }

            if(cmd == CMD_ACTIVE){
                string msg;
                inData >> msg;
                continue;
            }
            if(cmd == CMD_MESSAGE){
                sf::Uint8 msg;
                inData >> msg;
                switch(msg){
                    case MSG_START:
                        cout << "Cannot start game in game" << endl;
                    break;
                    case MSG_EXIT:
                        if(!isHost){
                            GameController::current().endGame();
                        } else {
                            cout << "A player left the game" << endl;
                        }
                        return;
                }
                continue;
            }
            if(cmd == CMD_SEND_MAP){
                cout << "Cannot load level in game" << endl;
                break;
            }
            if(cmd == CMD_GAME_INFO){
                unsigned int time;
                inData >> time;
                continue;
            }
            if(cmd == CMD_NEW_CLIENT){
                string nam;
                inData >> nam;
                cout << nam << " joined the game" << endl;
                if(isHost){
                    outData << sf::Uint8(CMD_MESSAGE) << MSG_INGAME;
                }
                continue;
            }
        }
    inData.clear();

    updateWriteStep(outData);
}
