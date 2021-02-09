#ifndef __MASK_H__
#define __MASK_H__

#define MASK_CIRCLE 1
#define MASK_RECT 2

#define _MASK_TEST_CIRCLECIRCLE 1
#define _MASK_TEST_RECTRECT 2
#define _MASK_TEST_CIRCLERECT 3

namespace Engine {

    class Mask {
    public:
        Mask(int type,float x,float y);
        virtual ~Mask();
        
        void initCircle(float radius,float xorig=0,float yorig=0);
        void initRect(float w,float h,float xorig=0,float yorig=0);

        bool collidesWith(Mask& m);

        int type;
        float x,y, xorig,yorig;

        union {
            struct {
                float radius;
            } circle;

            struct {
                float w,h;
            } rect;
        };
    };

}

#endif