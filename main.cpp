#include "./header/Bypass.hpp"
#include "./header/Vector3.hpp"
#include "./header/Utils.hpp"

int main()
{
	Bypass bypass;
	
	printHeader();
	std::cout << std::endl;

	std::cout << "settings.cfg found and parsed : ";
	if (bypass.ParseConfigFile(SETTINGS_CFG)) {
		printGreen("Success", true);
	} else {
		printRed("Failed", true);
		std::system("pause");
		return (EXIT_FAILURE);
	}

	std::cout << "kara.GG attachment : ";
	if (bypass.attach()
			&& bypass.getModuleBaseAddress(CLIENT_MODNAME)
			&& bypass.getModuleBaseAddress(ENGINE_MODNAME))
	{
		printGreen("Success", true);
		printWaitForEnterKey();
		bypass.startMultiThreading();
	} else {
		printRed("Failed", true);
		std::system("pause");
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}