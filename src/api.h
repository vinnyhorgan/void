#ifndef API_H
#define API_H

#include <string>

#include "angelscript.h"

using namespace std;

namespace Api
{
    // Utils
    void log(string &str);
    string toString(int value);
    string toString(float value);
    string toString(bool value);

    // Graphics
    void print(string &str, int x, int y);
    void rectangle(string &mode, int x, int y, int width, int height);

    // Math
    float random();
    float sqrt(float value);

    // Keyboard
    bool isDown(int key);
    bool isPressed(int key);
    bool isReleased(int key);
}

#endif
