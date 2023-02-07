﻿//========= Copyright � 1996-2010, Valve Corporation, All rights reserved. ============//
//
// Purpose: global dynamic light.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//------------------------------------------------------------------------------
// FIXME: This really should inherit from something	more lightweight
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Purpose : Sunlight shadow control entity
//------------------------------------------------------------------------------
class CGlobalLight : public CBaseEntity
{
public:
	DECLARE_CLASS(CGlobalLight, CBaseEntity);

	CGlobalLight();

	void Spawn(void);
	bool KeyValue(const char* szKeyName, const char* szValue);
	virtual bool GetKeyValue(const char* szKeyName, char* szValue, int iMaxLen);
	int  UpdateTransmitState();

	// Inputs
	void	InputSetAngles(inputdata_t& inputdata);
	void	InputEnable(inputdata_t& inputdata);
	void	InputDisable(inputdata_t& inputdata);
	void	InputSetTexture(inputdata_t& inputdata);
	void	InputSetEnableShadows(inputdata_t& inputdata);
	void	InputSetLightColor(inputdata_t& inputdata);

	void	InputSetEnableDynamicSky(inputdata_t& inputdata);
	void	InputSetTimescale(inputdata_t& inputdata);
	void	InputSetTime(inputdata_t& inputdata);

	virtual int	ObjectCaps(void) { return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

private:
	CNetworkVector(m_shadowDirection);

	CNetworkVar(bool, m_bEnabled);
	bool m_bStartDisabled;

	CNetworkString(m_TextureName, MAX_PATH);
	CNetworkVector(m_LinearFloatLightColor);
	CNetworkVector(m_LinearFloatAmbientColor);
	CNetworkVar(float, m_flColorTransitionTime);
	CNetworkVar(float, m_flSunDistance);
	CNetworkVar(float, m_flFOV);
	CNetworkVar(float, m_flNearZ);
	CNetworkVar(float, m_flNorthOffset);
	CNetworkVar(bool, m_bEnableShadows);
	CNetworkVar(bool, m_bEnableVolumetrics);

	CNetworkVar(bool, m_bEnableDynamicSky);
	CNetworkVar(bool, m_bEnableTimeAngles);
	CNetworkVar(float, m_flDayNightTimescale);
	CNetworkVar(float, m_fTime);
};

LINK_ENTITY_TO_CLASS(env_global_light, CGlobalLight);
LINK_ENTITY_TO_CLASS(env_environment, CGlobalLight);

BEGIN_DATADESC(CGlobalLight)

DEFINE_KEYFIELD(m_bEnabled, FIELD_BOOLEAN, "enabled"),
DEFINE_KEYFIELD(m_bStartDisabled, FIELD_BOOLEAN, "StartDisabled"),
DEFINE_AUTO_ARRAY_KEYFIELD(m_TextureName, FIELD_CHARACTER, "texturename"),
DEFINE_KEYFIELD(m_flSunDistance, FIELD_FLOAT, "distance"),
DEFINE_KEYFIELD(m_flFOV, FIELD_FLOAT, "fov"),
DEFINE_KEYFIELD(m_flNearZ, FIELD_FLOAT, "nearz"),
DEFINE_KEYFIELD(m_flNorthOffset, FIELD_FLOAT, "northoffset"),
DEFINE_KEYFIELD(m_bEnableShadows, FIELD_BOOLEAN, "enableshadows"),
DEFINE_KEYFIELD(m_bEnableVolumetrics, FIELD_BOOLEAN, "enablevolumetrics"),
DEFINE_KEYFIELD(m_flColorTransitionTime, FIELD_FLOAT, "colortransitiontime"),

DEFINE_KEYFIELD(m_bEnableDynamicSky, FIELD_BOOLEAN, "enabledynamicsky"),
DEFINE_KEYFIELD(m_bEnableTimeAngles, FIELD_BOOLEAN, "usetimeforangles"),

DEFINE_KEYFIELD(m_flDayNightTimescale, FIELD_FLOAT, "timescale"),
DEFINE_KEYFIELD(m_fTime, FIELD_FLOAT, "time"),

// Inputs
DEFINE_INPUT(m_flSunDistance, FIELD_FLOAT, "SetDistance"),
DEFINE_INPUT(m_flFOV, FIELD_FLOAT, "SetFOV"),
DEFINE_INPUT(m_flNearZ, FIELD_FLOAT, "SetNearZDistance"),
DEFINE_INPUT(m_flNorthOffset, FIELD_FLOAT, "SetNorthOffset"),

DEFINE_INPUTFUNC(FIELD_COLOR32, "LightColor", InputSetLightColor),
DEFINE_INPUTFUNC(FIELD_STRING, "SetAngles", InputSetAngles),
DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),
DEFINE_INPUTFUNC(FIELD_STRING, "SetTexture", InputSetTexture),
DEFINE_INPUTFUNC(FIELD_BOOLEAN, "EnableShadows", InputSetEnableShadows),
DEFINE_INPUTFUNC(FIELD_BOOLEAN, "EnableDynamicSky", InputSetEnableDynamicSky),
DEFINE_INPUTFUNC(FIELD_FLOAT, "EnableSetTimescale", InputSetTimescale),
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetTime", InputSetTime),

