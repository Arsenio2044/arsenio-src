
#ifndef __NEWGAMEOPTIONS_VARLIST_H__
#define __NEWGAMEOPTIONS_VARLIST_H__

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/Panel.h>
#include <stdstring.h>

class KeyValues;

//-----------------------------------------------------------------------------
// Purpose: COptionVarData
//-----------------------------------------------------------------------------
class COptionVarData : public vgui::Panel
{
public:
	COptionVarData(vgui::Panel* parent, char const* panelName);

	virtual	void	OnSizeChanged(int wide, int tall);

	vgui::Panel* pControl;
	vgui::Label* pPrompt;
	std::string m_video;
	std::string m_cvar;
	std::string m_help;
	std::string m_help_img;
	std::string m_help_vid;
};

//-----------------------------------------------------------------------------
// Purpose: A list of variable height child panels
//-----------------------------------------------------------------------------
class COptionVarList : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( COptionVarList, vgui::Panel ); 

public:
	typedef struct dataitem_s
	{
		// Always store a panel pointer
		vgui::Panel *panel;
	} DATAITEM;

	COptionVarList( vgui::Panel *parent, char const *panelName);
	~COptionVarList();

	void ResetScrollBarPos();

	// DATA & ROW HANDLING
	// The list now owns the panel
	virtual int	computeVPixelsNeeded( void );
	virtual int AddItem( vgui::Panel *panel );
	virtual int	GetItemCount( void );
	virtual vgui::Panel *GetItem(int itemIndex); // returns pointer to data the row holds
	virtual void RemoveItem(int itemIndex); // removes an item from the table (changing the indices of all following items)
	virtual void DeleteAllItems(); // clears and deletes all the memory used by the data items

	void ScrollToPos(int item, bool direct = false );
	int GetScrollPos();

	// career-mode UI wants to nudge sub-controls around
	void SetSliderYOffset(int pixels);

	// PAINTING
	virtual vgui::Panel *GetCellRenderer( int row );

	MESSAGE_FUNC_INT( OnSliderMoved, "ScrollBarSliderMoved", position );

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	vgui::Panel *GetEmbedded()
	{
		return _embedded;
	}

protected:

	DATAITEM	*GetDataItem( int itemIndex );

	virtual void PerformLayout();
	virtual void PaintBackground();
	virtual void OnMouseWheeled(int delta);

private:
	// list of the column headers
	vgui::Dar<DATAITEM *>	_dataItems;
	vgui::ScrollBar		*_vbar;
	vgui::Panel			*_embedded;

	int					_tableStartX;
	int					_tableStartY;
	int					_sliderYOffset;
};


#endif