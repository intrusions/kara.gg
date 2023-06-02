#include "Bypass.hpp"
#include "Vector3.hpp"
#include "Utils.hpp"

int main()
{
	Bypass* bypass = new Bypass();

	printHeader();
	std::cout << std::endl;

	std::cout << "kara.GG injected : ";
	if (bypass->attach()
			&& bypass->getModuleBaseAddress(CLIENT_MODNAME)
			&& bypass->getModuleBaseAddress(ENGINE_MODNAME))
	{
		printGreen("Success", true);
		printWaitForEnterKey();
		bypass->startMultiThreading();
	} else {
		printRed("Failed", true);
		std::system("pause");
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}