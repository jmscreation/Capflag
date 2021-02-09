#ifndef __GAME_H_FILE__
#define __GAME_H_FILE__

#define GAME_ID         783294
#define GAME_PORT       55883
#define BROADCAST_PORT  5851
#define FILE_HEADER     45585354561085
#define FILE_VERSION    2

#define RESPAWN_TIME    6 //seconds for respawn
#define GAMESTART_TIME  3 //seconds for game start

#define TEAM_BLUE   0
#define TEAM_RED    1
//#define TEAM_ 2
//#define TEAM_ 3

#define TEX_GAME        0
#define TEX_BACKGROUND  1

#define AUDIO_MENU_MUSIC    0
#define AUDIO_GAME_MUSIC    1
#define AUDIO_POINT         2
#define AUDIO_GUN           3
#define AUDIO_ALERT         4
#define AUDIO_LOSEFLAG      5
#define AUDIO_HURT          6
#define AUDIO_DIE1          7
#define AUDIO_DIE2          8
#define AUDIO_DIE3          9
#define AUDIO_MISS1         10
#define AUDIO_MISS2         11
#define AUDIO_MISS3         12
#define AUDIO_MISS4         13
#define AUDIO_MISS5         14
#define AUDIO_MISS6         15
#define AUDIO_MISS7         16
#define AUDIO_MISS8         17
#define AUDIO_MISS9         18
#define AUDIO_MISS10        19

#define FNT_MAIN        0
#define FNT_MENU        1
#define FNT_ERROR       2


class MyGame: public Engine::App {
public:
    MyGame();
    virtual ~MyGame();

    static Engine::TextureResources* texPack;
    static Engine::AudioResources* audioPack;
    static Engine::FontResources* fontPack;

    static CFGameAnimations* gameAnimations;


    static float point_direction(float x1, float y1, float x2, float y2);
    static void lengthdir(float, float, float&, float&);
    static int direction_sign_toward(float, float);
    static float direction_loop(float);

    static std::vector<std::string> teamNames;
    inline static const std::string teamName(int team) { if(team >= teamNames.size()) return ""; return teamNames.at(team); }

    static CFGameObject* collision_line(float x1, float y1, float x2, float y2, std::vector<int> objectTypes);
    static void collision_with_object(CFGameObject* me, CFGameObject* other, float speed);
    static const sf::Clock gameRuntime;

private:
    //void stepBefore(sf::Time &delta);
    void stepAfter(sf::Time &delta);
};





#endif // __GAME_H_FILE__
