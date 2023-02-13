#include "test.as"

bool showGreeting = false;

void init()
{
    hello();
}

void update(float dt)
{
    if (vd::keyboard::isPressed(65))
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
