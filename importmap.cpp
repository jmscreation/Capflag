#include "includes.h"

using namespace std;

ImportMap::ImportMap(bool showWarnings) {
    lastError = "";
    hasError = false;
    mapName = "";
    displayWarnings = showWarnings;
}

ImportMap::ImportMap(const string& path, bool showWarnings) {
    lastError = "";
    hasError = false;
    mapName = "";

    displayWarnings = showWarnings;
    loadMap(path);
}

ImportMap::~ImportMap() {
}

bool ImportMap::loadMap(const string& path) {
    lastError = "";
    hasError = false;
    warningList.clear();
    mapName = "";
    content.str(""); //clear the stream

    if(!readFileData(path.data())){
        setError("File doesn't exist or is corrupt");
        return false;
    }
    if(!loadMapContent()){
        setError(string("Map content is corrupt: ") + lastError);
        return false;
    }
    if(displayWarnings) for(string& msg : warningList) cout << "Warning: " << msg << endl;
    return true;
}

bool ImportMap::loadMap(const stringstream& data) {
    lastError = "";
    warningList.clear();
    hasError = false;
    mapName = "";
    content.str(data.str()); // copy data buffer
    contentLength = data.str().size();
    if(!loadMapContent()){
        setError(string("Map content is corrupt: ") + lastError);
        return false;
    }
    if(displayWarnings) for(string& msg : warningList) cout << "Warning: " << msg << endl;
    return true;
}

bool ImportMap::readFileData(const char* path) {
    ifstream file(path, ios::in | ios::binary);

    if(!file.is_open()) return false;

    file.seekg(0, file.end);
    auto size = file.tellg();
    file.seekg(0, file.beg);

    content.str(""); // clear buffer
    unsigned int bytesRead = 0;
    while(!file.eof() && bytesRead < size && file.tellg() != -1){
        char chk[128];
        file.read(chk, 128);
        bytesRead += file.gcount();
        content.write(chk, file.gcount());
    }
    file.close();
    contentLength = bytesRead;
    return (bytesRead == size);

}

bool ImportMap::loadMapContent() {
    unsigned long long isize, header;
    content.seekg(0, content.beg); // read from beginning of data stream
    objectList.clear();

    if(!read(isize)) return setError("Invalid Size"); // load size header
    if(contentLength != isize) return setError("Content Length Mismatch");

    read(header);
    if(header != FILE_HEADER) return setError("Bad Header");

    unsigned char crc, fileVersion;
    read(crc);
    if(crc == 255){
        read(fileVersion);
        switch(fileVersion){
            case 0: return setError("Cannot load a file version this old");
            case 1: addWarning("This is an old map version.\nPlease update this map to the latest version"); break;
        }
    }
    if(fileVersion > 0)
        read(mapName);
    if(fileVersion > FILE_VERSION) addWarning("This file version seems to be newer.\nThe file may not load properly.");
    bool complete = false;
    while(1){
        unsigned char cmd;
        if(!read(cmd)) return setError("Missing end of file!");
        switch(cmd){
            case 16:{
                complete = true;
            break;}
            case 90:{   //WALL
                unsigned short x, y;
                read(x);
                read(y);
                objectList.push_back({x, y, GAMEOBJ_WALL, 0});
            break;}
            case 91:{   //AI UNIT
                unsigned short x, y;
                unsigned char team;
                read(x);
                read(y);
                read(team);
                objectList.push_back({x, y, GAMEOBJ_AI, team});
            break;}
            case 92:{   //CUSTOM INSTANCE - Will Be Deprecated
                unsigned char objectType, team;
                unsigned short x, y;
                read(objectType);
                if(fileVersion >= 2){
                    read(team);
                    read(x);
                    read(y);
                    switch(objectType){
                        case GAMEOBJ_FLAG:
                        case GAMEOBJ_BASE:{
                            objectList.push_back({x, y, objectType, team});
                        break;}
                        default:{
                            addWarning(string("Bad object index: ") + to_string(objectType));
                        break;}
                    }
                } else {    ///Old version has a different object type for the different teams
                    read(x);
                    read(y);
                    switch(objectType){
                        case GAMEOBJ_BASE_A:
                            objectType = GAMEOBJ_BASE;
                        case GAMEOBJ_FLAG_A:{
                            if(objectType != GAMEOBJ_BASE) objectType = GAMEOBJ_FLAG;
                            objectList.push_back({x, y, objectType, TEAM_BLUE});
                        break;}
                        case GAMEOBJ_BASE_B:
                            objectType = GAMEOBJ_BASE;
                        case GAMEOBJ_FLAG_B:{
                            if(objectType != GAMEOBJ_BASE) objectType = GAMEOBJ_FLAG;
                            objectList.push_back({x, y, objectType, TEAM_RED});
                        break;}
                        default:{
                            addWarning(string("Bad object index: ") + to_string(objectType));
                        break;}
                    }
                }
            break;}
            default:{
                addWarning(string("Found non-existent object entry [") + to_string(cmd) + "]\nMap file is corrupt." );
            break;}
        }
        if(complete) break;
        unsigned char integrity;
        if(!read(integrity)) return setError("Integrity failed because the file ended unexpectedly");
        if(integrity != 0) return setError("Failed integrity check on object list");
    }
    unsigned long long _eof;
    if(!read(_eof)) addWarning("End of file invalid");
    if(_eof != 10) addWarning("End of file not written correctly");

    if(fileVersion < 2){    ///Old Version Manipulation - Do stuff because it's old

        //Fix offsets to 0,0
        unsigned int xMin = INT_MAX, yMin = INT_MAX;
        for(GameObjectType& obj : objectList){
            if(obj.x < xMin) xMin = obj.x;
            if(obj.y < yMin) yMin = obj.y;
        }
        for(GameObjectType& obj : objectList){
            obj.x -= xMin - 32;
            obj.y -= yMin - 32;
        }

        //The old game files do not include a 32w x 34h border of walls - add here if it's an old game file
        const unsigned int width=32, height=34;
        for(unsigned int x=0;x<width;x++){
            objectList.push_back({x*32, 0, GAMEOBJ_WALL, 0});
            objectList.push_back({x*32, 32*height, GAMEOBJ_WALL, 0});
        }
        for(unsigned int y=0;y<=height;y++){
            objectList.push_back({0, y*32, GAMEOBJ_WALL, 0});
            objectList.push_back({32*width, y*32, GAMEOBJ_WALL, 0});
        }
    }


    return true;

}


bool ImportMap::loadGame() {
    if(!objectList.size()) setError("Map not loaded");
    if(errorOccurred()) return false;

    for( const GameObjectType& obj : objectList){
        switch(obj.type){
            case GAMEOBJ_WALL: new MapObj_Wall(obj.x, obj.y); break;
            case GAMEOBJ_BASE:
                new MapObj_Base(obj.x, obj.y, obj.prop);  //base at pos with team prop
                break;
            case GAMEOBJ_FLAG:
                new MapObj_Flag(obj.x, obj.y, obj.prop); //flag at pos with team prop
                break;
            case GAMEOBJ_AI:
                if(GameController::current().isHost){
                    MapObj_AI* ii = new MapObj_AI(obj.x, obj.y, obj.prop); // new AI at pos with team prop
                    MplayController::spawnPlayer(ii);
                }
                break;
        }
    }
    return true;
}
