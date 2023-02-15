#include "test.as"

vd::graphics::Image a;
vd::graphics::Image b;
vd::graphics::Image c;

float x = 100;
float y = 100;

vd::Version version;

void init()
{
    hello();

    a = vd::graphics::newImage("assets/test.png");
    b = vd::graphics::newImage("assets/test.png");
    c = vd::graphics::newImage("assets/test.png");
}

void update(float dt)
{
    if (vd::keyboard::isDown(vd::keyboard::Key::A))
    {
        x -= 200 * dt;
    }
    else if (vd::keyboard::isDown(vd::keyboard::Key::D))
    {
        x += 200 * dt;
    }

    if (vd::keyboard::isDown(vd::keyboard::Key::W))
    {
        y -= 200 * dt;
    }
    else if (vd::keyboard::isDown(vd::keyboard::Key::S))
    {
        y += 200 * dt;
    }

    version = vd::getVersion();
    vd::log("Version: " + vd::toString(version.major) + "." + vd::toString(version.minor) + "." + vd::toString(version.patch));
}

void draw()
{
    vd::graphics::print("FPS: " + vd::toString(vd::timer::getFPS()), 10, 10);

    vd::graphics::rectangle("line", 100, 100, 100, 100);

    vd::graphics::drawImage(a, x, y);
    vd::graphics::drawImage(b, 200, 200);
    vd::graphics::drawImage(c, 300, 300);
}
