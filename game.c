#include "WedoEngine.h"

int main( int argc, char *argv[] ) {
	if( WedoEngine_Initialize() ) {
		WedoEngine_ExecuteScript(argc, argv, "test.fe");
		WedoEngine_Terminate();
	}

	return 0;
}
