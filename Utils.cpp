#include "Utils.hpp"

void	printHeader() {
	std::cout << "  _                     _____  _____ " << std::endl;
	std::cout << " | |                   / ____|/ ____|" << std::endl;
	std::cout << " | | ____ _ _ __ __ _ | |  __| |  __ " << std::endl;
	std::cout << " | |/ / _` | '__/ _` || | |_ | | |_ |" << std::endl;
	std::cout << " |   < (_| | | | (_| || |__| | |__| |" << std::endl;
	std::cout << " |_|\\_\\__,_|_|  \\__,_(_)_____|\\_____|" << std::endl;
}

void	printGreen(std::string str, bool nl) {

	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, 2);

	std::cout << str;
	
	if (nl)
		std::cout << std::endl;

	SetConsoleTextAttribute(handle, 15);
}

void	printRed(std::string str, bool nl) {

	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, 12);

	std::cout << str;
	
	if (nl)
		std::cout << std::endl;

	SetConsoleTextAttribute(handle, 15);
}

void	printCheatIsActive(bool aim, bool glow, bool trig, bool radar) {

	std::system("cls");
	printHeader();
	std::cout << std::endl;

	std::cout << "|    AIMBOT|      GLOW|      TRIG|  RADAR 2D|" << std::endl;

	if (aim) {
		std::cout << "|    ";
		printGreen("ON", false);
		std::cout << "    |";
	} else {
		std::cout << "|    ";
		printRed("OFF", false);
		std::cout << "   |";
	}

	if (glow) {
		std::cout << "    ";
		printGreen("ON", false);
		std::cout << "    |";
	} else {
		std::cout << "    ";
		printRed("OFF", false);
		std::cout << "   |";
	}

	if (trig) {
		std::cout << "    ";
		printGreen("ON", false);
		std::cout << "    |";
	} else {
		std::cout << "    ";
		printRed("OFF", false);
		std::cout << "   |";
	}

	if (radar) {
		std::cout << "    ";
		printGreen("ON", false);
		std::cout << "    |";
	} else {
		std::cout << "    ";
		printRed("OFF", false);
		std::cout << "   |";
	}
}

void printWaitForEnterKey() {
	std::cout << "Press Enter to continue...";
	std::cin.get();
}