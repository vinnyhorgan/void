#ifndef API_H
#define API_H

#include <string>

#include "angelscript.h"

void print(std::string &str);
std::string toString(float value);
void graphicsPrint(std::string &str, int x, int y);

#endif