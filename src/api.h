#ifndef API_H
#define API_H

#include <string>
#include <vector>

#include "raylib.h"
#include "angelscript.h"

using namespace std;

extern vector<string> consoleHistory;
extern Vector2 virtualMouse;

namespace Api
{
    struct Version
    {
        int major;
        int minor;
        int patch;
    };

    struct Vector2
    {
        float x;
        float y;
    };

    struct Image
    {
        int id;
    };

    void log(string &str);
    string toString(int value);
    string toString(float value);
    string toString(bool value);
    Version getVersion();

    namespace Graphics
    {
        void print(string &str, int x, int y);
        void rectangle(string &mode, int x, int y, int width, int height);
        Image newImage(string &path);
        void drawImage(Image image, int x, int y);
        void point(int x, int y);
    }

    namespace Math
    {
        float random();
    }

    namespace Mouse
    {
        enum MouseButton
        {
            LeftB = 0,
            RightB = 1,
            Middle = 2,
        };

        Vector2 getPosition();
        bool isDown(MouseButton button);
        bool isPressed(MouseButton button);
        bool isReleased(MouseButton button);
    }

    namespace Keyboard
    {
        enum Key
        {
            A = 65,
            B = 66,
            C = 67,
            D = 68,
            E = 69,
            F = 70,
            G = 71,
            H = 72,
            I = 73,
            J = 74,
            K = 75,
            L = 76,
            M = 77,
            N = 78,
            O = 79,
            P = 80,
            Q = 81,
            R = 82,
            S = 83,
            T = 84,
            U = 85,
            V = 86,
            W = 87,
            X = 88,
            Y = 89,
            Z = 90,
            Space = 32,
            Enter = 13,
            Escape = 27,
            Up = 38,
            Down = 40,
            Left = 37,
            Right = 39,
            Shift = 16,
            Control = 17,
            Alt = 18,
            Tab = 9,
            Backspace = 8,
            CapsLock = 20,
            Zero = 48,
            One = 49,
            Two = 50,
            Three = 51,
            Four = 52,
            Five = 53,
            Six = 54,
            Seven = 55,
            Eight = 56,
            Nine = 57,
            F1 = 112,
            F2 = 113,
            F3 = 114,
            F4 = 115,
            F5 = 116,
            F6 = 117,
            F7 = 118,
            F8 = 119,
            F9 = 120,
            F10 = 121,
            F11 = 122,
            F12 = 123,
        };

        bool isDown(Key key);
        bool isPressed(Key key);
        bool isReleased(Key key);
    }

    namespace Timer
    {
        int getFPS();
    }
}

#endif
