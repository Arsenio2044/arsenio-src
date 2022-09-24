//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//


#include "cbase.h"
#include "movevars_shared.h"

#if defined( TF_CLIENT_DLL ) || defined( TF_DLL )
#include "tf_gamerules.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// some cvars used by player movement system
#if defined( HL2_DLL ) || defined( HL2_CLIENT_DLL )
#define DEFAULT_GRAVITY_STRING	"600"
#else
#define DEFAULT_GRAVITY_STRING	"800"
#endif

// Mobility
#define DEFAULT_JUMP_HEIGHT_STRING "50.0"
#define DEFAULT_SLIDE_TIME_STRING "2000.0" 
#define DEFAULT_SLIDE_SPEED_BOOST_STRING "75.0"
#define DEFAULT_WALLRUN_TIME_STRING "2000.0"
#define DEFAULT_WALLRUN_SPEED_STRING "300.0"
#define DEFAULT_WALLRUN_BOOST_STRING "60.0"

float GetCurrentGravity( void )
{
#if defined( TF_CLIENT_DLL ) || defined( TF_DLL )
	if ( TFGameRules() )
	{
		return ( sv_gravity.GetFloat() * TFGameRules()->GetGravityMultiplier() );
	}
#endif 

	return sv_gravity.GetFloat();
}

float GetJumpHeight(void)
{
	return sv_jump_height.GetFloat();
}

float GetSlideTime( void )
{
	return arsenio_slide_time.GetFloat();
}
float GetWallRunTime( void )
{
	return arsenio_wallrun_time.GetFloat();
}
float GetSlideSpeedBoost( void )
{
	return arsenio_slide_speed_boost.GetFloat();
}
float GetWallRunSpeed( void )
{
	return arsenio_wallrun_speed.GetFloat();
}
float GetWallRunBoost( void )
{
	return arsenio_wallrun_boost.GetFloat();
}

ConVar	sv_gravity		( "sv_gravity", DEFAULT_GRAVITY_STRING, FCVAR_NOTIFY | FCVAR_REPLICATED, "World gravity." );

// Mobility 

ConVar  sv_jump_height  ( "sv_jump_height", DEFAULT_JUMP_HEIGHT_STRING, FCVAR_NOTIFY | FCVAR_REPLICATED, "Jump Height.");
ConVar  arsenio_slide_time( "arsenio_slide_time", DEFAULT_SLIDE_TIME_STRING, FCVAR_NOTIFY | FCVAR_REPLICATED, "Powerslide time." );
ConVar  
    arsenio_slide_speed_boost(
		"arsenio_slide_speed_boost",
		DEFAULT_SLIDE_SPEED_BOOST_STRING,
		FCVAR_NOTIFY | FCVAR_REPLICATED,
		"Speed boost for powerslide." );
ConVar 
    arsenio_wallrun_time( 
	    "arsenio_wallrun_time", 
		DEFAULT_WALLRUN_TIME_STRING, 
		FCVAR_NOTIFY | FCVAR_REPLICATED, 
		"Wallrun max duration.");

ConVar
	sv_airjump_delta(
		"sv_airjump_delta",
		"125.0",
		FCVAR_NOTIFY | FCVAR_REPLICATED,
		"Amount to change direction in airjump.");

ConVar
	arsenio_wallrun_anticipation(
		"arsenio_wallrun_anticipation",
		"2",
		FCVAR_NOTIFY | FCVAR_REPLICATED,
		"0 - none, 1 - Eye roll only, 2 - Full (bumps)." );

ConVar
	arsenio_wallrun_boost(
		"arsenio_wallrun_boost",
		DEFAULT_WALLRUN_BOOST_STRING,
		FCVAR_NOTIFY | FCVAR_REPLICATED,
		"Wallrun speed boost." );
ConVar
	arsenio_wallrun_jump_boost(
		"arsenio_wallrun_jump_boost",
		"0.15",
		FCVAR_NOTIFY | FCVAR_REPLICATED,
		"Fraction of wallrun speed to add to jump." );
