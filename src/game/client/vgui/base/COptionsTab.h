
#ifndef __NEWGAMEOPTIONS_TAB_H__
#define __NEWGAMEOPTIONS_TAB_H__

#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>
#ifdef JSON_ENABLE
#include <json/reader.h>
#include <json/writer.h>
#include <fstream>
#endif

class COptionHiddenButton : public vgui::Button
{
	DECLARE_CLASS_SIMPLE( COptionHiddenButton, vgui::Button );
public:
	COptionHiddenButton(vgui::Panel *parent, const char *panelName, const char *text, vgui::Panel *pActionSignalTarget=NULL, const char *pCmd=NULL, const char *pCmd2=NULL);
	COptionHiddenButton(vgui::Panel *parent, const char *panelName, const wchar_t *text, vgui::Panel *pActionSignalTarget=NULL, const char *pCmd=NULL, const char *pCmd2=NULL);
	void OnMousePressed(vgui::MouseCode code) override;
	void OnCursorEntered() override;
	void SetVarItem(bool state) { m_varitem = state; }
private:
	std::string szCmd;
	std::string szCmd2;
	bool m_varitem;
	void ApplySchemeSettings(vgui::IScheme* pScheme) override;
};

class COptionsTab : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( COptionsTab, vgui::Panel );
public:
	// Create our options tab, and tell it which file it should read from.
	COptionsTab( Panel* parent, const char *szName, const char *szFile, int tabval );
	~COptionsTab();
#ifdef JSON_ENABLE
	Json::Value Get();
#endif
	int GetTabValue() { return m_iTabVal; }
	void SetActive(bool state) { m_Active = state; }
	std::string GetTab() { return m_szName; }
	void SetLabelSize(int w, int t);
private:
	void Paint() override;
	void DrawBackground(int w, int t);
	void DrawBorder(int w, int t);
	void OnMousePressed(vgui::MouseCode code) override;
	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void OnCommand( const char* command ) override;
	int m_iTabVal;
	std::string m_szName;
	bool m_Active;
	vgui::HFont m_Font;
	vgui::Label *m_Label;
	COptionHiddenButton *m_ButttonHidden;
};


#endif