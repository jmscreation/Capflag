#ifndef _ENGINE_GUI_H_
#define _ENGINE_GUI_H_

#define TOUCH_NONE 999

namespace Engine {

    namespace Gui {

        class Component;

        typedef bool (*Callback)(Engine::Gui::Component* _this, const sf::Vector2f&, sf::Mouse::Button btn);

        class Theme {
        public:
            struct ComponentTheme {
                sf::Color color_back, color_border, color_disabled, color_press;
                float border;
                ComponentTheme(const sf::Color& backgroundColor, const sf::Color& borderColor, const sf::Color& disabledColor, const sf::Color& pressColor, float borderWidth):
                    color_back(backgroundColor), color_border(borderColor), color_disabled(disabledColor), color_press(pressColor), border(borderWidth) {}
            };

            struct CheckboxTheme {
                sf::Vector2f size;
                sf::Color color_check;
                CheckboxTheme(const sf::Vector2f& size, const sf::Color& checkColor):
                    size(size), color_check(checkColor) {}
            };

            struct TextboxTheme {
                sf::Vector2f size;
                sf::Color color_text, color_cursor;
                sf::Font* font_text;
                unsigned int font_size;
                TextboxTheme(const sf::Vector2f& size, const sf::Color& textColor, const sf::Color& cursorColor, sf::Font* textFont, unsigned int sizeFont=0):
                    size(size), color_text(textColor), color_cursor(cursorColor), font_text(textFont), font_size(sizeFont) {}
            };

            struct ButtonTheme {
                sf::Vector2f size;
                sf::Color color_text;
                sf::Font* font_text;
                unsigned int font_size;
                ButtonTheme(const sf::Vector2f& size, const sf::Color& textColor, sf::Font* textFont, unsigned int sizeFont=0):
                    size(size), color_text(textColor), font_text(textFont), font_size(sizeFont) {}
            };

            Theme(const ComponentTheme& a, const CheckboxTheme& b, const TextboxTheme& c, const ButtonTheme& d);
            Theme(const Theme& a);
            virtual ~Theme();

            ComponentTheme themeComponent;
            CheckboxTheme themeCheckbox;
            TextboxTheme themeTextbox;
            ButtonTheme themeButton;
        };

        class Component : public Engine::ScreenDrawable, public Engine::Input::Mouse, public Engine::Input::Touch {
        public:
            Component(Theme&, Callback);
            virtual ~Component();
            void drawScreenFront(sf::RenderWindow &win, sf::Time &delta);
            void setPosition(const sf::Vector2f&);
            void setPosition(float, float);
            bool insideBox(const sf::Vector2f&);
            bool insideBox(float, float);
            sf::Vector2f getPosition();
            virtual void Disable(bool);
            bool Disabled() { return disabled; }
        protected:
            sf::FloatRect boundingBox;
            float x, y;
            bool disabled;
            const Theme currentTheme;
            virtual void draw(sf::RenderWindow &win) = 0;
            virtual void updatePosition() = 0;
            virtual void onDown(const sf::Vector2f&, sf::Mouse::Button) {}
            virtual void onUp(const sf::Vector2f&, sf::Mouse::Button) {}
            virtual void mousePress(const sf::Vector2f&, sf::Mouse::Button) {}
            virtual void mouseRelease(const sf::Vector2f&, sf::Mouse::Button) {}

        private:
            bool click;
            unsigned int touch;
            Callback callbackFunction;
            void mouseButtonPress(sf::Event::MouseButtonEvent &evt);
            void mouseButtonRelease(sf::Event::MouseButtonEvent &evt);
            void touchBegin(sf::Event::TouchEvent &evt);
            void touchEnd(sf::Event::TouchEvent &evt);
        };

        class Checkbox : public Component {
        public:
            Checkbox(Theme& theme, Callback = NULL);
            virtual ~Checkbox();
            void Check(bool);
            bool Check() { return checked; }

        private:
            sf::RectangleShape outside, check, overlay;
            void draw(sf::RenderWindow &win);
            void updatePosition();
            void onDown(const sf::Vector2f&, sf::Mouse::Button);
            void onUp(const sf::Vector2f&, sf::Mouse::Button);
            void mouseRelease(const sf::Vector2f&, sf::Mouse::Button);
            bool checked, clicking;
        };


        //Textbox
        class Textbox : public Component , public Engine::Input::Window , public Engine::Input::Keyboard {
        public:
            Textbox(Theme& theme, Callback = NULL);
            virtual ~Textbox();
            virtual void onEnter(std::string& str) {}
            virtual void onTab(std::string& str) {}
            void setString(const std::string&);
            std::string getString();
            void Select();
            void Deselect();
            bool Selected() { return selected; }
            void Disable(bool);
        private:
            sf::RectangleShape outside, textCursor, overlay;
            sf::Text textObj;
            std::string textString;

            void updateString();
            void updateCursor();

            void draw(sf::RenderWindow &win);
            void updatePosition();
            void onUp(const sf::Vector2f&, sf::Mouse::Button);
            void onDown(const sf::Vector2f&, sf::Mouse::Button);
            void mouseRelease(const sf::Vector2f&, sf::Mouse::Button);

            void keyPress(sf::Event::KeyEvent &evt);
            void windowTextEntered(sf::Event::TextEvent &evt);

            sf::Font* textFont;
            bool selected;
        };

        //Button
        class Button : public Component {
        public:
            Button(Theme&, Callback = NULL, const std::string& btnString = "");
            virtual ~Button();
            void setString(const std::string&);
            std::string getString();
        private:
            sf::RectangleShape outside, overlay;
            sf::Text textObj;

            void draw(sf::RenderWindow &win);
            void updatePosition();
            void onDown(const sf::Vector2f&, sf::Mouse::Button);
            void mouseRelease(const sf::Vector2f&, sf::Mouse::Button);

            sf::Font* textFont;
        };

    }
}

#endif // _ENGINE_GUI_H_
