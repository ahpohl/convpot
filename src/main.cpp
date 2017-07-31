#include "main.h"
#include "msql.h"

#include <iostream>
using namespace std;

int main(int argc, char* argv[])
{
	// create uninitialised pointer to setup class
	Sql* prog;

	// try to create program instance
	try {
		// create new instance of setup
		prog = new Sql(argc, argv);
	}

	// catch exceptions
    catch(const return_exception& e) {
#if(DEBUG == 1)
    	cout << e.what() << endl;
#endif
    	return EXIT_SUCCESS;
    }

    catch(const exception& e) {
   		cout << "Exception caught: " << e.what() << endl;
    	return EXIT_FAILURE;
    }

	// delete pointer to program
	delete prog;

	return EXIT_SUCCESS;
}
