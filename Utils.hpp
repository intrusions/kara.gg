#pragma once

#include <windows.h>
#include <iostream>
#include <cstdlib>
#include <iomanip>

void	printHeader(void);
void	printGreen(std::string str, bool nl);
void	printRed(std::string str, bool nl);
void	printCheatIsActive(bool aim, bool glow, bool trig, bool radar);
void	printWaitForEnterKey(void);


