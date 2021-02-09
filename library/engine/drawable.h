#ifndef __ENGINE_DRAWABLE_H__
#define __ENGINE_DRAWABLE_H__

#ifndef DRAWABLE_REGION_SIZE
#define DRAWABLE_REGION_SIZE 512
#endif

namespace Engine {

    class BaseDrawable {
        static ArbGrid<BaseDrawable*> grid;
    public:
        AnimatedSprite sprite;

        BaseDrawable(Animation* ani,float x,float y,float d=0);
        virtual ~BaseDrawable();

        float depth() { return _depth; }
        void depth(float d);

        virtual void draw(sf::RenderWindow& win,sf::Time& delta);

    private:
        float _depth;

        void move(float ox,float oy,float nx,float ny);
        void remove(float x,float y);

        friend class GameObject;
        friend class Drawable;
        friend class App;
    };

    class Drawable: public BaseDrawable {
    public:
        Drawable(Animation* ani,float x=0,float y=0,float depth=0);
        virtual ~Drawable();

        void move(float dx,float dy);
        void setPosition(float x,float y);
        void x(float x);
        void y(float y);
        float x() { return _x; }
        float y() { return _y; }

    private:
        float _x,_y;
    };

    class WorldDrawable {
        static SortList<WorldDrawable*> list;
    public:
        WorldDrawable(float d=0);
        virtual ~WorldDrawable();

        float depth() { return _depth; }
        void depth(float d);

        virtual void drawWorldBack(sf::RenderWindow& win,sf::Time& delta) {}
        virtual void drawWorldFront(sf::RenderWindow& win,sf::Time& delta) {}

    private:
        float _depth;

        friend class Engine::App;
    };

    class ScreenDrawable {
        static SortList<ScreenDrawable*> list;
    public:
        ScreenDrawable(View* vw=NULL,float d=0);
        virtual ~ScreenDrawable();

        float depth() { return _depth; }
        void depth(float d);

        View& view() { return *_view; }

        virtual void drawScreenBack(sf::RenderWindow& win,sf::Time& delta) {}
        virtual void drawScreenFront(sf::RenderWindow& win,sf::Time& delta) {}

    private:
        float _depth;
        View* _view;

        friend class Engine::App;
    };

}

#endif // __ENGINE_DRAWABLE_H__
