//========= Copyright Rob Cruickshank, All rights reserved. ===================//
//
// Purpose: Provide timeout functions to avoid 
// getting stuck in a buggy function, 
// e.g. ValidateNavGoal() in ai_basenpc_movement.cpp
//
// $NoKeywords: $
//=============================================================================//

#ifndef TIMEOUT_H
#define TIMEOUT_H
#ifdef _WIN32
#pragma once
#endif

#ifdef _WIN32
#include <Windows.h>  

#pragma warning(disable : 4100)		// unreferenced formal parameter

// Misc util to throw exception (timer function). Used to timeout
// ValidateNavGoal(), which doesn't always return
static VOID CALLBACK TimeoutFunc( PVOID lpParam, BOOLEAN TimerOrWaitFired );

static HANDLE SetTimeout( int millisec );

static VOID ClearTimeout( HANDLE hTimerQueue );

#endif // _WIN32
#endif // TIMEOUT_H
