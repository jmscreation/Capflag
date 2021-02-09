#ifndef MPLAYCONTROLLER_H
#define MPLAYCONTROLLER_H

#define CMD_ACTIVE      0
#define CMD_MESSAGE     1
#define CMD_SEND_MAP    3
#define CMD_GAME_INFO   4
#define CMD_GAME_DATA   8
#define CMD_VALID_CLIENT  254
#define CMD_NEW_CLIENT  255

#define MSG_EXIT        0
#define MSG_START       1
#define MSG_INGAME      2

#define GAME_DATA_SYNC                  0
#define GAME_DATA_UPDATE_PLAYER         4
#define GAME_DATA_UPDATE_FLAG           5
#define GAME_DATA_SPAWN_PLAYER          8
#define GAME_DATA_SPAWN_BONUS           10
#define GAME_DATA_SPAWN_AI              12
#define GAME_DATA_FLAG_STATE            13
#define GAME_DATA_AI_TOGGLE_FOLLOW      14
#define GAME_DATA_UPDATE_SCORES         16


struct BufferContext {
    sf::Packet *in, *out;
};

class MplayController : public Engine::Active {
    static MplayController* currentController;
public:
    virtual ~MplayController();

    static void mplayEnd();
    static MplayController& mplayBegin(BufferContext ctx);
    static void spawnPlayer(MapObj_Player* player);
    static void spawnBonus(float xx, float yy, int ty);
    static void flagCapture(int team);
    static void flagWarn(int team);
    static void aiToggleFollow(MapObj_AI* ai, MapObj_Player* player);
    static bool getFlagByTeam(unsigned int findTeam, MapObj_Flag*& flag);
    static bool getPlayerById(unsigned int findID, MapObj_Player*& player);
    bool isHost, connected;

protected:
    BufferContext bufCtx;
    void step(sf::Time& delta);

    bool updateReadStep(sf::Packet& buf);
    void updateWriteStep(sf::Packet& buf);

private:
    MplayController();
};

#endif // MPLAYCONTROLLER_H
