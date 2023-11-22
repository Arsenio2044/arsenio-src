// Disable warning C4005 for the duration of this file
#pragma warning(disable : 4005)


#include "cbase.h"
#include "SystemRequirementsCheck.h"

// Link with dxguid.lib to use DXGI functions
#pragma comment(lib, "dxguid.lib")



void Check()
{
    CheckSystemRequirements();
    Error("Your system does not meet the minimum system requirements to run this game.");
}

int GetCPUSpeed()
{
    // Query CPU frequency in MHz using Windows API
    DWORD bufSize = sizeof(DWORD);
    DWORD mhz = 0;

    if (RegGetValueA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", "~MHz", RRF_RT_ANY, nullptr, &mhz, &bufSize) == ERROR_SUCCESS)
    {
        return static_cast<int>(mhz);
    }

    return -1; // Error retrieving CPU speed
}

int GetGPUMemory()
{
    // Query GPU memory using DirectX
    IDXGIFactory1* pFactory;
    if (CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&pFactory) == S_OK)
    {
        IDXGIAdapter1* pAdapter;
        if (pFactory->EnumAdapters1(0, &pAdapter) == S_OK)
        {
            DXGI_ADAPTER_DESC1 desc;
            if (pAdapter->GetDesc1(&desc) == S_OK)
            {
                return static_cast<int>(desc.DedicatedVideoMemory / (1024 * 1024)); // Convert to MB
            }
        }
    }

    return -1; // Error retrieving GPU memory
}

int GetRAMSize()
{
    // Query physical RAM size using Windows API
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);

    if (GlobalMemoryStatusEx(&memStatus))
    {
        return static_cast<int>(memStatus.ullTotalPhys / (1024 * 1024)); // Convert to MB
    }

    return -1; // Error retrieving RAM size
}

// Function to check system requirements
bool CheckSystemRequirements()
{
    int cpuSpeed = GetCPUSpeed();     // Get CPU speed in MHz
    int gpuMemory = GetGPUMemory();   // Get GPU memory in MB
    int ramSize = GetRAMSize();       // Get RAM size in MB

    // Compare system specs to minimum requirements
    if (cpuSpeed < MIN_REQUIRED_CPU_SPEED ||
        gpuMemory < MIN_REQUIRED_GPU_MEMORY ||
        ramSize < MIN_REQUIRED_RAM)
    {
        // Display an error message to the user
        Error("Your system does not meet the minimum system requirements to run this game.");
        Warning("Minimum Requirements:\n");
        Warning("CPU Speed: %d MHz\n", MIN_REQUIRED_CPU_SPEED);
        Warning("GPU Memory: %d MB\n", MIN_REQUIRED_GPU_MEMORY);
        Warning("RAM: %d MB\n", MIN_REQUIRED_RAM);

        return false; // System requirements not met
    }

    return true; // System requirements met
}



// Re-enable warning C4005
#pragma warning(default : 4005)