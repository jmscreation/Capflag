#include "includes.h"

using namespace Engine;
using std::cout, std::endl;

GameController* GameController::currentController = nullptr;
GameSettings GameController::settings;

GameController& GameController::current() {
    if(currentController == nullptr) currentController = new GameController;
    return *currentController;
}

GameController::GameController() {
    windowedMode = sf::VideoMode(640, 480);
    const std::vector<sf::VideoMode>& modes = sf::VideoMode::getFullscreenModes();
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
    music->volume(float(MainMenu::musicVolume) / 100.f);

    playerControlled = nullptr;
    viewFollows = nullptr;
    gameScores = {0,0};
    gameScores[TEAM_RED] = 0;
    gameScores[TEAM_BLUE] = 0;

    // background tiles
    for(int i=0; i < 10500; i++){
        AnimatedSprite* spr = new AnimatedSprite(&MyGame::gameAnimations->Background);
        background.push_back(spr);
        spr->setPosition(float(i % 100)*32 - 480, float(i/100)*32 - 480);
    }
    gameArea = {4096,4096,0,0};
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

    CFGameObject::grid = new PathGrid((gameArea.left + gameArea.width) / AI_GRID_SIZE + 1, (gameArea.top + gameArea.height) / AI_GRID_SIZE + 1);
    PathGrid &grid = *CFGameObject::grid;

    for(CFGameObject* o : CFGameObject::gameObjects){
        if(o == nullptr || o->type() != GAMEOBJ_WALL) continue;
        int maskSize = o->mask->rect.w;
        int part = maskSize / AI_GRID_SIZE;
        int xx = o->x() / AI_GRID_SIZE - part / 2, yy = o->y() / AI_GRID_SIZE - part / 2;
        grid.addStatic(xx, yy, part, part);
    }
}

