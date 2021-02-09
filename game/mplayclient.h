#ifndef MPLAYCONFIG_H
#define MPLAYCONFIG_H


class MplayClient : public Engine::Active {
public:
    MplayClient();
    virtual ~MplayClient();

    void connectToHost(sf::IpAddress ip, unsigned short port);
    inline bool getErrorCount() { return errors.size(); }
    inline const std::string getLastError() { std::string msg; msg = errors.back(); errors.pop_back(); return msg; }
    inline bool isConnected() { return connected; }
    inline bool isConnecting() { return connecting; }
    inline sf::IpAddress getIp() { return client.getRemoteAddress(); }
    inline unsigned int getId() { return myID; }

    inline sf::Packet& getData() { return in; }
    inline void sendData(sf::Packet& data) { out.append(data.getData(), data.getDataSize()); }
    BufferContext getBufferContext();

    static void printPacket(sf::Packet& buf);

    void disconnect();

protected:
    sf::TcpSocket client;
    bool connecting, connected;

    inline void setError(const std::string& msg) { errors.push_back(msg); }

    void step(sf::Time&);

private:
    sf::Packet in, out;
    unsigned int myID;
    std::vector<std::string> errors;
};

#endif // MplayClient_H
