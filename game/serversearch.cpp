#include "includes.h"

using namespace Engine;
using namespace std;

ServerSearch::ServerSearch(): close(false), error(false) {
    foundServerList.reserve(8);
    Handle = new sf::Thread(&ServerSearch::SearchHandle, this);
    Handle->launch();
}

ServerSearch::~ServerSearch() {
    close = true;
    delete Handle;
}


int ServerSearch::serverCount(){
    sf::Lock lock(mutex);
    return foundServerList.size();
}

vector<ServerSearch::ServerMember> ServerSearch::serverList(){
    sf::Lock lock(mutex);
    return foundServerList;
}

//Main thread loop handle for finding the server
void ServerSearch::SearchHandle(){
    sf::UdpSocket socket;
    socket.setBlocking(false);
    sf::Packet buf;
    sf::IpAddress serverIp;
    unsigned short port = BROADCAST_PORT;

    sf::Socket::Status status = socket.bind(port);
    if(status != sf::Socket::Done){
        cout << "Failed to bind announcement port" << endl;
        error = true;
        socket.unbind();
        return;
    }
    while(!close){
        status = socket.receive(buf, serverIp, port);
        sf::sleep(sf::milliseconds(100));
        {   //Locked Mutex Scope
            mutex.lock();
            for(unsigned int i=0;i<foundServerList.size();++i){
                if(foundServerList[i].expire.getElapsedTime().asSeconds() > 4){
                    foundServerList.erase(foundServerList.begin() + i);
                    i--;
                }
            }
            mutex.unlock();
            if(close) break;

            if(status != sf::Socket::Done) continue;
            mutex.lock();
            sf::Int32 id, tm = 0;
            if(!(buf >> id) || id != GAME_ID) {
                cout << "Invalid Game ID" << endl;
                buf.clear();
                mutex.unlock();
                continue;
            }
            string name;
            if(!(buf >> name)) name = "<Unknown Server>";
            buf >> tm;

            bool found = false;
            for(ServerMember& m : foundServerList){
                if(m.ip == serverIp){
                    m.name = name;
                    m.alive = tm;
                    m.expire.restart();
                    found = true;
                    break;
                }
            }
            if(!found) foundServerList.emplace_back(ServerMember {name, serverIp, tm});
            mutex.unlock();
        }
    }
    socket.unbind();
}
