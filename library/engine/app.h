#ifndef __ENGINE_APP_H__
#define __ENGINE_APP_H__

int main(int argc,char** argv);

namespace Engine {

    class BaseDrawable;

    class App {
        static App* currentApp;
    public:
        App(int width, int height, std::string title="", sf::Uint32 style=sf::Style::Default, bool defaultView=true);
        App(bool defaultView=true);
        virtual ~App();

        static App& current() { return *currentApp; }

        sf::RenderWindow& window() { return *win; }
        int views() { return viewStack.size(); }
        View& view(int ind=0);
        int addView(View* vw);
        void removeView(View* vw);
        int viewIndex(View* vw);
        int width() { return (int)win->getSize().x; }
        int height() { return (int)win->getSize().y; }
        void screenSize(int width, int height);
        bool setWindowTransparency(unsigned char alpha);

        sf::Time& delta() { return dt; }

        std::vector<std::string>& parameters();

        virtual void stepBefore(sf::Time& delta) {}
        virtual void stepAfter(sf::Time& delta) {}

    private:
        void run();

        static void setAppParams(int argc,char** argv);
        static std::vector<std::string> params;

        sf::Time dt;
        sf::RenderWindow* win;

        std::vector<View*> viewStack;

        SortList<BaseDrawable*> drawStack;

        sf::VideoMode videoMode;

        friend int ::main(int argc,char** argv);
        friend class BaseDrawable;
    };

}

#endif // __ENGINE_APP_H_
