#include "test.as"

bool showGreeting = false;
array<int> a = {3, 2, 1};

void init()
{
    hello();

    vd::Version version = vd::getVersion();

    vd::log(vd::toString(version.major));
    vd::log(vd::toString(a[0]));
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
        vd::graphics::print("Hellooo!", 10, 10);
    }

    vd::graphics::rectangle("line", 100, 100, 100, 100);
}

void filesdropped(array<string> filenames)
{
    for (uint i = 0; i < filenames.length(); i++)
    {
        vd::log(filenames[i]);
    }
}

void focus(bool focus)
{
    vd::log("Focus: " + vd::toString(focus));
}

void resize(int width, int height)
{
    vd::log("Resize: " + vd::toString(width) + "x" + vd::toString(height));
}

void textinput(string text)
{
    vd::log("Text input: " + text);
}
