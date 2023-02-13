#include "api.h"

#include <stdio.h>
#include <string>

#include "angelscript.h"
#include "raylib.h"

void print(std::string &str)
{
    printf("%s\n", str.c_str());
}

std::string toString(float value)
{
    return std::to_string(value);
}

void graphicsPrint(std::string &str, int x, int y)
{
    DrawText(str.c_str(), x, y, 32, WHITE);
}