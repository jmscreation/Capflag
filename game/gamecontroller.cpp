#include "includes.h"

using namespace std;
using namespace Engine;

GameController* GameController::currentController = nullptr;
GameSettings GameController::settings;

GameController& GameController::current() {
    if(currentController == nullptr) currentController = new GameController;
    return *currentController;
}

GameController::GameController() {
    windowedMode = sf::VideoMode(640, 480);
    const vector<sf::VideoMode>& modes = sf::VideoMode::getFullscreenModes();
    for(const sf::VideoMode& mode : modes){
        cout << mode.width << " x " << mode.height << " @ " << mode.bitsPerPixel << "bit" << endl;
    }
    fullMode = modes[0];

    fullscreen = true; // inverted
    toggleFullscreen(); // create first window

    isHost = MplayServer::isRunning();
    CFGameObject::isHost = isHost;

    mplayID = 0;
    gameSync = 0;
    myTeam = -1;
    gameEnd = false;
    gameStarted = false;

    inputCont = new InputController;
    music = MyGame::audioPack->getSound(AUDIO_GAME_MUSIC)->loop();
    music->volume(0.35);

    playerControlled = nullptr;
    viewFollows = nullptr;
    gameScores = {0,0};
    gameScores[TEAM_RED] = 0;
    gameScores[TEAM_BLUE] = 0;

    // background tiles
    for(int i=0; i < 3500; i++){
        AnimatedSprite* spr = new AnimatedSprite(&MyGame::gameAnimations->Background);
        background.push_back(spr);
        spr->setPosition(float(i % 60)*32 - 480, float(i/60)*32 - 480);
    }
    gameArea = {1000,1000,0,0};
    for(CFGameObject* op : CFGameObject::gameObjects){
        if(op == nullptr) continue;
        CFGameObject& o = *op;
        if(o.x() < gameArea.left) gameArea.left = o.x();
        if(o.y() < gameArea.top) gameArea.top = o.y();
    }
    for(CFGameObject* op : CFGameObject::gameObjects){
        if(op == nullptr) continue;
        CFGameObject& o = *op;
        if(o.x() > gameArea.left + gameArea.width) gameArea.width = o.x() - gameArea.left;
        if(o.y() > gameArea.top + gameArea.height) gameArea.height = o.y() - gameArea.top;
    }

    CFGameObject::grid = new PathGrid(gameArea.width / 8, gameArea.height / 8);
    PathGrid &grid = *CFGameObject::grid;

    for(CFGameObject* o : CFGameObject::gameObjects){
        if(o == nullptr || o->type() != GAMEOBJ_WALL) continue;
        int xx = o->x()/8 - 3, yy = o->y()/8 - 3;
        grid.addStatic(xx, yy, 6, 6);
    }
}

GameController::~GameController() {
    music->stop(true);
    delete inputCont;
    delete CFGameObject::grid;
    for(AnimatedSprite* s : background) delete s;

    vector<CFGameObject*> temp = CFGameObject::gameObjects;
    CFGameObject::gameObjects.clear();
    for(CFGameObject* o : temp) delete o;

    MplayController::mplayEnd();

    currentController = nullptr;
}

void GameController::toggleFullscreen() {
    sf::Vector2f viewSize(480, 360);
    sf::FloatRect viewPort({0,0,1,1});
    float factor;

    if(fullscreen){
        MyGame::current().window().create(windowedMode, "", sf::Style::None);
        factor = 1 - (float)windowedMode.height * viewSize.x / viewSize.y / (float)windowedMode.width;
    } else {
        MyGame::current().window().create(fullMode, "", sf::Style::None | sf::Style::Fullscreen);
        factor = 1 - (float)fullMode.height * viewSize.x / viewSize.y / (float)fullMode.width;
    }
    viewPort.left = factor / 2.f;
    viewPort.width = 1 - factor;
    MyGame::current().view().setViewport(viewPort);
    MyGame::current().view().setSize(viewSize);
    MyGame::current().window().setFramerateLimit(45);
    fullscreen = !fullscreen;
}

