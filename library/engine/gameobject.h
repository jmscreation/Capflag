#ifndef __ENGINE_GAMEOBJECT_H__
#define __ENGINE_GAMEOBJECT_H__

namespace Engine {

    class GameObject: public BaseCollidable, public BaseDrawable {
    public:
        GameObject(Animation* ani,float x=0,float y=0,float depth=0);
        virtual ~GameObject();

        void move(float dx,float dy);
        void setPosition(float x,float y);
        void x(float x);
        void y(float y);
        float x() { return _x; }
        float y() { return _y; }

        virtual int type() = 0;

    private:

        float _x,_y;
    };

}

#endif // __ENGINE_GAMEOBJECT_H__