ConVar
	arsenio_wallrun_jump_push(
		"arsenio_wallrun_jump_push",
		"0.25",
		FCVAR_NOTIFY | FCVAR_REPLICATED,
		"Fraction of wall jump speed to go to pushing off wall." );
 ConVar 
	 arsenio_wallrun_speed (
		 "arsenio_wallrun_speed",
		 DEFAULT_WALLRUN_SPEED_STRING,
		 FCVAR_NOTIFY | FCVAR_REPLICATED, 
		 "Wallrun speed.");
 ConVar
	 arsenio_wallrun_accel(
		 "arsenio_wallrun_accel",
		 "4.25",
		 FCVAR_NOTIFY | FCVAR_REPLICATED,
		 "Wallrun acceleration." );

 ConVar
	 arsenio_wallrun_roll (
		 "arsenio_wallrun_roll",
		 "14.0",
		 FCVAR_NOTIFY | FCVAR_REPLICATED,
		 "Wallrun view roll angle.");

 ConVar
	 arsenio_wallrun_max_rise (
		 "arsenio_wallrun_max_rise",
		 "25.0",
		 FCVAR_NOTIFY | FCVAR_REPLICATED,
		 "Wallrun upward limit.");

 ConVar
	 arsenio_wallrun_min_rise (
		 "arsenio_wallrun_min_rise",
		 "-50.0",
		 FCVAR_NOTIFY | FCVAR_REPLICATED,
		 "Wallrun downward limit.");

 ConVar
	 arsenio_wallrun_scramble_z (
		 "arsenio_wallrun_scramble_z",
		 "28.0",
		 FCVAR_NOTIFY | FCVAR_REPLICATED,
		 "Height we can climb to.");

 ConVar
	 arsenio_wallrun_lookahead(
		 "arsenio_wallrun_lookahead",
		 "0.22",
		 FCVAR_NOTIFY | FCVAR_REPLICATED,
		 "Amount of time (in seconds) to lookahead for bumps or corners when wallrunning." );

 ConVar
	 arsenio_wallrun_inness(
		 "arsenio_wallrun_inness",
		 "360",
		 FCVAR_NOTIFY | FCVAR_REPLICATED,
		 "Scaling factor for how much to steer inward toward the wall when wallrunning" );
 ConVar
	 arsenio_wallrun_outness(
		 "arsenio_wallrun_outness",
		 "300",
		 FCVAR_NOTIFY | FCVAR_REPLICATED,
		 "Scaling factor for how much to steer outward around obstacles when wallrunning" );

 ConVar
	 arsenio_wallrun_lookness(
		 "arsenio_wallrun_lookness",
		 "1",
		 FCVAR_NOTIFY | FCVAR_REPLICATED,
		 "Scaling factor for how much to adjust view to look where you're going when wallrunning" );
 ConVar
	 arsenio_wallrun_look_delay(
		 "arsenio_wallrun_look_delay",
		 "0.3",
		 FCVAR_NOTIFY | FCVAR_REPLICATED,
		 "How long to wait before aligning the view with the move direction when wallrunning." );

 ConVar
	 arsenio_wallrun_feet_z(
		 "arsenio_wallrun_feet_z",
		 "10",
		 FCVAR_NOTIFY | FCVAR_REPLICATED,
		 "Fudge for starting a wallrun with your feet below the bottom edge of the wall" );

 ConVar 
	 arsenio_wallrun_stick_angle(
		 "arsenio_wallrun_stick_angle", 
		 "45",
		 FCVAR_NOTIFY | FCVAR_REPLICATED, 
		 "Min angle away from wall norm for player to wallrun");

 ConVar 
	 arsenio_wallrun_corner_stick_angle(
		 "arsenio_wallrun_corner_stick_angle", 
		 "80",
		 FCVAR_NOTIFY | FCVAR_REPLICATED,
		 "End wallrun at end of wall if facing within this angle of wall norm" );
 ConVar
	 sv_duck_time(
		 "sv_duck_time",
		 "0.2",
		 FCVAR_NOTIFY | FCVAR_REPLICATED,
		 "Amount of time (in seconds) it takes to duck.",
		 true, 0.001, true, 1);

 ConVar
	 sv_itsa_me(
		 "sv_itsa_me",
		 "0",
		 FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_CHEAT,
		 "Destroy enemies by landing on top of them." );
 ConVar
	 sv_coyote_time(
		 "sv_coyote_time",
		 "0.2",
		 FCVAR_NOTIFY | FCVAR_REPLICATED,
		 "Time after leaving a surface that jumps are still allowed." );
 ConVar 
	 sv_floorlava_dpm( 
	     "sv_floorlava_dpm", 
		 "0", 
		 FCVAR_NOTIFY | FCVAR_REPLICATED, 
		 "If not zero, damage per minute from being on the floor");

 ConVar 
	 cl_show_speedo( 
	     "cl_show_speedo", 
		 "0", 
		 FCVAR_NOTIFY | FCVAR_REPLICATED, 
		 "Display speedometer in upper right corner, refresh every n frames" );

 ConVar
	 certain_restrictions(
		 "certain_restrictions",
		 "0",
		 FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_ARCHIVE,
		 "Limits the speed you can reach by jumping, sliding, or wallrunning" );

 ConVar
	 arsenio_slide_lock(
	     "arsenio_slide_lock", 
		 "0", 
		 FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_ARCHIVE,
		 "Locks your move direction when sliding" );

