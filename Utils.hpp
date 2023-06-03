#pragma once

#include "Mutex.hpp"
#include <windows.h>
#include <iostream>
#include <cstdlib>
#include <iomanip>

void	printHeader(void);
void	printGreen(std::string str, bool nl);
void	printRed(std::string str, bool nl);
void	printOn(void);
void	printOff(void);
void	printMenu(Mutex *mutex);
void	printWaitForEnterKey(void);

const int getWeaponPaint(const short itemId);