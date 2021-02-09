#ifndef __ENGINE_VIEW_H__
#define __ENGINE_VIEW_H__

namespace Engine {

    class View: public sf::View {
    public:
        View();
        View(const sf::FloatRect &r);
        View(const sf::Vector2f &c, const sf::Vector2f &s);
        virtual ~View();

        void screenToWorld(int sx,int sy,float &wx,float &wy);
        void worldToScreen(float wx,float wy,int &sx,int &sy);

        float viewMouseX();
        float viewMouseY();

        void zoomAt(int sx,int sy,float factor);

    private:
        void box(float& l,float& t,float& r,float& b);

        friend class App;
    };

}

#endif // __ENGINE_VIEW_H__
