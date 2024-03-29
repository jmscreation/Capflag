#ifndef MAINMENU_H
#define MAINMENU_H


struct MsgBox {
    std::string text, title;
    UINT flags;
};

class MainMenu : public Engine::Active , public Engine::ScreenDrawable {
    public:
        MainMenu();
        virtual ~MainMenu();

        void startGame();
        static bool getPlaySounds() { return playSounds; }

        static void freeMemory();
    protected:
        static MainMenu* _this;

        void step(sf::Time&);
        void drawScreenBack(sf::RenderWindow &win, sf::Time &delta);

        static ImportMap* currentLevel;
        static unsigned int musicVolume;
        static bool playSounds;
		static int currentTeam;
        static std::string playerName;
        static std::string loadMapFile;
        unsigned int& currentTime; // link directly to game settings

        sf::Text menuTitle, menuError;
        std::string message, errorMessage, gameStatus;
        sf::Clock resetError;
        void sendCommand(sf::Uint8 msg);
        void sendMap();
        void sendGameInfo();
        void reloadConfigurationData();

        virtual void onConnect();
        virtual void onDisconnect();

    private:
        static const std::string uniqueID;
        static MplayClient* localClient;
        static ServerSearch* searchController;
        static ServerAnnounce* broadcastController;
        static MsgBox* messageBox;
        sf::IpAddress connectTo, configIp;
        bool isHost, connected, autoConnect, tryOpen;
        sf::Clock refreshRate, menuStart, timeoutCheck;
        friend class GameController;
};

#endif // MAINMENU_H
