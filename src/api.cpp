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

extern vector<string> consoleHistory;

namespace Api
{
    // Utils
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

    // Graphics
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

    // Math
    float random()
    {
        return GetRandomValue(0, 100) / 100.0f;
    }

    float sqrt(float value)
    {
        return sqrtf(value);
    }

    // Keyboard
    bool isDown(int key)
    {
        return IsKeyDown(key);
    }

    bool isPressed(int key)
    {
        return IsKeyPressed(key);
    }

    bool isReleased(int key)
    {
        return IsKeyReleased(key);
    }

    // timer
    int getFPS()
    {
        return GetFPS();
    }
}
