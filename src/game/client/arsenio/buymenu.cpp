

//The following include files are necessary to allow your buymenu.cpp to compile.
#include "cbase.h"
#include "buymenu.h"
using namespace vgui;
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
 
//CBuyMenu class: Tutorial example class
class CBuyMenu : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CBuyMenu, vgui::Frame);
	//CMyPanel : This Class / vgui::Frame : BaseClass
 
	CBuyMenu(vgui::VPANEL parent); 	// Constructor
	~CBuyMenu(){};				// Destructor
 
protected:
	//VGUI overrides:
	virtual void OnTick();
	virtual void OnCommand(const char* pcCommand);
 
private:
	//Other used VGUI control Elements:
	Button *m_pCloseButton;
};
 
// Constuctor: Initializes the Panel
CBuyMenu::CBuyMenu(vgui::VPANEL parent)
	: BaseClass(NULL, "BuyMenu")
{
	SetParent(parent);
 
	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);
 
	SetProportional(false);
	SetTitleBarVisible(true);
	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(false);
	SetSizeable(false);
	SetMoveable(false);
	SetVisible(true);
 
 
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme"));
 
	LoadControlSettings("resource/UI/MainBuyMenu.res");
 
	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);
 
	//DevMsg("Buy Menu has been constructed\n");
 
	//Button done
	m_pCloseButton = new Button(this, "Button", "Close", this, "turnoff");
	m_pCloseButton->SetPos(433, 472);
	m_pCloseButton->SetDepressedSound("common/bugreporter_succeeded.wav");
	m_pCloseButton->SetReleasedSound("ui/buttonclick.wav");
}
 
//Class: CMyPanelInterface Class. Used for construction.
class CMyPanelInterface : public BuyMenu
{
private:
	CBuyMenu *buymenu;
public:
	CMyPanelInterface()
	{
		buymenu = NULL;
	}
	void Create(vgui::VPANEL parent)
	{
		buymenu = new CBuyMenu(parent);
	}
	void Destroy()
	{
		if (buymenu)
		{
			buymenu->SetParent((vgui::Panel *)NULL);
			delete buymenu;
		}
	}
	void Activate(void)
	{
		if (buymenu)
		{
			buymenu->Activate();
		}
	}
};
static CMyPanelInterface g_BuyMenu;
BuyMenu* buymenu = (BuyMenu*)&g_BuyMenu;
 
ConVar cl_showbuymenu("cl_showbuymenu", "0", FCVAR_CLIENTDLL, "Sets the state of the buy panel <state>");
 
void CBuyMenu::OnTick()
{
	BaseClass::OnTick();
	SetVisible(cl_showbuymenu.GetBool());
}
 
CON_COMMAND(ToggleBuyMenu, "Toggles the buy menu on or off")
{
	cl_showbuymenu.SetValue(!cl_showbuymenu.GetBool());
	buymenu->Activate();
};
 
void CBuyMenu::OnCommand(const char* pcCommand)
{
	BaseClass::OnCommand(pcCommand);
 
	if (!Q_stricmp(pcCommand, "turnoff"))
		cl_showbuymenu.SetValue(0);
}