DEFINE_FIELD(m_LinearFloatLightColor, FIELD_VECTOR),
DEFINE_FIELD(m_LinearFloatAmbientColor, FIELD_VECTOR),

END_DATADESC()


IMPLEMENT_SERVERCLASS_ST_NOBASE(CGlobalLight, DT_GlobalLight)
SendPropVector(SENDINFO(m_shadowDirection), -1, SPROP_NOSCALE),
SendPropBool(SENDINFO(m_bEnabled)),
SendPropString(SENDINFO(m_TextureName)),
SendPropVector(SENDINFO(m_LinearFloatLightColor)),
SendPropVector(SENDINFO(m_LinearFloatAmbientColor)),
SendPropFloat(SENDINFO(m_flColorTransitionTime)),
SendPropFloat(SENDINFO(m_flSunDistance), 0, SPROP_NOSCALE),
SendPropFloat(SENDINFO(m_flFOV), 0, SPROP_NOSCALE),
SendPropFloat(SENDINFO(m_flNearZ), 0, SPROP_NOSCALE),
SendPropFloat(SENDINFO(m_flNorthOffset), 0, SPROP_NOSCALE),
SendPropBool(SENDINFO(m_bEnableShadows)),

SendPropBool(SENDINFO(m_bEnableDynamicSky)),
SendPropFloat(SENDINFO(m_flDayNightTimescale)),
SendPropFloat(SENDINFO(m_fTime)),
END_SEND_TABLE()


CGlobalLight::CGlobalLight()
{
#if defined( _X360 )
	Q_strcpy(m_TextureName.GetForModify(), "effects/flashlight_border");
#else
	Q_strcpy(m_TextureName.GetForModify(), "effects/flashlight001");
#endif
	m_flColorTransitionTime = 0.5f;
	m_flSunDistance = 10000.0f;
	m_LinearFloatLightColor.Init(1.0f, 1.0f, 1.0f);
	m_flFOV = 5.0f;
	m_bEnableShadows = false;

	m_bEnableDynamicSky = false;
	m_bEnableTimeAngles = false;
	m_flDayNightTimescale = 1.0f;
	m_fTime = 0.0f;
}


//------------------------------------------------------------------------------
// Purpose : Send even though we don't have a model
//------------------------------------------------------------------------------
int CGlobalLight::UpdateTransmitState()
{
	// ALWAYS transmit to all clients.
	return SetTransmitState(FL_EDICT_ALWAYS);
}

extern void UTIL_ColorStringToLinearFloatColor(Vector& color, const char* pString);

