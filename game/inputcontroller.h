#ifndef INPUTCONTROLLER_H
#define INPUTCONTROLLER_H


class InputController : public Engine::Input::Keyboard , public Engine::Input::Mouse , public Engine::Active {
public:
    InputController();
    virtual ~InputController();

    CFGameObject* myPlayer;

protected:
    unsigned int moveDir;

    virtual void mouseButtonPress(sf::Event::MouseButtonEvent &evt);
    virtual void mouseButtonRelease(sf::Event::MouseButtonEvent &evt);
    virtual void keyPress(sf::Event::KeyEvent &evt);
    virtual void keyRelease(sf::Event::KeyEvent &evt);
    virtual void step(sf::Time &delta);

};

#endif // INPUTCONTROLLER_H
