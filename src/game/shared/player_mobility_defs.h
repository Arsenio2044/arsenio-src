//========= Copyright Rob Cruickshank, All rights reserved. ============//
//
// Purpose: Shared definitions for client and server mobility code
//
//=============================================================================//

#ifndef PLAYER_MOBILITY_DEFS_H
#define PLAYER_MOBILITY_DEFS_H
#ifdef _WIN32
#pragma once
#endif

enum AirJumpState
{
	AIRJUMP_READY = 1,    // Player has not airjumped yet
	AIRJUMP_NORM_JUMPING, // Player is normal jumping
	AIRJUMP_NOW,          // Player is airjumping right now
	AIRJUMP_DONE          // Player has airjumped already
};

enum WallRunState
{
	WALLRUN_NOT = 0, // Not wallrunning
	WALLRUN_LEAN_IN, // About to start wall running, lean in
	WALLRUN_RUNNING, // Wallrunning
	WALLRUN_JUMPING, // Jumping off the wall
	WALLRUN_STALL,   // Wallrunning, but facing into the wall or 
	                 // otherwise not moving along it
	WALLRUN_SCRAMBLE // basically waterjumping - vertical velocity is allowed
};

// This is different from basebludgeonweapon
enum PlayerMeleeState
{
	MELEE_NO,  // doesn't mean we are not doing quick melees, just that 
	           // the player class doesn't care
	MELEE_SLASH, // doing a slash attack so change the angle of the viewmodel
	MELEE_SURGE_MOVE, // Forward charging attack
	MELEE_SURGE_HIT,
	MELEE_DONE // just finished, so speed up the animation of switching to the prev weapon
	// Not implemented yet, but hope to add a charge attack that will affect player movement
};

/* HL2_NORM_SPEED + 5, must be going faster than this
* to powerslide */
#define GAMEMOVEMENT_POWERSLIDE_MIN_SPEED 195.0f

static const float GAMEMOVEMENT_WALLRUN_MAX_Z = 20.0f;
static const float GAMEMOVEMENT_WALLRUN_MIN_Z = -50.0f;

#endif // PLAYER_MOBILITY_DEFS_H