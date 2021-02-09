#ifndef __ENGINE_COLLIDABLE_H__
#define __ENGINE_COLLIDABLE_H__

#ifndef COLLIDABLE_REGION_SIZE
#define COLLIDABLE_REGION_SIZE 512
#endif

namespace Engine {

    class Mask;
    class BaseCollidable;

    class CellIterator {
        ArbGrid<BaseCollidable*>::CellIterator arbIter;

        friend class BaseCollidable;
    };
    class PositionIterator {
        ArbGrid<BaseCollidable*>::CellIterator arbIter;
        int cell,x,y;

        friend class BaseCollidable;
    };
    class AllIterator {
        ArbGrid<BaseCollidable*>::AllIterator arbIter;

        friend class BaseCollidable;
    };
    class RectIterator {
        ArbGrid<BaseCollidable*>::CellIterator arbIter;
        int cell, x,y,w,h;
        float x1,y1,x2,y2;

        friend class BaseCollidable;
    };

    class BaseCollidable {
        static ArbGrid<BaseCollidable*> grid;
        static CellIterator defaultCellIter;
        static PositionIterator defaultPosIter;
        static AllIterator defaultAllIter;
        static RectIterator defaultRectIter;
    public:
        BaseCollidable(float x,float y);
        virtual ~BaseCollidable();

        static void beginCell(float x,float y,CellIterator& iter = defaultCellIter);
        static BaseCollidable* iterateCell(CellIterator& iter = defaultCellIter);
        static void beginPosition(float x,float y,PositionIterator& iter = defaultPosIter);
        static BaseCollidable* iteratePosition(PositionIterator& iter = defaultPosIter);
        static void beginAll(AllIterator& iter = defaultAllIter);
        static BaseCollidable* iterateAll(AllIterator& iter = defaultAllIter);
        static void beginRect(float x1,float y1,float x2,float y2,RectIterator& iter = defaultRectIter);
        static BaseCollidable* iterateRect(RectIterator& iter = defaultRectIter);

        virtual int type() = 0;
        virtual void move(float dx,float dy) = 0;
        virtual void setPosition(float x,float y) = 0;
        virtual void x(float x) = 0;
        virtual void y(float y) = 0;
        virtual float x() = 0;
        virtual float y() = 0;
        void beginCollision(PositionIterator& iter = defaultPosIter);
        BaseCollidable* iterateCollidable(PositionIterator& iter = defaultPosIter);

        Mask* mask;

    private:
        void move(float ox,float oy,float nx,float ny);
        void remove(float x,float y);

        friend class App;
        friend class Collidable;
        friend class GameObject;
    };

    class Collidable: public BaseCollidable {
    public:
        Collidable(float x=0,float y=0);
        virtual ~Collidable();

        void move(float dx,float dy);
        void setPosition(float x,float y);
        void x(float x);
        void y(float y);
        float x() { return _x; }
        float y() { return _y; }
        void beginCollision();
        BaseCollidable* iterateCollidable();

        virtual int type() = 0;

    private:
        float _x,_y;
    };

}

#endif // __ENGINE_COLLIDABLE_H__