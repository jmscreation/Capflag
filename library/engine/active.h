#ifndef __ENGINE_ACTIVE_H__
#define __ENGINE_ACTIVE_H__

namespace Engine {

    class Active {
        static std::vector<Active*> list;
        static bool iterating;
    public:
        Active();
        virtual ~Active();

        virtual void step(sf::Time& delta) {}

        friend class App;
    };

    class ActiveEnd {
        static std::vector<ActiveEnd*> list;
        static bool iterating;
    public:
        ActiveEnd();
        virtual ~ActiveEnd();

        virtual void stepEnd(sf::Time& delta) {}

        friend class App;
    };

}

#endif // __ENGINE_ACTIVE_H__