

#ifndef SEARCHPATH_MANAGER_H
#define SEARCHPATH_MANAGER_H

#ifdef _WIN32
#pragma once
#endif



#include "filesystem.h"
#include "mapentities.h"
#include "convar.h"
#include <vector>

using namespace std;

enum SPM_DEBUG_LVL {
	SPM_DEBUG_NONE,
	SPM_DEBUG_BASIC,
	SPM_DEBUG_FULL
};

// Class to manage search path configuration. We can reconfigure the search path
// for HL2, Ep1, or Ep2 based on the current map. 
class SearchPathManager {


public:
	
	// index values for core search path configurations
	static const int SPM_HL2 = 0;
	static const int SPM_EP1 = -1;
	static const int SPM_EP2 = -2;
	static const int SPM_OTHER = -3;

	SearchPathManager( IFileSystem *filesystem ) :
		m_pFileSystem( filesystem ),
		m_iCurrentConfig(SPM_EP2),
		m_iDebugLevel(SPM_DEBUG_BASIC)
	{}

	int GetCurrentConfig() { return m_iCurrentConfig; }

	// Read the current search paths, then recreate in the 
	// order required for the current game
	void ConfigureSearchPaths( int index );

private:

	void GetAndSplitSearchPath( const char* pPath, CUtlVector<char*, CUtlMemory<char*> > &pathlist );
	void RestoreSearchPathsForGame( const char* pPath, CUtlVector<char*, CUtlMemory<char*> > &paths );


	// Standard search paths for every configuration
	CUtlVector<char*, CUtlMemory<char*> > BASE_PATH;
	CUtlVector<char*, CUtlMemory<char*> > EXECUTABLE_PATH;
	CUtlVector<char*, CUtlMemory<char*> > PLATFORM;
	CUtlVector<char*, CUtlMemory<char*> > mod_path;
	CUtlVector<char*, CUtlMemory<char*> > gamebin_path;
	CUtlVector<char*, CUtlMemory<char*> > mod_write_path;
	CUtlVector<char*, CUtlMemory<char*> > default_write_path;
	CUtlVector<char*, CUtlMemory<char*> > game_write_path;
	CUtlVector<char*, CUtlMemory<char*> > log_dir_path;
	CUtlVector<char*, CUtlMemory<char*> > config_path;
	CUtlVector<char*, CUtlMemory<char*> > map_path;

	// Paths to add to the GAME search path
	CUtlVector<char*, CUtlMemory<char*> > hl2SearchPaths;
	CUtlVector<char*, CUtlMemory<char*> > episodicSearchPaths;
	CUtlVector<char*, CUtlMemory<char*> > ep2SearchPaths;

	// Other mod search paths to add to GAME search path
	CUtlVector<char*, CUtlMemory<char*> > m_Paths;

	// Names of mods (index corresponds to m_Paths)
	CUtlVector<char*, CUtlMemory<char*> > m_szModNames;

	IFileSystem *m_pFileSystem;

	int m_iCurrentConfig;

	SPM_DEBUG_LVL m_iDebugLevel;

};

#endif // SEARCHPATH_MANAGER_H