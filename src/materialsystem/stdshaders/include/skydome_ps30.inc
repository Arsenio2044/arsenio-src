// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#ifndef SKYDOME_PS30_H
#define SKYDOME_PS30_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class skydome_ps30_Static_Index
{
	unsigned int m_nCONVERT_TO_SRGB : 1;
#ifdef _DEBUG
	bool m_bCONVERT_TO_SRGB : 1;
#endif	// _DEBUG
public:
	void SetCONVERT_TO_SRGB( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nCONVERT_TO_SRGB = i;
#ifdef _DEBUG
		m_bCONVERT_TO_SRGB = true;
#endif	// _DEBUG
	}

	skydome_ps30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nCONVERT_TO_SRGB = 0;
#ifdef _DEBUG
		m_bCONVERT_TO_SRGB = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bCONVERT_TO_SRGB );
		return ( 2 * m_nCONVERT_TO_SRGB ) + 0;
	}
};

#define shaderStaticTest_skydome_ps30 psh_forgot_to_set_static_CONVERT_TO_SRGB


class skydome_ps30_Dynamic_Index
{
	unsigned int m_nRENDER_SKY : 2;
#ifdef _DEBUG
	bool m_bRENDER_SKY : 1;
#endif	// _DEBUG
public:
	void SetRENDER_SKY( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nRENDER_SKY = i;
#ifdef _DEBUG
		m_bRENDER_SKY = true;
#endif	// _DEBUG
	}

	skydome_ps30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nRENDER_SKY = 0;
#ifdef _DEBUG
		m_bRENDER_SKY = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bRENDER_SKY );
		return ( 1 * m_nRENDER_SKY ) + 0;
	}
};

#define shaderDynamicTest_skydome_ps30 psh_forgot_to_set_dynamic_RENDER_SKY


#endif	// SKYDOME_PS30_H