GameController::~GameController() {
    music->stop(true);
    delete inputCont;
    delete CFGameObject::grid;
    for(AnimatedSprite* s : background) delete s;

    std::vector<CFGameObject*> temp = CFGameObject::gameObjects;
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
    std::vector<sf::Vector2f> posList;
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
    if(startTimer.getElapsedTime().asSeconds() > settings.gameStartTime) {
        if(!gameStarted){   //begin the game if it hasn't started
            gameStarted = true;
            gameBegin();
        } else {
            if(getGameTimeLeft() == 0 || gameEnd) {
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
                std::string text;
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
        if(settings.autoAiSpawner){
            auto MakeEnemyFollow = [&](MapObj_AI* ai){
                for(CFGameObject* o : CFGameObject::gameObjects){
                    if(o == nullptr || (o->type() != GAMEOBJ_PLAYER && o->type() != GAMEOBJ_AI)) continue;
                    MapObj_Player* player = (MapObj_Player*)o;
                    if(player->team == ai->team || player->alpha < 10) continue;

                    if(!(rand() % 4)){
                        ai->follow = player;
                        break;
                    }
                };
            };

            auto SpawnAiEnemy = [&](){
                sf::Vector2f pos(getBasePos(TEAM_RED));
                MapObj_AI* p = new MapObj_AI(pos.x, pos.y, TEAM_RED);
                MplayController::spawnPlayer(p);
                MakeEnemyFollow(p);
            };

            if(spawnAiTimer.getElapsedTime().asSeconds() > settings.respawnTime){
                SpawnAiEnemy();

                spawnAiTimer.restart();
            }

            for(CFGameObject* o : CFGameObject::gameObjects){
                if(o == nullptr || o->type() != GAMEOBJ_AI) continue;
                MapObj_AI* ai = (MapObj_AI*)o;
                if(ai->team != TEAM_RED || ai->follow != nullptr) continue;

                MakeEnemyFollow(ai);
            }
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
                    sf::RectangleShape box({AI_GRID_SIZE, AI_GRID_SIZE});
                    box.setPosition(x * AI_GRID_SIZE, y*AI_GRID_SIZE);
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
        sf::CircleShape fakeP(4);
        fakeP.setPosition(p.fakeX, p.fakeY); // AI next step
        fakeP.setFillColor(sf::Color(255,0,255,128));
        win.draw(fakeP);
    }
    for(CFGameObject *o : CFGameObject::gameObjects){
        if(o == nullptr || (o->type() != GAMEOBJ_PLAYER && o->type() != GAMEOBJ_AI)) continue;
        MapObj_Player &p = *(MapObj_Player*)o;
        if(p.myPath != nullptr){
            sf::CircleShape point(4);
            int xx, yy; p.myPath->getDestination(xx, yy); // AI goal
            point.setPosition(xx*AI_GRID_SIZE, yy*AI_GRID_SIZE);
            point.setFillColor(sf::Color(0,0,255,128));
            win.draw(point);
        }
    }

    for(CFGameObject *o : CFGameObject::gameObjects){
        if(o == nullptr || (o->type() != GAMEOBJ_PLAYER && o->type() != GAMEOBJ_AI)) continue;
        
        MapObj_Player &p = *(MapObj_Player*)o;
        if(p.mask == nullptr) continue;

        if(p.mask->type == MASK_CIRCLE){
            sf::CircleShape point(p.mask->circle.radius);
            point.setPosition(p.mask->x - p.mask->xorig, p.mask->y - p.mask->yorig);
            point.setFillColor(sf::Color(0,255,0,128));
            win.draw(point);
        }
        if(p.mask->type == MASK_RECT){
            sf::RectangleShape box({p.mask->rect.w, p.mask->rect.h});
            box.setPosition(p.mask->x - p.mask->xorig, p.mask->y - p.mask->yorig);
            box.setFillColor(sf::Color(0,255,0,128));
            win.draw(box);
        }
    }
    #endif

    if(playerControlled != nullptr){
        MapObj_Player& p = *(MapObj_Player*)playerControlled;
        sf::Text playerStats;
        playerStats.setFont(*MyGame::fontPack->getFont(FNT_MAIN));
        playerStats.setCharacterSize(10);
        playerStats.setFillColor(sf::Color::Yellow);
        playerStats.setString(std::string("Life: ") + std::to_string(p.life) + "\nBonus: " + getBonusName(p.bonusType));
        playerStats.setPosition(p.x(), p.y() - 40);
        win.draw(playerStats);
    }
    if(!gameStarted){
        sf::RectangleShape shape(v.getSize());
        shape.setPosition((v.getCenter() - (v.getSize() / 2.0f)));
        shape.setFillColor(sf::Color::Black);
        win.draw(shape);
        sf::Text countDown;
        countDown.setFont(*MyGame::fontPack->getFont(FNT_MAIN));
        countDown.setCharacterSize(48);
        countDown.setFillColor(sf::Color::White);
        countDown.setPosition(v.getCenter() - sf::Vector2f(48.f,48.f));
        countDown.setString(std::to_string( uint32_t(double(settings.gameStartTime) - startTimer.getElapsedTime().asSeconds() + 1.0) ));
        win.draw(countDown);
    } else {
        sf::RectangleShape bar({v.getSize().x, 32});
        bar.setPosition(v.getCenter() - (v.getSize() / 2.0f));
        bar.setFillColor(sf::Color(32,32,32));

        sf::Text barText;
        barText.setFont(*MyGame::fontPack->getFont(FNT_MAIN));
        barText.setCharacterSize(10);
        barText.setFillColor(sf::Color::White);
        barText.setString("Red Team Score: " + std::to_string(gameScores[TEAM_RED]) + " Blue Team Score: " + std::to_string(gameScores[TEAM_BLUE]) + "     Game Time: " + std::to_string(getGameTimeLeft()));
        barText.setPosition(bar.getPosition().x + 64, bar.getPosition().y + 16);

        win.draw(bar);
        win.draw(barText);
    }
}

void GameController::drawWorldBack(sf::RenderWindow& win, sf::Time& delta) {
    for(AnimatedSprite* s : background) win.draw(*s);
}
