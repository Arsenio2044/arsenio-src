#include "cbase.h"
#include "CursorClip.h"
#include "winlite.h"
#include "tier0/icommandline.h"
#include "vcrmode.h"

#include "ienginevgui.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Contains code taken from VGUI System.cpp
#pragma region VGUI
#ifdef _WIN32
static BOOL CALLBACK GetMainApplicationWindowHWND_EnumProc(HWND hWnd, LPARAM lParam)
{
	// Return TRUE to continue enumeration or FALSE to stop

	// Given window thread/process id
	DWORD dwProcessId, dwThreadId;
	dwThreadId = GetWindowThreadProcessId(hWnd, &dwProcessId);

	// Our thread/process id
	DWORD dwOurProcessId;
	dwOurProcessId = GetCurrentProcessId();

	// Foreign process
	if (dwOurProcessId != dwProcessId)
		return TRUE;
	// Service window
	if (!IsWindowVisible(hWnd) || !IsWindowEnabled(hWnd))
		return TRUE;

	// Assume that we found it
	*(HWND *)lParam = hWnd;
	return FALSE; // stop enumeration
}
#endif // #ifdef _WIN32

#ifdef _WIN32
static HWND GetMainApplicationWindowHWND()
#else
static void* GetMainApplicationWindowHWND()
#endif
{
#ifdef _WIN32
	HWND hWnd = NULL;

	//
	// Pass 1: calling on a GUI thread
	//
	DWORD dwThreadId = GetCurrentThreadId();

	GUITHREADINFO gti;
	memset(&gti, 0, sizeof(gti));
	gti.cbSize = sizeof(gti);
	GetGUIThreadInfo(dwThreadId, &gti);

	hWnd = gti.hwndActive;
	for (HWND hParent = hWnd ? GetParent(hWnd) : hWnd;
		hParent; hWnd = hParent, hParent = GetParent(hWnd))
		continue;

	if (hWnd)
		return hWnd;

	//
	// Pass 2: non-GUI thread requiring the main winow
	//
	EnumWindows(GetMainApplicationWindowHWND_EnumProc, (LPARAM)&hWnd);
	if (hWnd)
		return hWnd;
#endif // #ifdef _WIN32

	// Failed to find the window by all means...
	return NULL;
}
#pragma endregion

CCursorClipManagement *g_pCursorClipManager = NULL;
CCursorClipManagement::CCursorClipManagement()
{
	// Get the HWND of game's window.
	// This will return NULL if the platform is not WIN32.
	m_HWND = GetMainApplicationWindowHWND();

	m_bToolsMode = CommandLine()->CheckParm("-tools");
	m_bDevMode = CommandLine()->CheckParm("-dev") || CommandLine()->CheckParm("-developer");
	m_bEnabled = CommandLine()->CheckParm("-nocursorclipping") || !IsPC() || IsX360() ? false : true;

	m_bLockState = false;
	m_bLockStateMouseDown = false;
	m_bLastMouseDownState = false;
	m_bMouseDownStartedOutside = false;
	m_flMouseDownStartedAt = 0;
	m_iAction = CURSOR_CLIPMANAGEMENT_NONE;

}

CCursorClipManagement::~CCursorClipManagement()
{
}


