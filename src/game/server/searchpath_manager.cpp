/*
 * Mobility Mod V2. 
 * This class holds logic to rearrange the search path so that the right 
 * files are loaded for the right maps. E.g. HL2 maps need the HL2 Alyx
 * model, or you get missing animations and broken scenes. Ditto the Ep2
 * maps and Ep2 Alyx model. And other things of that nature.
 *
 * The search path is read from gameinfo.txt in the mod directory. It points
 * to the hl2 directories.
 */

#include "cbase.h"
#include "searchpath_manager.h"


// Read the current search paths, then recreate in the 
// order required for the current game
void SearchPathManager::ConfigureSearchPaths( int index )
{
	// Get searchpaths
	
	// This buffer size is not good - there's not much
	// headroom if they want to add custom search paths
	// for custom models etc. 
	// Someone tried it and their search path got truncated, 
	// broke the game.
	const int searchpath_len = 2048;
	char searchpath[searchpath_len];

	m_iCurrentConfig = index;

	// split,  loop and remove rejects
	CUtlVector<char*, CUtlMemory<char*> > pathdirs;

	if (ep2SearchPaths.Count() == 0)
	{
		if (m_iDebugLevel == SPM_DEBUG_FULL)
		    Msg( "Reading search paths for first time\n" );
		// We start by make lists of all the search paths. Some of these we will
		// add back as soon as we clear the search paths from the filesystem, others
		// we might need later
		GetAndSplitSearchPath( "BASE_PATH", BASE_PATH );
		GetAndSplitSearchPath( "EXECUTABLE_PATH", EXECUTABLE_PATH );
		GetAndSplitSearchPath( "PLATFORM", PLATFORM );
		GetAndSplitSearchPath( "MOD", mod_path );
		GetAndSplitSearchPath( "GAMEBIN", gamebin_path );
		GetAndSplitSearchPath( "MOD_WRITE", mod_write_path );
		GetAndSplitSearchPath( "DEFAULT_WRITE_PATH", default_write_path );
		GetAndSplitSearchPath( "LOGDIR", log_dir_path );
		GetAndSplitSearchPath( "CONFIG", config_path );

		m_pFileSystem->GetSearchPath( "GAME", true, searchpath, searchpath_len );

		if (m_iDebugLevel == SPM_DEBUG_FULL) {
			Msg( "Initial GAME searchpath: \n\n%s\n\n", searchpath );
			Msg( "Open files:\n" );
		}

		Q_SplitString( searchpath, ";", pathdirs );
		for (int i = 0; i < pathdirs.Count(); i++)
		{
			if (Q_strstr( pathdirs[i], ".bsp" ) != NULL)
			{
				if (m_iDebugLevel == SPM_DEBUG_FULL) {
					Msg( "Found map path :   %s\n\n", pathdirs[i] );
				}
				map_path.AddToTail( pathdirs[i] );
			}
			else if (Q_strstr( pathdirs[i], "ep2" ) != NULL)
			{
				ep2SearchPaths.AddToTail( pathdirs[i] );
			}
			else if (Q_strstr( pathdirs[i], "episodic" ) != NULL)
			{
				episodicSearchPaths.AddToTail( pathdirs[i] );
			}
			else
			{
				hl2SearchPaths.AddToTail( pathdirs[i] );
			}
		}
	}

	// Remove ALL search paths! 
	// (There is a function to remove particular search paths, but it doesn't 
	//  seem to work for VPK paths, so we have to completely recreate)
	m_pFileSystem->RemoveAllSearchPaths();
	m_pFileSystem->GetSearchPath( "GAME", true, searchpath, searchpath_len );

	// Add back the search paths for this game only

	// Start with standard paths
	RestoreSearchPathsForGame( "GAME", map_path );
	RestoreSearchPathsForGame( "MOD", mod_path );
	RestoreSearchPathsForGame( "BASE_PATH", BASE_PATH );
	RestoreSearchPathsForGame( "EXECUTABLE_PATH", EXECUTABLE_PATH );
	RestoreSearchPathsForGame( "PLATFORM", PLATFORM );
	RestoreSearchPathsForGame( "GAMEBIN", gamebin_path );
	RestoreSearchPathsForGame( "MOD_WRITE", mod_write_path );
	RestoreSearchPathsForGame( "DEFAULT_WRITE_PATH", default_write_path );
	RestoreSearchPathsForGame( "LOGDIR", log_dir_path );
	RestoreSearchPathsForGame( "CONFIG", config_path );

	if ( m_iCurrentConfig > 0 ) {

		// Add mod paths
		if (m_iDebugLevel == SPM_DEBUG_FULL) {
			Msg( "Adding search path:\n%s\n to end of %s.", m_Paths[m_iCurrentConfig], "GAME" );
		}
		m_pFileSystem->AddSearchPath( m_Paths[m_iCurrentConfig], "GAME", PATH_ADD_TO_TAIL );
	}

	// Add GAME search paths back in priority order for this game.
	// (We still want to be able to spawn hunters in hl2, so we still want 
	// all the search paths available)
	
	switch ( m_iCurrentConfig ) {

	case SearchPathManager::SPM_HL2:
		RestoreSearchPathsForGame( "GAME", hl2SearchPaths );
		RestoreSearchPathsForGame( "GAME", ep2SearchPaths );
		RestoreSearchPathsForGame( "GAME", episodicSearchPaths );
		break;
	case SearchPathManager::SPM_EP1:
		RestoreSearchPathsForGame( "GAME", episodicSearchPaths );//fallthrough
		RestoreSearchPathsForGame( "GAME", hl2SearchPaths );
		RestoreSearchPathsForGame( "GAME", ep2SearchPaths );
		break;
	case SearchPathManager::SPM_EP2:// fallthrough
	default:
		RestoreSearchPathsForGame( "GAME", ep2SearchPaths );
		RestoreSearchPathsForGame( "GAME", episodicSearchPaths );
		RestoreSearchPathsForGame( "GAME", hl2SearchPaths );
	}

	
	m_pFileSystem->GetSearchPath( "GAME", true, searchpath, 2048 );
	if (m_iDebugLevel >= SPM_DEBUG_BASIC) {
		Msg( "\n\nFetched MODIFIED hl2 search path : %s\n", searchpath );
	}
}



void SearchPathManager::GetAndSplitSearchPath( const char* pPath, CUtlVector<char*, CUtlMemory<char*> > &pathlist )
{
	char searchpath[2048];
	m_pFileSystem->GetSearchPath( pPath, true, searchpath, 2048 );
	Q_SplitString( searchpath, ";", pathlist );
}

void SearchPathManager::RestoreSearchPathsForGame( 
	const char* pPath, CUtlVector<char*, CUtlMemory<char*> > &paths )
{
	// loop and re-add rejects (at the end)
	for (int i = 0; i < paths.Size(); i++)
	{
		if (m_iDebugLevel == SPM_DEBUG_FULL) {
			Msg( "Adding search path:\n%s\n to end of %s.", paths[i], pPath );
		}
		m_pFileSystem->AddSearchPath( paths[i], pPath, PATH_ADD_TO_TAIL );

	}
}