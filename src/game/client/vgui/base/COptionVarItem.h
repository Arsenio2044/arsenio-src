
#ifndef __NEWGAMEOPTIONS_VARITEM_H__
#define __NEWGAMEOPTIONS_VARITEM_H__

#include <vgui_controls/Label.h>
#include <stdstring.h>

class COptionHiddenButton;
class COptionSlider;

typedef struct IVoiceTweak_s IVoiceTweak;

struct OptionItemValues_t
{
	std::string name;
	std::string value;
};

class COptionVarItem : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( COptionVarItem, vgui::Panel );
public:
	enum ButtonBinding_t
	{
		BIND_OK = 0,
//		BIND_ERROR_SAMEBUTTON,	// No longer used, as it removes the same bind, if we are trying to bind it to the same key.
		BIND_ERROR_INPUT_KEBOARD,
		BIND_ERROR_INPUT_CONTROLLER,
		BIND_ERROR_CONTROLLER_BIND
	};

	enum ButtonInputState_t
	{
		BUTTON_PRESS = 0,	// Execute concommand, or increase if we can't execute concommand
		BUTTON_LEFT,		// Decrease
		BUTTON_RIGHT		// Increase
	};

	enum SelectedState_t
	{
		STATE_NONE = 0,
		STATE_HOVER,
		STATE_BUTTON
	};

	COptionVarItem(vgui::Panel* parent, char const* panelName, const char *szLabel, vgui::Panel* pActionSignalTarget, const char* szCommand);
	~COptionVarItem();
	void SetValue(int input);
	void CheckConvar(ConVar *cvar_ref); // Used to check special vars
	int GetValue() { return m_iCurrent; }
	void GetStringValue( char *buf, int len );
	void SetStringValue( const char *buf );
	void ClearList();
	int AddItem(const char *szText, std::string value = "");	// We only need text, as we will read the value id for our cvars
	void Paint() override;
	void DrawBackground(int w, int t);
	void DrawValueChoices(int pos, int posx, int wide);
	void SetActive(bool state) { m_Active = state; }
	void SetRestart(bool state) { m_RestartRequired = state; }
	void SetValueText(const char* szText);
	void SetValueText(const wchar_t* szText);
	void OnCommand(const char* command);
	void OnThink();
	void CurrentPosChange(bool increase);
	void SetTitle(bool state, bool stateSS);
	void SetKeyBind(const std::string& bind);
	void SetController(const std::string& bind) { m_szController = bind; }
	void SetCvar(const std::string& bind) { m_szCvar = bind; }
	void SetCustomDivide(int w) { m_iCustomDivide = w; }
	std::string GetCvar() const { return m_szCvar; }
	void SetSpecial(const std::string& bind, const std::string &cvar);
	void OverrideChoicesCommand(const char* szCommand);
	void SetSlider(const std::string& szmin, const std::string& szmax, const std::string& szstep, const std::string& szvar, bool displayvalue, bool displayfloat, bool inversefill, int floatformat, bool restartneeded );

	ButtonBinding_t TryApplyKeyBind( const ButtonCode_t &bind, const char *szKey );
	void RebuildLabel();
	void GrabButtonBind();
	bool IsValidController( ButtonCode_t &bind, int controller, bool binding = false );
	bool IsCorrectController( ButtonCode_t &code, int controller, bool binding );
	void SwitchButtonBind( ButtonCode_t &input, const ButtonCode_t &code1, const ButtonCode_t &code2 );
	void SetKeyIcon( const ButtonCode_t &code );

	bool IsCursorMoving();

	bool IsSpecial(const std::string& special);
	void ApplyChanges(const char *cvar);
	void ApplyDefault(const char *cvar);

	int GetSelectedValue( int val ) const;
	int GetSelectedValue( std::string val ) const;
	std::string GetCurrentValue( int val ) const;
	std::string GetCurrentName( int val ) const;

	bool RestartRequired() const { return m_RestartRequired; }

	// Specials
	void SetResolutionList(int asp, bool bWindowed);
	void GetResSize(int &w, int &t);

	SelectedState_t GetSelected() { return m_iSelectedState; }
	bool IsInButtonState() { return (m_iSelectedState == STATE_BUTTON); }

	// Used by keyboard / controller input support
	void SetSelected(SelectedState_t updateHelp = STATE_HOVER);
	void SetInputState(ButtonInputState_t state);

	bool IsTitle() { return m_bTitle; }
	bool IsBindingKey() { return m_bInCaptureMode; }
	bool IsTyping();
private:
	void		SetCamTexture( COptionHiddenButton *pBtn, int id, int x, int y, int w, int h );
	void		ApplySchemeSettings(vgui::IScheme *pScheme);
	void		StartTestMicrophone();
	void		EndTestMicrophone();

	vgui::ImagePanel* m_pMicMeter;
	vgui::ImagePanel* m_pMicMeter2;
	vgui::ImagePanel* m_pMicMeterIndicator;
	vgui::TextEntry* m_pStringInput;
	ButtonCode_t m_bcButton;
	SelectedState_t m_iSelectedState;

	int m_sx; // Saved mouse x pos
	int m_sy; // Saved mouse y pos

private:
	int m_iCustomDivide;

	std::string m_szCvar;
	std::string m_szSpecial;
	std::string m_szKeyBind;
	std::string m_szController;
	bool m_bTitle;
	bool m_Active;
	bool m_RestartRequired;
	vgui::Label* m_label_text;
	vgui::Label* m_label_value;
	CUtlVector<OptionItemValues_t> m_list;
	COptionHiddenButton *pHiddenButton;
	COptionHiddenButton *pChoicesButton;
	COptionSlider *pSlider;
	int m_iCurrent;
	bool m_bInCaptureMode;
	float m_flRefreshingTime;

	COptionHiddenButton* pCamLeft;
	COptionHiddenButton* pCamRight;
};

#endif