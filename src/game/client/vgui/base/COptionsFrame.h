
#ifndef __NEWGAMEOPTIONS_FRAME_H__
#define __NEWGAMEOPTIONS_FRAME_H__

#include <vgui_controls/Panel.h>
#include <stdstring.h>

// Our option tabs
class COptionsTab;
class COptionVarList;
class COptionVarData;
class COptionVarHelp;

#define MAX_TABS	7

// Our main base frame for our game options
class COptionsFrame : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( COptionsFrame, vgui::Panel );
public:
	COptionsFrame(Panel *parent, const char *panelName);
	~COptionsFrame();
	void LoadOptionsTab(COptionsTab* tab, bool bIsTabClick);
	void SetTabSizes(int w, int t);
	bool HasChangedValues();
	bool RequiresRestart();
	void ApplyChanges();
	void ApplyDefault();
	void AspectRatioChanged(int asp);
	void ChangeImageFrame(int frame);
	void ChangeHelpImage_PhoneBackground(int img);
	void UpdateHelpBox(const std::string& szName);
	COptionsTab *GetTabChoice() { return m_TabChoice; }

	// Deselect all items in button state
	void DeselectAllItems();
	void DeselectHoverItem();

	enum SelectedItemAction_t
	{
		Action_Press = 0,
		Action_Right,
		Action_Left
	};

	void SelectedItemAction(SelectedItemAction_t state);
	void SelectNewItem(bool next);
	void SelectItem(int item);

	bool CanSelectNewItemTab();

	// Refreshes current tab
	void SelectNewTab(bool next);
	void ReloadCurrentTab();
	void HideAllItems(COptionsTab *item);
	void ReloadAndSetTab();
	void ReloadOnBind();

protected:
	bool CheckIfTitle(int item);
	int GetNextAvailableItem(bool next);
	void UpdateImage(const char* szImage);

	void ClearVectors();
	int GetDisplayMode();
	void BuildOptionsTabs();
	void CreateTab(const char* name, const char* szFile, int tab_val);
	void Paint() override;

private:
	int m_iItemSelected;
	COptionsTab *m_TabChoice;
	int m_nTabWide;
	int m_nTabTall;
	CUtlVector<COptionsTab*> tabs;	// Our tabs
	CUtlVector<COptionVarList*> frame_val;
	COptionVarHelp *frame_info;					// Our information goes into this panel
};

#endif