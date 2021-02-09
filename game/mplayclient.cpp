#include "includes.h"

MplayClient::MplayClient() {
    connecting = false;
    client.setBlocking(false);
    myID = 0;
}

MplayClient::~MplayClient() {
}

BufferContext MplayClient::getBufferContext() {
    return {&in, &out};
}

void MplayClient::printPacket(sf::Packet& buf){
    std::cout << "Size: " << buf.getDataSize() << " Content: ";
    unsigned char c;
    for (unsigned int i=0; i <= buf.getDataSize(); c = *(((unsigned char*)buf.getData()) + i++)){
        std::cout << int(c) << ",";
    }
    std::cout << std::endl;
}

void MplayClient::step(sf::Time& delta) {

    if(connecting) {
        sf::Packet buf;
        sf::Socket::Status status = client.receive(buf);
        switch(status) {
            case sf::Socket::NotReady:
            case sf::Socket::Disconnected:  //still connecting
                return;
            case sf::Socket::Error:
                setError("Failed to connect");
                break;
            case sf::Socket::Done:{
                sf::Uint8 valid;
                sf::Uint32 id;
                buf >> valid >> id;
                if(valid == CMD_VALID_CLIENT){
                    connected = true;
                    std::cout << "My ID is " << id << std::endl;
                    myID = id;
                } else {
                    setError("Server is invalid");
                }
                break;
            }
        }
        connecting = false;
    }

    if(connected) {
        sf::Packet tmp;
        sf::Socket::Status status;
        do {
            status = client.receive(tmp);
            if(status == sf::Socket::Disconnected || status == sf::Socket::Error) {
                setError("Server disconnected unexpectedly (receive)");
                connected = false;
            }
            if(tmp.getDataSize()){
                in.append(tmp.getData(), tmp.getDataSize());
                tmp.clear();
            } else break;
        } while(1);

        if(out.getDataSize()) {
            //std::cout << "Send data" << std::endl;
            status = client.send(out);
            out.clear();
            if(status == sf::Socket::Disconnected || status == sf::Socket::Error) {
                setError("Server disconnected unexpectedly (send)");
                connected = false;
            }
        }
        if(!connected) std::cout << "something went wrong - socket disconnected" << std::endl;
    }
}

void MplayClient::connectToHost(sf::IpAddress ip, unsigned short port) {
    connecting = true;
    client.connect(ip, port, sf::seconds(10));
}

void MplayClient::disconnect() {
    if(!connected && !connecting) return;
    client.disconnect();
    connected = false;
    connecting = false;
}
