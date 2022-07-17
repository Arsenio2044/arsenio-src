
#ifndef __CONTUI_NEWGAMEOPTIONS_H__
#define __CONTUI_NEWGAMEOPTIONS_H__

#include "CONTUI.h"

class COptionsFrame;

namespace CONTUI {
	class NewGameOptions : public CL4DFrame
	{
		DECLARE_CLASS_SIMPLE( NewGameOptions, CL4DFrame );

	public:
		NewGameOptions(vgui::Panel *parent, const char *panelName);
		~NewGameOptions();
		void ApplySettings( KeyValues *inResourceData ) override;
		void ApplySchemeSettings(vgui::IScheme* pScheme) override;
		void OnCommand( const char* ) override;
		void OnKeyCodePressed( vgui::KeyCode code );
		void OnAspectRatioChanged( int iAspect );
		void OnChangeImageFrame( int iFrame );
		void OnPhoneBackgroundChanged( int iImage );
		void OnHelpUpdate( const std::string &szName );

		// If buttons are in button state, deselect them.
		void DeselectAllItems();

		void static ResetKeyControllerSettings();
		void static ResetKeyBinds();
		void static ResetCurrentTab();
		void static IgnoreNewChanges();
		void static IgnoreNewChangesBack();
		void static ApplyRecommended();
		void static ApplyRestart();

		void SetRecommendedSettings();

		COptionsFrame *GetFrame() { return mainframe; }
		
		bool IsValueChanged() { return m_bStateChanges; }
		bool RequiresRestart() { return m_bRequiresRestart; }
		void SetValueChanged(bool state) { m_bStateChanges = state; }
		void SetRequiresRestart(bool state) { m_bRequiresRestart = state; }
	private:
		bool m_bStateChanges;
		bool m_bRequiresRestart;
		int m_nTabWide;
		int m_nTabTall;
		COptionsFrame* mainframe;
	};
}

#endif // __CONTUI_NEWGAMEOPTIONS_H__