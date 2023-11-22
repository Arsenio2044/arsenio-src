// Disable warning C4005 for the duration of this file
#pragma warning(disable : 4005)

#ifndef BRUH_H
#define BRUH_H

#include "cbase.h"
#include <windows.h>
#include "tier0/platform.h"
#include "vgui/ILocalize.h"
#include <iostream>
#include <dxgi.h>



#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)-1)
#endif

// Define your minimum system requirements here
const int MIN_REQUIRED_CPU_SPEED = 2000; // Minimum CPU speed in MHz
const int MIN_REQUIRED_GPU_MEMORY = 512;  // Minimum GPU memory in MB
const int MIN_REQUIRED_RAM = 19000;        // Minimum RAM in MB

// Function prototype for CheckSystemRequirements
bool CheckSystemRequirements();

void Check();

#endif // BRUH_H

// Re-enable warning C4005
#pragma warning(default : 4005)
