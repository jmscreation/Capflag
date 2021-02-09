#ifndef __ENGINE_RESOURCES_H__
#define __ENGINE_RESOURCES_H__

namespace Engine {

    class FileResources {
        std::vector<std::stringstream*> package;
        bool success;
        std::stringstream* failed;
    public:
        FileResources();
        FileResources(const std::vector<const char*>& array);
        FileResources(const std::vector<int>& array);
        virtual ~FileResources();

        void load(const std::vector<const char*>& array);
        void load(const std::vector<int>& array);

        bool successful() { return success; }

        std::stringstream* failureSubstitute() { return failed; }
        bool failureSubstitute(const char* path);
        bool failureSubstitute(int resource);
        bool failureSubstitute(std::stringstream& str);

        std::stringstream* getFile(int ind);
    };

    class FontResources {
        std::vector<sf::Font*> package;
        bool success;
        sf::Font* failed;
    public:
        FontResources();
        FontResources(const std::vector<const char*>& array);
        FontResources(const std::vector<int>& array);
        virtual ~FontResources();

        void load(const std::vector<const char*>& array);
        void load(const std::vector<int>& array);

        bool successful() { return success; }

        sf::Font* failureSubstitute() { return failed; }
        bool failureSubstitute(const char* path);
        bool failureSubstitute(int resource);
        bool failureSubstitute(sf::Font& fnt);

        sf::Font* getFont(int ind);
    };

    class ImageResources {
        std::vector<sf::Image*> package;
        bool success;
        sf::Image* failed;
    public:
        ImageResources();
        ImageResources(const std::vector<const char*>& array);
        ImageResources(const std::vector<int>& array);
        virtual ~ImageResources();

        void load(const std::vector<const char*>& array);
        void load(const std::vector<int>& array);

        bool successful() { return success; }

        sf::Image* failureSubstitute() { return failed; }
        bool failureSubstitute(const char* path);
        bool failureSubstitute(int resource);
        bool failureSubstitute(sf::Image& img);

        sf::Image* getImage(int ind);
    };

    class TextureResources {
        std::vector<sf::Texture*> package;
        bool success;
        sf::Texture* failed;
    public:
        TextureResources();
        TextureResources(const std::vector<const char*>& array);
        TextureResources(const std::vector<int>& array);
        virtual ~TextureResources();

        void load(const std::vector<const char*>& array);
        void load(const std::vector<int>& array);

        bool successful() { return success; }

        sf::Texture* failureSubstitute() { return failed; }
        bool failureSubstitute(const char* path);
        bool failureSubstitute(int resource);
        bool failureSubstitute(sf::Image& img);

        sf::Texture* getTexture(int ind);
    };

    class AudioResources {
        std::vector<SoundBuffer*> package;
        bool success;
        SoundBuffer* failed;
    public:
        AudioResources();
        AudioResources(const std::vector<const char*>& array);
        AudioResources(const std::vector<int>& array);
        virtual ~AudioResources();

        void load(const std::vector<const char*>& array);
        void load(const std::vector<int>& array);

        bool successful() { return success; }

        SoundBuffer* failureSubstitute() { return failed; }
        bool failureSubstitute(const char* path);
        bool failureSubstitute(int resource);
        SoundBuffer* getSound(int ind);
    };

}

#endif // __ENGINE_RESOURCES_H__
