#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

class GameController : public Engine::Active , public Engine::WorldDrawable {
    static GameController* currentController;
public:
    virtual ~GameController();
    static GameController& current();

    static GameSettings settings;

    void setControlPlayer(CFGameObject* obj);
    inline MapObj_Player* getControlPlayer() { return (MapObj_Player*)playerControlled; }
    inline int getTeamScore(int team) { if(team >= gameScores.size()) return 0; return gameScores.at(team); }
    inline int getGameTimeLeft() { return std::max(int(settings.gameTime - gameTime.getElapsedTime().asSeconds()), 0); }

    sf::Vector2f getBasePos(int team);
    const std::string getBonusName(unsigned int ty) const;
    void scoreTeam(int);
    void spawnMyPlayer(int team);
    void toggleFullscreen();
    void spawnNewPlayer(int x, int y, int team, bool isAi, unsigned int id);
    int spawnBonus(float xx, float yy, int ty=-1);
    inline void endGame() { gameEnd = true; }

    unsigned int mplayID;
    size_t gameSync;
    int myTeam;
    bool isHost, gameEnd, gameStarted;
    std::vector<int> gameScores;
    sf::FloatRect gameArea;

protected:
    GameController();
    InputController* inputCont;
    Engine::SoundInstance* music;
    sf::Clock startTimer, spawnBonusTimer, gameTime;
    CFGameObject *viewFollows, *playerControlled;
    std::vector<Engine::AnimatedSprite*> background;
    sf::VideoMode windowedMode, fullMode;
    bool fullscreen;

    virtual void step(sf::Time &delta);

private:
    void gameBegin();
    void drawWorldBack(sf::RenderWindow&, sf::Time&);
    void drawWorldFront(sf::RenderWindow&, sf::Time&);
};

#endif // GAMECONTROLLER_H
