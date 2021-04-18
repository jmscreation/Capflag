#include "includes.h"

using namespace Engine;
using namespace std;

MainMenu* MainMenu::_this = NULL;

const string MainMenu::uniqueID = sha1(to_string(int(time(NULL)) + rand()));
MplayClient* MainMenu::localClient = new MplayClient;
ServerSearch* MainMenu::searchController = new ServerSearch;
ServerAnnounce* MainMenu::broadcastController = new ServerAnnounce(MainMenu::uniqueID);
unsigned int MainMenu::musicVolume = 100;

int MainMenu::currentTeam = TEAM_BLUE;
string MainMenu::playerName = "";
MsgBox* MainMenu::messageBox = nullptr;
ImportMap* MainMenu::currentLevel = nullptr;

MainMenu::MainMenu(): currentTime(GameController::settings.gameTime) {
    assert(_this == NULL); _this = this;
    MyGame::current().window().create(sf::VideoMode(256, 256), "Main Menu", sf::Style::Titlebar);
    view().reset({0,0,256,256});
    MyGame::current().window().setFramerateLimit(45);

    gameStatus = "Finding the best server to connect to...";

    menuTitle.setFont(*MyGame::fontPack->getFont(FNT_MENU));
    menuTitle.setCharacterSize(12);
    menuTitle.setPosition(32, 32);
    menuTitle.setFillColor(sf::Color::White);

    menuError.setFont(*MyGame::fontPack->getFont(FNT_ERROR));
    menuError.setCharacterSize(8);
    menuError.setPosition(20, 170);
    menuError.setFillColor(sf::Color::Red);
    menuError.setOrigin(0, 0);

    sf::IpAddress addr(sf::IpAddress::getLocalAddress());

    if(currentLevel == nullptr) currentLevel = new ImportMap(true);

    do { /// Load Config File - Allow Looping Until Satisfied

        string dir = MyGame::current().parameters()[0];
        dir = dir.erase(dir.find_last_of("\\"));

        ConfigFile cfg(dir + "\\" + CONFIG_FILE);

		cout << "Loading configuration file..." << endl;

		for(const Entry& e : cfg.getEntries()){
			cout << "\t" << e.name << ":" << e.value << "\n";
		}

        if(cfg.entryExists("ip")){
            addr = cfg.getEntry("ip")->value;
        } else {
            cfg.addEntry({"ip", addr.toString()});
        }

        if(cfg.entryExists("auto-server")){
            string v = cfg.getEntry("auto-server")->value;
            for(char &c : v) c = tolower(c);
            if(v == "true"){
                autoConnect = true;
            } else if(v == "false"){
                autoConnect = false;
            } else {
                autoConnect = true;
                cfg.deleteEntry("auto-server");
                cfg.addEntry({"auto-server", "true"});
            }
        } else {
            autoConnect = true;
            cfg.addEntry({"auto-server", "true"});
        }

        if(cfg.entryExists("name")){
            playerName = cfg.getEntry("name")->value;
        } else {
            playerName = Dialog::EnvironmentVariable("username");
            cfg.addEntry({"name", playerName});
        }

		if(cfg.entryExists("music")){
            try {
				musicVolume = stoi(cfg.getEntry("music")->value);
            } catch(const exception& e){
                cfg.deleteEntry("music"); // invalid entry
                continue;
            }
        } else {
            if(cfg.addEntry({"music", to_string(musicVolume)}))
                continue;
        }

        if(cfg.entryExists("gametimer")){
            try {
                GameController::settings.gameTime = stoi(cfg.getEntry("gametimer")->value) ;
            } catch(const exception& e){
                cfg.deleteEntry("gametimer"); // invalid entry
                continue;
            }
        } else {
            if(cfg.addEntry({"gametimer", "120"}))
                continue;
        }


        if(cfg.entryExists("magsize")){
            try {
                GameController::settings.magSize = stoi(cfg.getEntry("magsize")->value) ;
            } catch(const exception& e){
                cfg.deleteEntry("magsize"); // invalid entry
                continue;
            }
        } else {
            if(cfg.addEntry({"magsize", "25"}))
                continue;
        }

        if(cfg.entryExists("tm_respawn")){
            try {
                GameController::settings.respawnTime = stoi(cfg.getEntry("tm_respawn")->value) ;
            } catch(const exception& e){
                cfg.deleteEntry("tm_respawn"); // invalid entry
                continue;
            }
        } else {
            if(cfg.addEntry({"tm_respawn", "8"}))
                continue;
        }

        if(cfg.entryExists("tm_invisible")){
            try {
                GameController::settings.bonusDuration[BONUS_INVISIBLE] = stoi(cfg.getEntry("tm_invisible")->value) ;
            } catch(const exception& e){
                cfg.deleteEntry("tm_invisible"); // invalid entry
                continue;
            }
        } else {
            if(cfg.addEntry({"tm_invisible", "12"}))
                continue;
        }

        if(cfg.entryExists("tm_speed")){
            try {
                GameController::settings.bonusDuration[BONUS_SPEED] = stoi(cfg.getEntry("tm_speed")->value) ;
            } catch(const exception& e){
                cfg.deleteEntry("tm_speed"); // invalid entry
                continue;
            }
        } else {
            if(cfg.addEntry({"tm_speed", "15"}))
                continue;
        }

        if(cfg.entryExists("tm_bonus")){
            try {
                GameController::settings.bonusSpawnTime = stoi(cfg.getEntry("tm_bonus")->value) ;
            } catch(const exception& e){
                cfg.deleteEntry("tm_bonus"); // invalid entry
                continue;
            }
        } else {
            if(cfg.addEntry({"tm_bonus", "10"}))
                continue;
        }

        break; // don't loop at end
    } while(1);

    connectTo = addr;
    isHost = MplayServer::isRunning();
    connected = false;
    if(!autoConnect){
        broadcastController->broadcasting = false;
    }

    if(messageBox != nullptr){
        MessageBox(MyGame::current().window().getSystemHandle(), messageBox->text.data(), messageBox->title.data(), messageBox->flags);
        delete messageBox;
        messageBox = nullptr;
    }
}

