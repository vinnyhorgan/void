#include "test.as"

bool showGreeting = false;

void init()
{
    hello();
}

void update(float dt)
{
    if (vd::keyboard::isPressed(vd::keyboard::Key::A))
    {
        showGreeting = !showGreeting;
    }
}

void draw()
{
    if (showGreeting)
    {
        vd::graphics::print("Hello!", 10, 10);
    }
}
