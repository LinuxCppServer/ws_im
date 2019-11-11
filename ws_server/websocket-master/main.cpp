#include "framework.h"

int main(int argc, char **argv){
	if (argc < 3)
	{
		std::cout << "usage: <./ws_srv> <ip?> <port?>" << std::endl;
		exit(-1);
	}
	CFrameworkService::getServerInstance(argv[1], argv[2])->run();
	return 0;
}
