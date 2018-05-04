#include "Parser.h"

#include <iostream>

int main()
{
	Parser p;
	try
	{
		p.parse("e:/test.c");
		p.run();
	}
	catch (std::string &e)
	{
		std::cout << e;
	}

	return 0;
}
