#include "includes.h"

using namespace std;

ConfigFile::ConfigFile(const string& path): filePath(path), entryList({}) {
    if(!syncFileIn()){
        if(!syncFileOut()) cerr << "error writing configuration" << endl;
    }
}

ConfigFile::~ConfigFile() {
}

bool ConfigFile::syncFileOut(){
    string data;
    remove(filePath.data()); // delete old config file
    file.open(filePath, ios::out | ios::binary);
    if(!file.is_open()) return false;

    for(const Entry& c : entryList){
        data = c.name + "\t" + c.value + "\n";
        file.write(data.data(), data.size());
    }
    file.close();
    return true;
}

bool ConfigFile::syncFileIn(){
    size_t sz;
    char* buffer;

    file.open(filePath, ios::in | ios::binary);
    if(!file.is_open()) return false;

    file.seekg(0, file.end);
    sz = file.tellg(); // file size
    file.seekg(0, file.beg);

    buffer = new char[sz];
    file.read(buffer, sz);
    file.close();

    entryList.clear(); // clear cache database

    int part = 0; // parsing first, whitespace, last
    Entry tempEntry; // temporary entry data

    for(size_t i=0;i<sz;++i){
        char c = buffer[i];
        if(c == '\r') continue;

        switch(part){
            case -1: // comment - note: comments will be erased when flushed back to disk
                if(c == '\n') part++;
                break;
            case 0: // name
                if(c == '\n'){
                    tempEntry.clear();
                    break;
                }
                if(c == '#' && !tempEntry.name.size()){ // first character is #
                    part = -1;
                    continue;
                }
                if(c == ' ' || c == '\t'){
                    part++; // no need to re-use character
                    continue;
                }
                tempEntry.name += c;
                break;
            case 1: // bypass all white-space -> start value collector
                if(c == ' ' || c == '\t') break; // skip more white-space
                i--; // re-use character for next part
                part++; // continue to value collector
                break;
            case 2: // value collector -> end of entry
                if(c == '\n'){
                    part = 0;
                    dbAddEntry(tempEntry);
                    tempEntry.clear();
                    break;
                }
                tempEntry.value += c;
                break;
        }
    }
    /** parse data:
        <name> <white-space> <value> <new-line>
    */
    delete[] buffer;
    return true;
}

void ConfigFile::dbAddEntry(const Entry& entry){
    for(Entry &e : entryList){
        if(e.name == entry.name){
            e.value = entry.value; // overwrite existing entry in cache database
            return;
        }
    }
    entryList.push_back(entry); // add entry to cache database
}

bool ConfigFile::entryExists(const string& name) const {
    for(const Entry& e : entryList) if(e.name == name) return true;
    return false;
}

bool ConfigFile::addEntry(const Entry& entry){
    if(entryExists(entry.name)) return false;
    entryList.push_back(entry);

    return syncFileOut();
}

const Entry* ConfigFile::getEntry(const string& name) const {
    for(const Entry& e : entryList){
        if(e.name == name) return &e; // return entry address
    }
    return nullptr; // not found - return null
}

void ConfigFile::deleteEntry(const string& name){
    bool changed = false;
    for(size_t i=0; i<entryList.size(); ++i){
        const Entry& e = entryList[i];
        if(e.name == name){
            entryList.erase(entryList.begin() + i);
            changed = true;
            break;
        }
    }
    if(changed) syncFileOut();
}