#if defined( DOD_DLL ) || defined( CSTRIKE_DLL ) || defined( HL1MP_DLL )
ConVar	sv_stopspeed	( "sv_stopspeed","100", FCVAR_NOTIFY | FCVAR_REPLICATED, "Minimum stopping speed when on ground." );
#else
ConVar	sv_stopspeed	( "sv_stopspeed","100", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY, "Minimum stopping speed when on ground." );
#endif // DOD_DLL || CSTRIKE_DLL

ConVar	sv_noclipaccelerate( "sv_noclipaccelerate", "5", FCVAR_NOTIFY | FCVAR_ARCHIVE | FCVAR_REPLICATED);
ConVar	sv_noclipspeed	( "sv_noclipspeed", "5", FCVAR_ARCHIVE | FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_specaccelerate( "sv_specaccelerate", "5", FCVAR_NOTIFY | FCVAR_ARCHIVE | FCVAR_REPLICATED);
ConVar	sv_specspeed	( "sv_specspeed", "3", FCVAR_ARCHIVE | FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_specnoclip	( "sv_specnoclip", "1", FCVAR_ARCHIVE | FCVAR_NOTIFY | FCVAR_REPLICATED);


ConVar	sv_maxspeed("sv_maxspeed", "600", FCVAR_NOTIFY | FCVAR_REPLICATED);


#ifdef _XBOX
	ConVar	sv_accelerate	( "sv_accelerate", "7", FCVAR_NOTIFY | FCVAR_REPLICATED);
#else

#if defined( CSTRIKE_DLL ) || defined( HL1MP_DLL )
	ConVar	sv_accelerate	( "sv_accelerate", "10", FCVAR_NOTIFY | FCVAR_REPLICATED);
#else
	ConVar	sv_accelerate	( "sv_accelerate", "10", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY);
#endif // CSTRIKE_DLL
	
#endif//_XBOX

#if defined( CSTRIKE_DLL ) || defined( HL1MP_DLL )
ConVar	sv_airaccelerate(  "sv_airaccelerate", "10", FCVAR_NOTIFY | FCVAR_REPLICATED);    
ConVar	sv_wateraccelerate(  "sv_wateraccelerate", "10", FCVAR_NOTIFY | FCVAR_REPLICATED);     
ConVar	sv_waterfriction(  "sv_waterfriction", "1", FCVAR_NOTIFY | FCVAR_REPLICATED);      
ConVar	sv_footsteps	( "sv_footsteps", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Play footstep sound for players" );
ConVar	sv_rollspeed	( "sv_rollspeed", "200", FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar	sv_rollangle	( "sv_rollangle", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Max view roll angle");
#else
ConVar	sv_airaccelerate(  "sv_airaccelerate", "10", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY );    
ConVar	sv_wateraccelerate(  "sv_wateraccelerate", "10", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY );     
ConVar	sv_waterfriction(  "sv_waterfriction", "1", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY );      
ConVar	sv_footsteps	( "sv_footsteps", "1", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY, "Play footstep sound for players" );
ConVar	sv_rollspeed	( "sv_rollspeed", "200", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY);
ConVar	sv_rollangle	( "sv_rollangle", "3", FCVAR_NOTIFY | FCVAR_REPLICATED, "Max view roll angle");
#endif // CSTRIKE_DLL

#if defined( DOD_DLL ) || defined( CSTRIKE_DLL ) || defined( HL1MP_DLL )
ConVar	sv_friction		( "sv_friction","4", FCVAR_NOTIFY | FCVAR_REPLICATED, "World friction." );
#else
ConVar	sv_friction		( "sv_friction","4", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY, "World friction." );
#endif // DOD_DLL || CSTRIKE_DLL

#if defined( CSTRIKE_DLL ) || defined( HL1MP_DLL )
ConVar	sv_bounce		( "sv_bounce","0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Bounce multiplier for when physically simulated objects collide with other objects." );
ConVar	sv_maxvelocity	( "sv_maxvelocity","3500", FCVAR_REPLICATED, "Maximum speed any ballistically moving object is allowed to attain per axis." );
ConVar	sv_stepsize		( "sv_stepsize","18", FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar	sv_backspeed	( "sv_backspeed", "0.6", FCVAR_ARCHIVE | FCVAR_REPLICATED, "How much to slow down backwards motion" );
ConVar  sv_waterdist	( "sv_waterdist","12", FCVAR_REPLICATED, "Vertical view fixup when eyes are near water plane." );
#else
ConVar	sv_bounce		( "sv_bounce","0", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY, "Bounce multiplier for when physically simulated objects collide with other objects." );
ConVar	sv_maxvelocity	( "sv_maxvelocity","3500", FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY, "Maximum speed any ballistically moving object is allowed to attain per axis." );
ConVar	sv_stepsize		( "sv_stepsize","18", FCVAR_NOTIFY | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY );
ConVar	sv_backspeed	( "sv_backspeed", "0.6", FCVAR_ARCHIVE | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY, "How much to slow down backwards motion" );
ConVar  sv_waterdist	( "sv_waterdist","12", FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY, "Vertical view fixup when eyes are near water plane." );
#endif // CSTRIKE_DLL

ConVar	sv_skyname		( "sv_skyname", "sky_urb01", FCVAR_ARCHIVE | FCVAR_REPLICATED, "Current name of the skybox texture" );

// Vehicle convars
ConVar r_VehicleViewDampen( "r_VehicleViewDampen", "1", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED );

// Jeep convars
ConVar r_JeepViewDampenFreq( "r_JeepViewDampenFreq", "7.0", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar r_JeepViewDampenDamp( "r_JeepViewDampenDamp", "1.0", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar r_JeepViewZHeight( "r_JeepViewZHeight", "10.0", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED );

// Airboat convars
ConVar r_AirboatViewDampenFreq( "r_AirboatViewDampenFreq", "7.0", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED );
ConVar r_AirboatViewDampenDamp( "r_AirboatViewDampenDamp", "1.0", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED);
ConVar r_AirboatViewZHeight( "r_AirboatViewZHeight", "0.0", FCVAR_CHEAT | FCVAR_NOTIFY | FCVAR_REPLICATED );