// This is to simplify our work, to avoid spreading things all around the engine.
// This can be called in FRAME_RENDER_END (OnRenderEnd, cdll_client_int.cpp).
void CCursorClipManagement::Think()
{
	// Checks that will always be static.
	// Should we really stop this from working when tools and devmode is active?
	if (m_bToolsMode || m_bDevMode)
		return;

	// Checks that can dynamically be changed.
	// Note: Maybe just enable this when the game is windowed.
	if (!m_bEnabled || !enginevgui->IsGameUIVisible())
	{
		// Try unlocking the cursor if we've locked it before.
		// Note: This will cause the cursor to be unlocked when the chat window is open, or any other tool windows.
		UnlockCursor();
		return;
	}

	// Check if the game window is currently active.
	if (engine->IsActiveApp())
	{
		// Lock the cursor.
		// Is the left button currently down?
		if (VCRHook_GetKeyState(VK_LBUTTON) < 0)
		{
			// That's where we are going to fix the VPanels from getting out of our bounds while moving.

			if (m_bLockState && !m_bLockStateMouseDown)
				m_bLockState = false;

			if (!m_bLastMouseDownState)
			{
				// First frame of mouse down.

				m_bLastMouseDownState = true;
				long left, top, right, bottom;
				GetCurrentRECT(left, top, right, bottom, true);

				tagPOINT curpos;
				VCRHook_GetCursorPos(&curpos);

				// Did they start in our clip area?
				if (curpos.x > left-1 && curpos.y > top-1 &&
					curpos.x < right+1 && curpos.y < bottom+1)
				{
					// Make sure they're not clicking a button or something, so add a delay.
					// TODO this sucks, do think about another way of detecting that.
					m_flMouseDownStartedAt = gpGlobals->curtime + 0.2f;
				}
				else
					// They might be moving a panel that is currently outside of our clip area, do not block.
					m_bMouseDownStartedOutside = true;
			}

			if (m_flMouseDownStartedAt < gpGlobals->curtime && !m_bMouseDownStartedOutside)
				LockCursor(true);
		}
		else
		{
			if (m_bLockStateMouseDown)
			{
				// First frame they stopped holding left button.

				m_bLockStateMouseDown = false;
				m_bLockState = false;
			}

			// Should we really try setting our booleans and floats to false and 0 every time?
			m_bLastMouseDownState = false;
			m_flMouseDownStartedAt = 0;
			m_bMouseDownStartedOutside = false;

			LockCursor();
		}
	}
	else
		// If we're not active, unlock the cursor whatsoever.
		UnlockCursor();
}

void CCursorClipManagement::LockCursor( bool mouseDown )
{
	// Do not allow if we've locked it already, unless we got an action.
	if (m_bLockState && m_iAction != CURSOR_CLIPMANAGEMENT_RELOCKREQUIRED)
		return;

	// Do not allow if the handle is not valid (i.e. we might be running on a different platform, something else than WIN32)
	if (!m_HWND)
		return;

	RECT rect;

	if (GetCurrentRECT(rect.left, rect.top, rect.right, rect.bottom, mouseDown))
		m_bLockState = ClipCursor(&rect);

	m_bLockStateMouseDown = mouseDown && m_bLockState;

	// Reset our action.
	if (m_iAction == CURSOR_CLIPMANAGEMENT_RELOCKREQUIRED)
		m_iAction = CURSOR_CLIPMANAGEMENT_NONE;
}

bool CCursorClipManagement::GetCurrentRECT(long &left, long &top, long &right, long &bottom, bool mouseDown)
{
	// Need a valid handle.
	if (!m_HWND)
		return false;

	bool bRetval = false;

	RECT rect;
	if (GetClientRect((HWND)m_HWND, &rect))
		bRetval = true;

	if (MapWindowPoints((HWND)m_HWND, NULL, (LPPOINT)&rect, 2) == 0)
		bRetval = false;

	if (mouseDown)
	{
		// Restrict the cursor to a specific area of the window, the rest will be specified by our padding.
		// Keep the padding as little as possible, as long as there are ten-twenty pixels to capture the panel with the cursor and move.
		// This is actually for people who just prefer to not hook into VGUI and fix the existing panels from going out of the bounds.
		int paddingLeft = 20;
		int paddingTop = 20;
		int paddingRight = 20;
		int paddingBottom = 20;

		// TODO maybe check if there is any window currently being moved? Checking the panels' active state would work too.
		rect.left += paddingLeft;
		rect.top += paddingTop;
		rect.right -= paddingRight;
		rect.bottom -= paddingBottom;
	}

	left = rect.left;
	top = rect.top;
	right = rect.right;
	bottom = rect.bottom;

	return bRetval;
}


void CCursorClipManagement::SetLockAction(int iAction)
{
	if (iAction != m_iAction)
		m_iAction = iAction;
}


void CCursorClipManagement::UnlockCursor( void )
{
	// Return if the cursor is already unlocked.
	if (!m_bLockState)
		return;

	m_bLockState = !((bool)ClipCursor(NULL));
	m_bLockStateMouseDown = false;
}

void CCursorClipManagement::Init(void)
{
	if (g_pCursorClipManager) return;

	// Create the singleton.
	static CCursorClipManagement mMan;
	g_pCursorClipManager = &mMan;
}