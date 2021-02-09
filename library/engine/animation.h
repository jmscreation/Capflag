#ifndef __ENGINE_ANIMATION_H__
#define __ENGINE_ANIMATION_H__

namespace Engine {

    class Animation {
        struct Strip {
            int fr,len;
        };
        std::vector<Strip> animationFrames;
        sf::Texture* texture;
        int x,y,width,height,strip;
    public:
        Animation(sf::Texture* tex,std::vector<int>&& aniFrames,int x,int y,int imgWidth,int imgHeight,int stripWidth);
        virtual ~Animation();

        inline sf::Texture* getTexture() { return texture; }

        friend class AnimatedSprite;
    };

    class AnimatedSprite: public sf::Sprite {
        Animation* animator;
        int ani, img, delay, d;
    public:
        AnimatedSprite(Animation* a=nullptr);
        virtual ~AnimatedSprite();

        void updateAnimation(Animation* a);
        inline Animation* getAnimation() { return animator; }
        void animation(int anim, int frame=0);
        int animation();
        void frame(int frame);
        int frame();
        void next();
        void prev();

        void frameDelay(int delay);
        int frameDelay();
    };

}

#endif // __ENGINE_ANIMATION_H__
