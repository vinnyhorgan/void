#include "test.as"

class Pixel
{
    vd::Vector2 position;
}

array<Pixel> pixels;

void init()
{
    hello();
}

void update(float dt)
{
    if (vd::mouse::isDown(vd::mouse::MouseButton::Left))
    {
        vd::Vector2 mouse = vd::mouse::getPosition();

        Pixel newPixel;
        newPixel.position = mouse;

        pixels.insertLast(newPixel);
    }

    for (uint i = 0; i < pixels.length(); i++)
    {
        pixels[i].position.y += 1;

        if (pixels[i].position.y > 1000)
        {
            pixels.removeAt(i);
        }
    }
}

void draw()
{
    vd::Vector2 mouse = vd::mouse::getPosition();
    vd::graphics::print("FPS: " + vd::toString(vd::timer::getFPS()), 10, 10);
    vd::graphics::print("Mouse: " + vd::toString(int(mouse.x)) + ", " + vd::toString(int(mouse.y)), 10, 50);
    vd::graphics::print("Pixels: " + vd::toString(pixels.length()), 10, 90);

    for (uint i = 0; i < pixels.length(); i++)
    {
        vd::graphics::point(int(pixels[i].position.x), int(pixels[i].position.y));
    }
}
