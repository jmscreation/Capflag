#include "includes.h"

using namespace Engine;
using namespace std;

ServerAnnounce::ServerAnnounce(const string& name) : serverName(name), broadcasting(true) {
    Handle = new sf::Thread(&ServerAnnounce::ServerAnnounceHandle, this);
    Handle->launch();
}

ServerAnnounce::~ServerAnnounce() {
    close = true;
    delete Handle;
}


//Thread handle loop
void ServerAnnounce::ServerAnnounceHandle(){
    sf::UdpSocket socket;
    socket.setBlocking(false);
    /* socket.bind(0, sf::IpAddress::Any)); */
    sf::Packet buf;
    serverUpTime = MyGame::gameRuntime.getElapsedTime().asMilliseconds();

    unsigned short port = BROADCAST_PORT;
    cout << "Announcement Server Running..." << endl;
    while(!close){
        if(broadcasting){
            buf << sf::Int32(GAME_ID) << serverName << serverUpTime;
            if(socket.send(buf, sf::IpAddress::Broadcast, port) != sf::Socket::Done){
                cout << "Failed to broadcast server" << endl;
            }
            buf.clear();
        }
        {
            sf::Lock lock(mutex);
            serverUpTime = MyGame::gameRuntime.getElapsedTime().asMilliseconds();
        }
        sf::sleep(sf::seconds(2));
    }
    socket.unbind();
}
