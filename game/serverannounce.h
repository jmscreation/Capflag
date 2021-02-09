#ifndef SERVERANNOUNCE_H
#define SERVERANNOUNCE_H


class ServerAnnounce {
    public:
        ServerAnnounce(const std::string& name);
        virtual ~ServerAnnounce();
        inline sf::Int32 getUpTime() { sf::Lock lock(mutex); return serverUpTime; }
        std::atomic<bool> broadcasting;

    private:
        std::string serverName;
        std::atomic<bool> close;
        sf::Thread* Handle;
        sf::Int32 serverUpTime;
        sf::Mutex mutex;

        void ServerAnnounceHandle();
};

#endif // SERVERANNOUNCE_H
