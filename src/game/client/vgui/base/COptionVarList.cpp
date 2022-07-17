// Backported from Zombie Panic! Source

#include <assert.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/ScrollBarSlider.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>

#include <KeyValues.h>
#include <vgui/MouseCode.h>
#include <vgui/KeyCode.h>
#include <vgui/IInput.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include "COptionVarList.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

class VScrollBarReversedButtons_Legacy_VarList : public ScrollBar
{
public:
	VScrollBarReversedButtons_Legacy_VarList( Panel *parent, const char *panelName, bool vertical );
	virtual void ApplySchemeSettings( IScheme *pScheme );
};

VScrollBarReversedButtons_Legacy_VarList::VScrollBarReversedButtons_Legacy_VarList( Panel *parent, const char *panelName, bool vertical ) : ScrollBar( parent, panelName, vertical )
{
}

void VScrollBarReversedButtons_Legacy_VarList::ApplySchemeSettings( IScheme *pScheme )
{
	ScrollBar::ApplySchemeSettings( pScheme );

	Button *pButton;
	pButton = GetButton( 0 );
	pButton->SetArmedColor(		pButton->GetSchemeColor("DimBaseText", pScheme), pButton->GetBgColor());
	pButton->SetDepressedColor(	pButton->GetSchemeColor("DimBaseText", pScheme), pButton->GetBgColor());
	pButton->SetDefaultColor(	pButton->GetFgColor(),							 pButton->GetBgColor());
	pButton->SetVisible(false);
	
	pButton = GetButton( 1 );
	pButton->SetArmedColor(		pButton->GetSchemeColor("DimBaseText", pScheme), pButton->GetBgColor());
	pButton->SetDepressedColor(	pButton->GetSchemeColor("DimBaseText", pScheme), pButton->GetBgColor());
	pButton->SetDefaultColor(	pButton->GetFgColor(),							 pButton->GetBgColor());
	pButton->SetVisible(false);

	// No color and no border
	SetBgColor(Color(0, 0, 0, 0));
	SetBorder(nullptr);

	SetScrollbarButtonsVisible(false);
	UseImages(nullptr, nullptr, nullptr, nullptr);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : x - 
//			y - 
//			wide - 
//			tall - 
// Output : 
//-----------------------------------------------------------------------------
COptionVarList::COptionVarList( vgui::Panel *parent, char const *panelName ) : Panel( parent, panelName )
{
	SetBounds( 0, 0, 100, 100 );
	_sliderYOffset = 0;

	_vbar = new VScrollBarReversedButtons_Legacy_VarList(this, "COptionVarListVScroll", true );
	_vbar->SetBounds( 0, 0, 20, 20 );
	_vbar->SetVisible(false);
	_vbar->AddActionSignalTarget( this );

	_embedded = new Panel( this, "PanelListEmbedded" );
	_embedded->SetBounds( 0, 0, 20, 20 );
	_embedded->SetPaintBackgroundEnabled( false );
	_embedded->SetPaintBorderEnabled( false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
COptionVarList::~COptionVarList()
{
	// free data from table
	DeleteAllItems();
}

void COptionVarList::ResetScrollBarPos()
{
	_vbar->SetValue(0);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	COptionVarList::computeVPixelsNeeded( void )
{
	int pixels =0;
	DATAITEM *item;
	Panel *panel;
	for ( int i = 0; i < _dataItems.GetCount(); i++ )
	{
		item = _dataItems[ i ];
		if ( !item )
			continue;

		panel = item->panel;
		if ( !panel )
			continue;

		int w, h;
		panel->GetSize( w, h );

		pixels += h;
	}
	pixels+=5; // add a buffer after the last item

	return pixels;

}

//-----------------------------------------------------------------------------
// Purpose: Returns the panel to use to render a cell
// Input  : column - 
//			row - 
// Output : Panel
//-----------------------------------------------------------------------------
Panel *COptionVarList::GetCellRenderer( int row )
{
	DATAITEM *item = _dataItems[ row ];
	if ( item )
	{
		Panel *panel = item->panel;
		return panel;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: adds an item to the view
//			data->GetName() is used to uniquely identify an item
//			data sub items are matched against column header name to be used in the table
// Input  : *item - 
//-----------------------------------------------------------------------------
int COptionVarList::AddItem( Panel *panel )
{
	InvalidateLayout();

	DATAITEM *newitem = new DATAITEM;
	newitem->panel = panel;
	panel->SetParent( _embedded );
	return _dataItems.PutElement( newitem );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : 
//-----------------------------------------------------------------------------
int	COptionVarList::GetItemCount( void )
{
	return _dataItems.GetCount();
}

//-----------------------------------------------------------------------------
// Purpose: returns pointer to data the row holds
// Input  : itemIndex - 
// Output : KeyValues
//-----------------------------------------------------------------------------
Panel *COptionVarList::GetItem(int itemIndex)
{
	if ( itemIndex < 0 || itemIndex >= _dataItems.GetCount() )
		return NULL;

	return _dataItems[itemIndex]->panel;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : itemIndex - 
// Output : DATAITEM
//-----------------------------------------------------------------------------
COptionVarList::DATAITEM *COptionVarList::GetDataItem( int itemIndex )
{
	if ( itemIndex < 0 || itemIndex >= _dataItems.GetCount() )
		return NULL;

	return _dataItems[ itemIndex ];
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : index - 
//-----------------------------------------------------------------------------
void COptionVarList::RemoveItem(int itemIndex)
{
	DATAITEM *item = _dataItems[ itemIndex ];
	delete item->panel;
	delete item;
	_dataItems.RemoveElementAt(itemIndex);

	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: clears and deletes all the memory used by the data items
//-----------------------------------------------------------------------------
void COptionVarList::DeleteAllItems()
{
	for (int i = 0; i < _dataItems.GetCount(); i++)
	{
		if ( _dataItems[i] )
		{
			delete _dataItems[i]->panel;
		}
		delete _dataItems[i];
	}
	_dataItems.RemoveAll();

	InvalidateLayout();
}

void COptionVarList::ScrollToPos(int index, bool direct)
{
	if ( direct )
		_vbar->SetValue( index );
	else
		_vbar->SetValue( index * ( GetItem(index)->GetTall() ) );
}

int COptionVarList::GetScrollPos()
{
	return _vbar->GetValue();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionVarList::OnMouseWheeled(int delta)
{
	int val = _vbar->GetValue();
	val -= (delta * 3 * 5);
	_vbar->SetValue(val);
}

//-----------------------------------------------------------------------------
// Purpose: relayouts out the panel after any internal changes
//-----------------------------------------------------------------------------
void COptionVarList::PerformLayout()
{
	int wide, tall;
	GetSize( wide, tall );

	int vpixels = computeVPixelsNeeded();
	int yScaled = 150;

	//!! need to make it recalculate scroll positions
	_vbar->SetVisible(true);
	_vbar->SetEnabled(false);
	_vbar->SetBgColor(Color(0, 0, 0, 0));
	_vbar->GetSlider()->SetBgColor(Color(0, 0, 0, 0));
	_vbar->SetBorder(nullptr);
	_vbar->SetScrollbarButtonsVisible(false);
	_vbar->UseImages(nullptr, nullptr, nullptr, nullptr);
	_vbar->SetRange( 0, vpixels - tall + vgui::scheme()->GetProportionalScaledValue( yScaled ) );
	_vbar->SetRangeWindow( vgui::scheme()->GetProportionalScaledValue( yScaled ) /*vpixels / 10*/ );
	_vbar->SetButtonPressedScrollValue( vgui::scheme()->GetProportionalScaledValue( yScaled ) );
	_vbar->SetPos(wide - 20, _sliderYOffset);
	_vbar->SetSize(18, tall - 2 - _sliderYOffset);
	_vbar->InvalidateLayout();

	int top = _vbar->GetValue();

	_embedded->SetPos( 0, -top );
	_embedded->SetSize( wide-20, vpixels );

	// Now lay out the controls on the embedded panel
	int y = 0;
	int h = 0;
	for ( int i = 0; i < _dataItems.GetCount(); i++, y += h )
	{
		DATAITEM *item = _dataItems[ i ];
		if ( !item || !item->panel )
			continue;

		h = item->panel->GetTall();
		item->panel->SetBounds( 8, y, wide-36, h );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionVarList::PaintBackground()
{
	Panel::PaintBackground();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *inResourceData - 
//-----------------------------------------------------------------------------
void COptionVarList::ApplySchemeSettings(IScheme *pScheme)
{
	Panel::ApplySchemeSettings(pScheme);

	SetBgColor(GetSchemeColor("Label.BgColor", GetBgColor(), pScheme));
}

void COptionVarList::OnSliderMoved( int position )
{
	InvalidateLayout();
	Repaint();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void COptionVarList::SetSliderYOffset( int pixels )
{
	_sliderYOffset = pixels;
}




//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
COptionVarData::COptionVarData( Panel *parent, char const *panelName )
: Panel( parent, panelName )
{
	pControl = NULL;
	pPrompt = NULL;

	m_help = "";
	m_help_img = "";
	m_video = "";

	SetPaintBackgroundEnabled( false );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void COptionVarData::OnSizeChanged( int wide, int tall )
{
	int inset = 4;

	if ( pPrompt )
	{
		int w = wide / 2;

		if ( pControl )
		{
			pControl->SetBounds( w + 20, inset, w - 20, tall - 2 * inset );
		}
		pPrompt->SetBounds( 0, inset, w + 20, tall - 2 * inset  );
	}
	else
	{
		if ( pControl )
		{
			pControl->SetBounds( 0, inset, wide, tall - 2 * inset  );
		}
	}
}
