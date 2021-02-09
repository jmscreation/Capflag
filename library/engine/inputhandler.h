#ifndef __ENGINE_INPUTHANDLER_H__
#define __ENGINE_INPUTHANDLER_H__

namespace Engine {

    namespace Input {

        class Window {
            static std::vector<Window*> list;
            static bool iterating;
        public:
            Window();
            virtual ~Window();

            virtual void windowResized(sf::Event::SizeEvent& evt) {}
            virtual void windowGainFocus() {}
            virtual void windowLostFocus() {}
            virtual void windowTextEntered(sf::Event::TextEvent& evt) {}

            friend class Engine::App;
        };

        class Keyboard {
            static std::vector<Keyboard*> list;
            static bool iterating;
            static unsigned char keysDown[sf::Keyboard::Key::KeyCount];
        public:
            Keyboard();
            virtual ~Keyboard();

            static bool isKeyDown(int ky);

            virtual void keyPress(sf::Event::KeyEvent& evt) {}
            virtual void keyHold(sf::Event::KeyEvent& evt) {}
            virtual void keyRelease(sf::Event::KeyEvent& evt) {}

            friend class Engine::App;
        };

        class Mouse {
            static std::vector<Mouse*> list;
            static bool iterating;
            static unsigned char buttonsDown[sf::Mouse::Button::ButtonCount];
            static int _x, _y;
        public:
            Mouse();
            virtual ~Mouse();

            static bool isMouseButtonDown(int btn);
            static int mouseX();
            static int mouseY();

            virtual void mouseButtonPress(sf::Event::MouseButtonEvent& evt) {}
            virtual void mouseButtonHold(sf::Event::MouseButtonEvent& evt) {}
            virtual void mouseButtonRelease(sf::Event::MouseButtonEvent& evt) {}
            virtual void mouseMove(sf::Event::MouseMoveEvent& evt) {}
            virtual void mouseWheel(sf::Event::MouseWheelScrollEvent& evt) {}

            friend class Engine::App;
            friend class Engine::View;
        };

        class Touch {
            static std::vector<Touch*> list;
            static bool iterating;
        public:
            Touch();
            virtual ~Touch();

            virtual void touchBegin(sf::Event::TouchEvent& evt) {}
            virtual void touchMove(sf::Event::TouchEvent& evt) {}
            virtual void touchEnd(sf::Event::TouchEvent& evt) {}

            friend class Engine::App;
        };

        class Joystick {
            static std::vector<Joystick*> list;
            static bool iterating;
        public:
            Joystick();
            virtual ~Joystick();

            virtual void joystickConnect(sf::Event::JoystickConnectEvent& evt) {}
            virtual void joystickDisconnect(sf::Event::JoystickConnectEvent& evt) {}
            virtual void joystickMove(sf::Event::JoystickMoveEvent& evt) {}
            virtual void joystickButtonPress(sf::Event::JoystickButtonEvent& evt) {}
            virtual void joystickButtonRelease(sf::Event::JoystickButtonEvent& evt) {}

            friend class Engine::App;
        };

    }

}

#endif // __ENGINE_INPUTHANDLER_H__