MainMenu::~MainMenu() {
    _this = nullptr;
}

void MainMenu::freeMemory(){
    delete broadcastController;
    delete searchController;
    delete localClient;
    if(currentLevel != nullptr){
        delete currentLevel;
        currentLevel = nullptr;
    }
}

void MainMenu::startGame() {    //Server starts game
    if(currentLevel->loadGame()) {
        cout << "Mplay controller create..." << endl;
        MplayController::mplayBegin(localClient->getBufferContext());
        GameController& c = GameController::current();
        c.mplayID = localClient->getId();
        c.myTeam = currentTeam;
        cout << "Exiting main menu..." << endl;
        delete this;
        return;
    } else {
        errorMessage = string("Error loading map -> \n") + currentLevel->getLastError();
    }
}

void MainMenu::sendMap(){
    if(!currentLevel->getContent().str().size()) return;
    cout << "Sending Map..." << endl;
    sf::Packet buf;
    buf << sf::Uint8(CMD_SEND_MAP) << currentLevel->getContent().str();
    cout << sha1(currentLevel->getContent().str()) << endl;
    localClient->sendData(buf);
}

void MainMenu::sendCommand(sf::Uint8 msg){
    sf::Packet buf;
    buf << sf::Uint8(CMD_MESSAGE) << msg;
    localClient->sendData(buf);
}

void MainMenu::sendGameInfo() {
    sf::Packet buf;
    buf << sf::Uint8(CMD_GAME_INFO) << GameController::settings;
    localClient->sendData(buf);
}

void MainMenu::onConnect() {
    cout << "Connected to server successfully" << endl;
    if(!isHost){
        gameStatus = "Waiting for host to start...";
    }
    sf::Packet buf;
    buf << sf::Uint8(CMD_NEW_CLIENT) << playerName; //tell everyone that I connected
    localClient->sendData(buf);
}

