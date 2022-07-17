//======================Copyright Glitch Software ===========================//
//
//  Purpose: Plays a movie and reports on finish.
//
//===========================================================================//

#include "cbase.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CPointCutscene : public CLogicalEntity
{
public:
	DECLARE_CLASS( CPointCutscene, CLogicalEntity );
	DECLARE_DATADESC();

	CPointCutscene( void ) { }
	~CPointCutscene( void ) { }

	virtual void Precache( void );
	virtual void Spawn( void );
	
private:

	void		InputPlayMovie( inputdata_t &data );
	void		InputMovieFinished( inputdata_t &data );

	string_t	m_strMovieFilename;
	bool		m_bAllowUserSkip;

	COutputEvent	m_OnPlaybackFinished;
};

LINK_ENTITY_TO_CLASS( point_cutscene, CPointCutscene ); // TUX: Let's not forget to declare the entity...

BEGIN_DATADESC( CPointCutscene )

	DEFINE_KEYFIELD( m_strMovieFilename, FIELD_STRING, "MovieFilename" ),
	DEFINE_KEYFIELD( m_bAllowUserSkip, FIELD_BOOLEAN, "allowskip" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "PlayMovie", InputPlayMovie ),
	DEFINE_INPUTFUNC( FIELD_VOID, "__MovieFinished", InputMovieFinished ),

	DEFINE_OUTPUT( m_OnPlaybackFinished, "OnPlaybackFinished" ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPointCutscene::Precache( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPointCutscene::Spawn( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPointCutscene::InputPlayMovie( inputdata_t &data )
{
	const char *szVideoCommand = ( m_bAllowUserSkip ) ? "playvideo_exitcommand" : "playvideo_exitcommand_nointerrupt";
	// Build the hacked string
	
	char szClientCmd[256];
	Q_snprintf(szClientCmd, sizeof(szClientCmd),
		"%s %s ent_fire %s __MovieFinished\n",
		szVideoCommand,
		STRING(m_strMovieFilename) );
/*				GetEntityNameAsCStr() )*/

	// Send it on
	engine->ServerCommand( szClientCmd );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPointCutscene::InputMovieFinished( inputdata_t &data )
{
	// Simply fire our output
	m_OnPlaybackFinished.FireOutput( this, this );
}