bool CGlobalLight::KeyValue(const char* szKeyName, const char* szValue)
{
	if (FStrEq(szKeyName, "color"))
	{
		Vector tmp;
		UTIL_ColorStringToLinearFloatColor(tmp, szValue);
		m_LinearFloatLightColor = tmp;
	}
	else if (FStrEq(szKeyName, "ambient"))
	{
		Vector tmp;
		UTIL_ColorStringToLinearFloatColor(tmp, szValue);
		m_LinearFloatAmbientColor = tmp;
	}
	else if (FStrEq(szKeyName, "angles"))
	{
		QAngle angles;
		UTIL_StringToVector(angles.Base(), szValue);
		if (angles == vec3_angle)
		{
			angles.Init(80, 30, 0);
		}
		Vector vForward;
		AngleVectors(angles, &vForward);
		m_shadowDirection = vForward;
		return true;
	}
	else if (FStrEq(szKeyName, "texturename"))
	{
#if defined( _X360 )
		if (Q_strcmp(szValue, "effects/flashlight001") == 0)
		{
			// Use this as the default for Xbox
			Q_strcpy(m_TextureName.GetForModify(), "effects/flashlight_border");
		}
		else
		{
			Q_strcpy(m_TextureName.GetForModify(), szValue);
		}
#else
		Q_strcpy(m_TextureName.GetForModify(), szValue);
#endif
	}

	return BaseClass::KeyValue(szKeyName, szValue);
}

bool CGlobalLight::GetKeyValue(const char* szKeyName, char* szValue, int iMaxLen)
{
	if (FStrEq(szKeyName, "color"))
	{
		//Q_snprintf( szValue, iMaxLen, "%d %d %d %d", m_LightColor.GetR(), m_LightColor.GetG(), m_LightColor.GetB(), m_LightColor.GetA() );
		return true;
	}
	else if (FStrEq(szKeyName, "texturename"))
	{
		Q_snprintf(szValue, iMaxLen, "%s", m_TextureName.Get());
		return true;
	}
	return BaseClass::GetKeyValue(szKeyName, szValue, iMaxLen);
}

//------------------------------------------------------------------------------
// Purpose :
//------------------------------------------------------------------------------
void CGlobalLight::Spawn(void)
{
	Precache();
	SetSolid(SOLID_NONE);

	if (m_bStartDisabled)
	{
		m_bEnabled = false;
	}
	else
	{
		m_bEnabled = true;
	}
}

//------------------------------------------------------------------------------
// Input values
//------------------------------------------------------------------------------
void CGlobalLight::InputSetAngles(inputdata_t& inputdata)
{
	const char* pAngles = inputdata.value.String();

	QAngle angles;
	UTIL_StringToVector(angles.Base(), pAngles);

	Vector vTemp;
	AngleVectors(angles, &vTemp);
	m_shadowDirection = vTemp;
}

//------------------------------------------------------------------------------
// Purpose : Input handlers
//------------------------------------------------------------------------------
void CGlobalLight::InputEnable(inputdata_t& inputdata)
{
	m_bEnabled = true;
}

void CGlobalLight::InputDisable(inputdata_t& inputdata)
{
	m_bEnabled = false;
}

void CGlobalLight::InputSetTexture(inputdata_t& inputdata)
{
	Q_strcpy(m_TextureName.GetForModify(), inputdata.value.String());
}

void CGlobalLight::InputSetEnableShadows(inputdata_t& inputdata)
{
	m_bEnableShadows = inputdata.value.Bool();
}

void CGlobalLight::InputSetLightColor(inputdata_t& inputdata)
{
	//m_LightColor = inputdata.value.Color32();
}

void CGlobalLight::InputSetEnableDynamicSky(inputdata_t& inputdata)
{
	m_bEnableDynamicSky = inputdata.value.Bool();
}

void CGlobalLight::InputSetTimescale(inputdata_t& inputdata)
{
	m_flDayNightTimescale = inputdata.value.Float();
}

void CGlobalLight::InputSetTime(inputdata_t& inputdata)
{
	m_fTime = inputdata.value.Float();
}