void MainMenu::onDisconnect() {
    gameStatus = "Finding the best server to connect to...";
    cout << "Disconnected from server" << endl;
}

sf::Clock hold; // static global member for button keypress
void MainMenu::step(sf::Time &delta) {
    if(menuStart.getElapsedTime().asSeconds() < 1) return;
    menuTitle.setString(string("Main Menu\n\n" + gameStatus + "\nGame Level: ")
                        + currentLevel->getName() + "\nGame Time: " + to_string(currentTime)
                        + "\nYour Team: " + MyGame::teamName(currentTeam)
                        + "\nPress Enter to start\n\n" + message);
    /// Start The Game
    if(MyGame::current().window().hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)){
        if(hold.getElapsedTime().asSeconds() > 0.3){
            if(isHost && connected) {
                cout << "Starting game..." << endl;
                sendGameInfo();
                sendCommand(MSG_START);
                startGame();
                if(_this == nullptr) return;
            } else {
                errorMessage = "Cannot start game as client";
            }
            hold.restart();
        }
    }
    /// Exit Main Menu - Quit
    if(MyGame::current().window().hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)){
        MyGame::current().window().close();
        delete this;
        return;
    }
    /// Change Team
    if(MyGame::current().window().hasFocus() && (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))){
        if(hold.getElapsedTime().asSeconds() > 0.3){
            currentTeam = !currentTeam;
            hold.restart();
        }
    }
    /// Change Time
    if(MyGame::current().window().hasFocus() && isHost){
        bool keyUp = sf::Keyboard::isKeyPressed(sf::Keyboard::Up), keyDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Down);
        if(keyUp || keyDown){
            if(hold.getElapsedTime().asSeconds() > 0.3){
                if(keyUp && currentTime < 400) currentTime += 10;
                if(keyDown && currentTime > 10) currentTime -= 10;
                hold.restart();
                sendGameInfo();
            }
        }
    }
    /// Open Map File
    if(MyGame::current().window().hasFocus() && (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl))
        && sf::Keyboard::isKeyPressed(sf::Keyboard::O) && isHost){
            string path;
            Dialog::LoadDialog(path, "C:\\Users\\Shock\\Desktop\\GM Stuff\\Levels\\", {{"Map Files(*.map)", "*.map"}}, 0, 0, "Open Map File");
            if(!path.size()){
                errorMessage = "No File Selected\n";
                return;
            }
            if(!currentLevel->loadMap(path)){
                errorMessage = currentLevel->getLastError();
                return;
            }
            while(currentLevel->hasWarnings()) {
                 errorMessage += "\n" + currentLevel->getNextWarning();
            }
            sendMap();
        }

    if(errorMessage != menuError.getString() ){
        menuError.setString(errorMessage);
        //menuError.setOrigin(menuError.getLocalBounds().width - menuError.getLocalBounds().left, 0);
        resetError.restart();
    }

    if(errorMessage != "" && resetError.getElapsedTime().asSeconds() > 8) {
        errorMessage = "";
    }

    if((searchController->serverCount() || !autoConnect) && refreshRate.getElapsedTime().asSeconds() > 4) {   ///negotiate who should be host
        ServerSearch::ServerMember* main = nullptr;
        vector<ServerSearch::ServerMember> slist = searchController->serverList();
        for(ServerSearch::ServerMember& i : slist) {
            if(i.name == uniqueID) continue;
            if(main == nullptr || i.alive > main->alive){
                if(i.ip == connectTo && !localClient->isConnected() && timeoutCheck.getElapsedTime().asSeconds() > 15) continue;
                main = &i;
            }
        }

        if(autoConnect && (main == nullptr || broadcastController->getUpTime() > main->alive)) {
            connectTo = sf::IpAddress::getLocalAddress();
            if(!MplayServer::isRunning()) {
                cout << "Connect to local server because no older servers were found" << endl;
                localClient->disconnect();
                new MplayServer;
                isHost = true;
            }
        } else {
            MplayServer::closeServer();
            isHost = false;
            if(autoConnect){
                if(connectTo != main->ip){
                    localClient->disconnect();
                    timeoutCheck.restart();
                    cout << "Connect to older server that is currently available: " << main->ip.toString() << endl;
                }
                connectTo = main->ip;
            }
        }

        if(!localClient->isConnected() && !localClient->isConnecting()) {
            if(!autoConnect) cout << "Manual server connect:" << endl;
            message = string("Connecting to server ") + connectTo.toString() + "...\n";
            localClient->connectToHost(connectTo, GAME_PORT);
            //sf::sleep(sf::seconds(1));
        }
        refreshRate.restart();
    }

    if(localClient->getErrorCount()) message += localClient->getLastError() + "\n";
    if(localClient->isConnected()) {
        if(!connected){
            onConnect();
            connected = true;
        }
        //when connected
        message = string("Connected to a ") + (isHost ? "local" : "remote") + " server.";
        if(connectTo != localClient->getIp()) {
            localClient->disconnect();
            message = "Found a better server to connect to...";
        }
        static sf::Clock testClock;
        if(testClock.getElapsedTime().asSeconds() > 1){
            testClock.restart();
            //sf::Packet outData;
            if(isHost) {
                gameStatus = to_string(MplayServer::getClientCount()) + " client(s) connected...";
                //outData << sf::Uint8(CMD_ACTIVE) << string("Server Runtime " + to_string(floor(MyGame::gameRuntime.getElapsedTime().asSeconds())) );
                //localClient->sendData(outData);
            }
        }
        {
            sf::Packet inData;
            if(localClient->getData().getDataSize()){
                inData = localClient->getData();
                localClient->getData().clear();
            }
            static sf::Clock timeout;
            timeout.restart();
            while(!inData.endOfPacket()){
                if(timeout.getElapsedTime().asSeconds() > 8) break; //assert(false); //crash because of timeout
                sf::Uint8 cmd;
                inData >> cmd;
                if(cmd == CMD_GAME_DATA){
                    gameStatus = "Waiting for host to end current match...";
                    inData.clear();
                    break;
                }
                if(cmd == CMD_ACTIVE){
                    string msg;
                    inData >> msg;
                    errorMessage = msg;
                }
                if(cmd == CMD_MESSAGE){
                    sf::Uint8 msg;
                    inData >> msg;
                    switch(msg){
                        case MSG_START:
                            startGame();
                            if(_this == nullptr) return;
                        break;
                        case MSG_EXIT:{
                            if(!isHost){
                                gameStatus = "Waiting for host to start...";
                            } else {
                                errorMessage = "A player left the game!";
                            }
                            break;}
                        case MSG_INGAME:{
                            if(!isHost){
                                gameStatus = "Waiting for host to end current match...";
                            }
                            break;}
                    }
                }
                if(cmd == CMD_SEND_MAP){
                    cout << "Incoming map..." << endl;
                    string level;
                    inData >> level;
                    cout << sha1(level) << endl;
                    stringstream levelStream;
                    levelStream.str(level);
                    if(!currentLevel->loadMap(levelStream)){
                        cout << currentLevel->getLastError() << endl;
                        errorMessage = currentLevel->getLastError();
                        continue;
                    }
                    while(currentLevel->hasWarnings()) errorMessage += "\n" + currentLevel->getNextWarning();
                }
                if(cmd == CMD_GAME_INFO){
                    inData >> GameController::settings;
                }
                if(cmd == CMD_NEW_CLIENT){
                    string nam;
                    inData >> nam;
                    cout << nam << " joined the game" << endl;
                    errorMessage += nam + " connected\n";
                    if(isHost){
                        sendGameInfo();
                        sendMap();
                    }
                }
            }
        }
    } else {
        if(connected) {
            onDisconnect();
            connected = false;
        }
    }
}

void MainMenu::drawScreenBack(sf::RenderWindow &win, sf::Time &delta) {
    win.draw(menuTitle);
    win.draw(menuError);
}
