#include "mengine.h"

int main( int argc, char* argv[] )
{
	MEngine MEngine;

	if(!MEngine.Initialize())
		return 1;

	while (!MEngine.ShouldQuit())
	{
		MEngine.Update();
		MEngine.Render();
	}

	MEngine.Shutdown();

	return 0;
}