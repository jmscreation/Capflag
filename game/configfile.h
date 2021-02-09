#ifndef CONFIGFILE_H
#define CONFIGFILE_H


struct Entry {
    std::string name, value;
    inline void clear() { name.clear(); value.clear(); }
};

class ConfigFile {
    std::fstream file;
    const std::string filePath;
    std::vector<Entry> entryList;

    public:
        ConfigFile(const std::string& path);
        virtual ~ConfigFile();

        bool entryExists(const std::string& name) const;
        const Entry* getEntry(const std::string& name) const;
        bool addEntry(const Entry& entry);
        void deleteEntry(const std::string& name);
        const std::vector<Entry>& getEntries() const { return entryList; }
    private:
        void dbAddEntry(const Entry& entry);
        bool syncFileIn();
        bool syncFileOut();
};

#endif // CONFIGFILE_H
