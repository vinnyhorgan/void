#include "api.h"

#include <cstdio>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <algorithm>

#include "angelscript.h"
#include "raylib.h"

using namespace std;

Color currentColor = WHITE;
map<int, Texture> loadedTextures;

namespace Api
{
    void log(string &str)
    {
        printf("%s\n", str.c_str());
        consoleHistory.push_back(str + "\n");
    }

    string toString(int value)
    {
        return to_string(value);
    }

    string toString(float value)
    {
        return to_string(value);
    }

    string toString(bool value)
    {
        return to_string(value);
    }

    Version getVersion()
    {
        Version version;

        version.major = 1;
        version.minor = 2;
        version.patch = 3;

        return version;
    }

    namespace Graphics
    {
        void print(string &str, int x, int y)
        {
            DrawText(str.c_str(), x, y, 32, WHITE);
        }

        void rectangle(string &mode, int x, int y, int width, int height)
        {
            if (mode == "fill")
            {
                DrawRectangle(x, y, width, height, currentColor);
            }
            else if (mode == "line")
            {
                DrawRectangleLines(x, y, width, height, currentColor);
            }
        }

        Image newImage(string &path)
        {
            int id = GetRandomValue(0, 1000000);

            Image newImage;

            newImage.id = id;

            loadedTextures[id] = LoadTexture(path.c_str());

            return newImage;
        }

        void drawImage(Image image, int x, int y)
        {
            DrawTexture(loadedTextures[image.id], x, y, WHITE);
        }

        void point(int x, int y)
        {
            DrawPixel(x, y, currentColor);
        }
    }

    namespace Math
    {
        float random()
        {
            return GetRandomValue(0, 100) / 100.0f;
        }
    }

    namespace Mouse
    {
        Vector2 getPosition()
        {
            Vector2 mouse;

            mouse.x = virtualMouse.x;
            mouse.y = virtualMouse.y;

            return mouse;
        }

        bool isDown(MouseButton button)
        {
            return IsMouseButtonDown(button);
        }

        bool isPressed(MouseButton button)
        {
            return IsMouseButtonPressed(button);
        }

        bool isReleased(MouseButton button)
        {
            return IsMouseButtonReleased(button);
        }
    }

    namespace Keyboard
    {
        bool isDown(Key key)
        {
            return IsKeyDown(key);
        }

        bool isPressed(Key key)
        {
            return IsKeyPressed(key);
        }

        bool isReleased(Key key)
        {
            return IsKeyReleased(key);
        }
    }

    namespace Timer
    {
        int getFPS()
        {
            return GetFPS();
        }
    }
}
