#include "includes.h"

using namespace std;
using namespace Engine;

MplayServer* MplayServer::_currentServer = nullptr;

MplayServer::MplayServer() {
    assert(_currentServer == nullptr);
    _currentServer = this;
    clientCount = 0;

    server.setBlocking(false);
    server.listen(GAME_PORT);
    cout << "Game Server Listening" << endl;

    tmpSoc = new sf::TcpSocket;

    running = true;
    handle = new sf::Thread(&serverHandle, this);
    handle->launch();
}

MplayServer::~MplayServer() {
    running = false;
    handle->wait();
    server.close();
    delete tmpSoc;
    _currentServer = nullptr;
    cout << "Game Server Closed" << endl;
}

void MplayServer::serverHandle(MplayServer* instance) {
    cout << "Server Handle Started" << endl;
    do {
        instance->step();
        sf::sleep(sf::milliseconds(3));
    } while(instance->running);
    cout << "Server Handle Closed" << endl;
}


void MplayServer::step() {

    static sf::Clock debugTimer;
    if(debugTimer.getElapsedTime().asSeconds() > 2) {
        debugTimer.restart();
    }

    if(server.accept(*tmpSoc) == sf::Socket::Done) {
        sf::Packet blank;
        tmpSoc->setBlocking(false);
        clientList.push_back({
            tmpSoc,
            sf::Socket::Done,
            clientCount++,
            true,
            blank, blank // copies
        });
        blank << sf::Uint8(CMD_VALID_CLIENT) << sf::Uint32(clientCount); //Important! This sends the client what ID they have
        tmpSoc->send(blank);
        tmpSoc = new sf::TcpSocket;
        cout << "New client joined [" << (clientCount) << "]" << endl;
    }

    for(ClientData& c : clientList) {
        c.in.clear();
        sf::Socket::Status status = c.soc->receive(c.in);
        if(status == sf::Socket::Error || status == sf::Socket::Disconnected){
            c.soc->disconnect();
            delete c.soc;
            c.connected = false;
        } else {
            if(c.out.getDataSize()){
                status = c.soc->send(c.out);
                c.out.clear();
                if(status == sf::Socket::Error || status == sf::Socket::Disconnected){
                    c.soc->disconnect();
                    delete c.soc;
                    c.soc = nullptr;
                    c.connected = false;
                }
            }
        }
        if(c.in.getDataSize()) {
            //cout << "Server received data" << endl;
        }
    }
    unsigned int sz = clientList.size();
    for(unsigned int i = 0; i < sz; ++i) {
        if(!clientList[i].connected){
            cout << "Client left" << endl;
            clientList.erase(clientList.begin() + i--);
            sz--;
        }
    }

    //Write all incoming data from all clients to the outgoing data of all other clients
    for(ClientData& inClient : clientList){
        if(!inClient.in.getDataSize()) continue; //bypass if client doesn't have data
        for(ClientData& outClient : clientList){
            if(&inClient == &outClient) continue;
            outClient.out.append(inClient.in.getData(), inClient.in.getDataSize());   //write the data
        }
    }
}