const std::string GameController::getBonusName(unsigned int ty) const {
    switch(ty) {
        case BONUS_NONE: return "None";
        case BONUS_INVISIBLE: return "Invisibility";
        case BONUS_SPEED: return "Speed";
        case BONUS_RETURNFLAG: return "Return Flag";
        case BONUS_WARPBASE: return "Warp To Base";
        case BONUS_GAINHEALTH: return "Gain Full Health";
        case BONUS_SPAWNAI: return "Spawn 1 AI";
    }
    return "Unknown";
}

void GameController::gameBegin() {
    if(myTeam == -1) return;
    spawnMyPlayer(myTeam);
    gameTime.restart();
}

void GameController::spawnMyPlayer(int team) {
    if(playerControlled != nullptr) return;
    MapObj_Player* p = new MapObj_Player(0, 0, team);
    setControlPlayer(p);
    p->respawnMe();
    p->mplayID = mplayID;
    MplayController::spawnPlayer(p);
}

void GameController::spawnNewPlayer(int x, int y, int team, bool isAi, unsigned int id) {
    if(id == mplayID) { cout << "you try spawn me? no!" << endl; return; }  //impossible
    MapObj_Player *p;
    if(isAi){
        p = new MapObj_AI(x, y, team);
    } else {
        p = new MapObj_Player(x, y, team);
    }
    p->mplayID = id;
}

int GameController::spawnBonus(float xx, float yy, int ty) {
    MapObj_Pickup* p = new MapObj_Pickup(xx, yy);
    if(ty != -1){
        p->bonusType = ty;
    }
    return p->bonusType;
}

void GameController::setControlPlayer(CFGameObject* obj) {
    playerControlled = obj;
    viewFollows = obj;
    inputCont->myPlayer = obj;
}

sf::Vector2f GameController::getBasePos(int team) {
    vector<sf::Vector2f> posList;
    for(CFGameObject* o : CFGameObject::gameObjects){
        if(o == nullptr || o->type() != GAMEOBJ_BASE) continue;
        if( ((MapObj_Base*)o)->team == team ) posList.push_back({o->x(), o->y()});
    }
    return posList.at(rand() % posList.size());
}

void GameController::scoreTeam(int team) {
    gameScores[team]++;
}

void GameController::step(sf::Time &delta){
    gameSync++;
    if((MyGame::current().window().hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
        || !MainMenu::localClient->isConnected() || gameEnd){
        delete this;
        new MainMenu;
        return;
    }
    if((MyGame::current().window().hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::F4))){
        toggleFullscreen();
        return;
    }
    if(viewFollows != nullptr){
        View& v = MyGame::current().view();
        const sf::Vector2f& center = v.getCenter();
        sf::Vector2f pos(viewFollows->x(), viewFollows->y());
        sf::Vector2f to = (pos - center) / float(10);

        v.move(to);
    }
    if(startTimer.getElapsedTime().asSeconds() > GAMESTART_TIME) {
        if(!gameStarted){   //begin the game if it hasn't started
            gameStarted = true;
            gameBegin();
        } else {
            if(getGameTimeLeft() == 0) {
                int winner[2] = {0, -1}; /// { Highest Score Points ,  Team That Is In Lead }
                for(int i=0;i<gameScores.size();++i){
                    if(gameScores[i] > winner[0]){
                        winner[0] = gameScores[i];
                        winner[1] = i;
                    }
                    if(gameScores[i] == winner[0] && i != winner[1]){
                        winner[1] = -1;
                    }
                }
                string text;
                if(winner[1] == -1){
                    text = "Game Ended In A Draw";
                } else {
                    text = MyGame::teamName(winner[1]) + " Won The Round";
                }
                endGame();

                MainMenu::messageBox = new MsgBox({text, "Game Over", MB_ICONINFORMATION | MB_TASKMODAL});
            }
        }
        startTimer.restart();
    }
    if(isHost){
        if(spawnBonusTimer.getElapsedTime().asSeconds() > settings.bonusSpawnTime){
            float xx, yy;
            do {
                xx = float(gameArea.left + rand() % int(gameArea.width*10)) / 10.f;
                yy = float(gameArea.top + rand() % int(gameArea.height*10)) / 10.f;
            } while(CFGameObject::grid->getStatic(xx / 8, yy / 8));
            MplayController::spawnBonus(xx, yy, spawnBonus(xx, yy));
            spawnBonusTimer.restart();
        }
    }
}

