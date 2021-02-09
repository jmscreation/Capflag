#ifndef SERVERSEARCH_H
#define SERVERSEARCH_H


class ServerSearch {
    public:
        ServerSearch();
        virtual ~ServerSearch();

        struct ServerMember {
            std::string name;
            sf::IpAddress ip;
            sf::Int32 alive;
            sf::Clock expire;
        };

        int serverCount();
        bool searchError() { return error; }
        std::vector<ServerMember> serverList();

    private:

        sf::Thread* Handle;
        sf::Mutex mutex;
        std::vector<ServerMember> foundServerList;
        std::atomic<bool> close, error;

        void SearchHandle();
};

#endif // SERVERSEARCH_H
