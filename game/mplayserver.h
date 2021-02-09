#ifndef MPLAYSERVER_H
#define MPLAYSERVER_H


typedef struct {
    sf::TcpSocket* soc;
    sf::Socket::Status status;
    unsigned int id;
    bool connected;
    sf::Packet in, out;
} ClientData;

class MplayServer {
public:
    MplayServer();
    virtual ~MplayServer();
    static inline bool isRunning() { return (_currentServer != NULL); }
    static inline void closeServer() { if(_currentServer != NULL) delete _currentServer; }
    static inline unsigned int getClientCount() { if(_currentServer == NULL) return 0; return _currentServer->clientList.size(); }

protected:
    sf::TcpListener server;

    sf::TcpSocket* tmpSoc;
    std::vector<ClientData> clientList;

    void step();

private:
    unsigned int clientCount;
    static MplayServer* _currentServer;
    std::atomic<bool> running;
    sf::Thread* handle;
    static void serverHandle(MplayServer* instance);
};

#endif // MPLAYSERVER_H
