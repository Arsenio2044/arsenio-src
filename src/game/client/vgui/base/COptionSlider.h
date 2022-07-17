
#ifndef __NEWGAMEOPTIONS_SLIDER_H__
#define __NEWGAMEOPTIONS_SLIDER_H__

#include "L4D360UI/CONTUI.h"

class COptionSlider : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( COptionSlider, vgui::Panel );
public:
	// Create our options tab, and tell it which file it should read from.
	COptionSlider( Panel* parent, const char *szName );
	~COptionSlider();

	float GetStepSize() { return m_stepSize; }
	float GetMin() { return m_min; }
	float GetMax() { return m_max; }
	float GetCurrentValue() { return m_curValue; }
	bool GetInversed() { return m_inverse; }

	void SetCurrentValue(float value, bool bReset = false);
	float Increment(float stepSize = 1.0f);
	float Decrement(float stepSize = 1.0f);

	void OnMousePressed(vgui::MouseCode code) override;
	void OnMouseReleased(vgui::MouseCode code) override;
	void OnCursorEntered() override;
	void OnCursorExited() override;
	void HandleMouseInput(bool bDrag);
	void ValidateMousePos();

	void Paint();

	bool GetDisplayValue() { return m_bDisplayValue; }

	void SetDisplayValue(bool val) { m_bDisplayValue = val; }
	void SetDisplayInFloat(bool val) { m_bDisplayInFloat = val; }
	void SetFloatFormat(int val) { m_iFloatFormat = val; }
	void SetConCommand(const char* conCommand);
	void SetStepSize(float stepSize) { m_stepSize = stepSize; }
	void SetMin(float min);
	void SetMax(float max);
	void SetInverse(bool inverse) { m_inverse = inverse; }
	void SetRestart(bool inverse) { m_RestartRequired = inverse; }

	float UpdateProgressBar();

	void Reset();
	void ApplyChanges();
	void ApplyDefault();
private:
	void DrawBorders();
	void DrawBackground();
	void DrawProgressBar();

	bool	m_bDragging;
	bool	m_inverse;

	bool	m_bDisplayValue;
	bool	m_bDisplayInFloat;
	int		m_iFloatFormat;

	float	m_min;
	float	m_max;
	float	m_curValue;
	float	m_stepSize;

	float	m_flprogress;

	Color m_focusColor;
	Color m_unfocusColor;

	std::string m_cvar;
	bool m_RestartRequired;
};

#endif