#include <iostream>

#include "vers.h"
#include "bayan.h"

using namespace std::literals::string_literals;


int main(int argc, char const* argv[]) 
{
	using namespace otus_hw8;
	try
	{
		Options options;
		if (!parse_command_line(argc, argv, options))
			return 1;
		
	}	
	catch(const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
