#ifndef IMPORTMAP_H
#define IMPORTMAP_H


class ImportMap
{
    public:
        ImportMap(bool=false);
        ImportMap(const std::string&, bool=false);
        virtual ~ImportMap();

        inline const char* getLastError() const { return lastError.data(); }
        inline bool errorOccurred() const { return hasError; }
        inline bool hasWarnings() const { return warningList.size(); }
        inline std::string getNextWarning() { if(warningList.size()) {std::string r = warningList.back(); warningList.pop_back(); return r;} return ""; }
        inline const std::vector<GameObjectType>& getObjectList() const { return objectList; }

        inline const std::stringstream& getContent() const { return content; }
        inline const std::string& getName() const { return mapName; }

        bool loadMap(const std::string&);
        bool loadMap(const std::stringstream&);
        bool loadGame();

    protected:
        std::stringstream content;
        unsigned long long contentLength;
        std::vector<std::string> warningList;
        std::vector<GameObjectType> objectList;
        std::string mapName;

    private:
        bool hasError, displayWarnings;
        std::string lastError;
        bool readFileData(const char* path);
        bool loadMapContent();
        inline bool setError(const std::string& msg) { lastError = msg; hasError = true; return false;}
        inline void addWarning(const std::string& msg) { warningList.push_back(msg); }


        bool read(unsigned long long& value){ content.read((char*)&value, sizeof(value)); return !content.bad(); }
        bool read(unsigned int& value){ content.read((char*)&value, sizeof(value)); return !content.bad(); }
        bool read(unsigned short& value){ content.read((char*)&value, sizeof(value)); return !content.bad(); }
        bool read(unsigned char& value){ content.read((char*)&value, sizeof(value)); return !content.bad(); }
        bool read(long long& value){ content.read((char*)&value, sizeof(value)); return !content.bad(); }
        bool read(int& value){ content.read((char*)&value, sizeof(value)); return !content.bad(); }
        bool read(short& value){ content.read((char*)&value, sizeof(value)); return !content.bad(); }
        bool read(char& value){ content.read((char*)&value, sizeof(value)); return !content.bad(); }
        bool read(std::string& value) {char c; do { content.read(&c, 1); if(content.bad()) return false; if(c) value += c;} while(c != 0); return !content.bad();}

};

#endif // IMPORTMAP_H