void GameController::drawWorldFront(sf::RenderWindow& win, sf::Time& delta) {
    Engine::View &v = MyGame::current().view();

    #ifdef DEBUG_MODE
    if(CFGameObject::grid != nullptr){
        PathGrid &grid = *CFGameObject::grid;
        for(int y=0; y<grid.getHeight(); ++y){
            for(int x=0; x<grid.getWidth(); ++x){
                if(grid.getStatic(x, y)){
                    sf::RectangleShape box({8, 8});
                    box.setPosition(x*8, y*8);
                    box.setFillColor(sf::Color(255,0,0,128));
                    win.draw(box);
                }
            }
        }
    }
    for(CFGameObject *o : CFGameObject::gameObjects){
        if(o == nullptr || o->type() != GAMEOBJ_AI) continue;
        MapObj_AI &p = *(MapObj_AI*)o;
        if(p.xto != -1 && p.yto != -1){
            sf::CircleShape point(4);
            point.setPosition(p.xto, p.yto); // AI next step

            point.setFillColor(sf::Color(0,255,0,128));
            win.draw(point);
        }
    }
    for(CFGameObject *o : CFGameObject::gameObjects){
        if(o == nullptr || (o->type() != GAMEOBJ_PLAYER && o->type() != GAMEOBJ_AI)) continue;
        MapObj_Player &p = *(MapObj_Player*)o;
        if(p.myPath != nullptr){
            sf::CircleShape point(4);
            int xx, yy; p.myPath->getDestination(xx, yy); // AI goal
            point.setPosition(xx*8, yy*8);
            point.setFillColor(sf::Color(0,0,255,128));
            win.draw(point);
        }
    }
    #endif

    if(playerControlled != nullptr){
        MapObj_Player& p = *(MapObj_Player*)playerControlled;
        sf::Text playerStats;
        playerStats.setFont(*MyGame::fontPack->getFont(FNT_MAIN));
        playerStats.setCharacterSize(10);
        playerStats.setFillColor(sf::Color::Yellow);
        playerStats.setString(string("Life: ") + to_string(p.life) + "\nBonus: " + getBonusName(p.bonusType));
        playerStats.setPosition(p.x(), p.y() - 40);
        win.draw(playerStats);
    }
    if(!gameStarted){
        sf::RectangleShape shape(v.getSize());
        shape.setPosition((v.getCenter() - (v.getSize() / 2.0f)));
        shape.setFillColor(sf::Color::Black);
        win.draw(shape);
    } else {
        sf::RectangleShape bar({v.getSize().x, 32});
        bar.setPosition(v.getCenter() - (v.getSize() / 2.0f));
        bar.setFillColor(sf::Color(32,32,32));

        sf::Text barText;
        barText.setFont(*MyGame::fontPack->getFont(FNT_MAIN));
        barText.setCharacterSize(10);
        barText.setFillColor(sf::Color::White);
        barText.setString("Red Team Score: " + to_string(gameScores[TEAM_RED]) + " Blue Team Score: " + to_string(gameScores[TEAM_BLUE]) + "     Game Time: " + to_string(getGameTimeLeft()));
        barText.setPosition(bar.getPosition().x + 64, bar.getPosition().y + 16);

        win.draw(bar);
        win.draw(barText);
    }
}

void GameController::drawWorldBack(sf::RenderWindow& win, sf::Time& delta) {
    for(AnimatedSprite* s : background) win.draw(*s);
}
