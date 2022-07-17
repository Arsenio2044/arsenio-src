#include "cbase.h"

#if defined( _WIN32 )
	#include <windows.h>
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CON_COMMAND( _apprestart, "Same as _restart, except it properly restarts the application instead of doing a \"fake\" restart, which can cause the SteamAPI to break." )
{
#if defined( _WIN32 )
	// additional information
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	// set the size of the structures
	ZeroMemory( &si, sizeof( si ) );
	si.cb = sizeof( si );
	ZeroMemory( &pi, sizeof( pi ) );

	// No args
	std::string szArgs = " -debugmode"; // We need the space, else it goes bonkers
	std::string szExec = "contagion.exe";

	LPSTR _args = const_cast<char*>( szArgs.c_str() );
	LPSTR path = const_cast<char*>( szExec.c_str() );
	// start the program up
	BOOL ret = CreateProcess(
		path,				// the path
		_args,				// Command line
		NULL,				// Process handle not inheritable
		NULL,				// Thread handle not inheritable
		FALSE,				// Set handle inheritance to FALSE
		0,					// No creation flags
		NULL,				// Use parent's environment block
		NULL,				// Use parent's starting directory 
		&si,				// Pointer to STARTUPINFO structure
		&pi					// Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
	);

	if ( ret == TRUE )
		engine->ClientCmd_Unrestricted( "quit" );
#endif
}
