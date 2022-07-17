
#ifndef __NEWGAMEOPTIONS_VARHELP_H__
#define __NEWGAMEOPTIONS_VARHELP_H__

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/Panel.h>
#include <stdstring.h>

#if defined( BINK_VIDEO )
	#include "avi/ibik.h"
#elif defined( MPEG_VIDEO )
	#include "avi/iffmpeg.h"
#endif

#if defined( BINK_VIDEO ) || defined( MPEG_VIDEO )
	#ifndef VIDEO_ENABLED
		#define VIDEO_ENABLED 1
	#endif
#endif

class COptionVarHelp : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(COptionVarHelp, vgui::Panel);
public:
	COptionVarHelp(vgui::Panel* parent, char const* panelName);
	~COptionVarHelp();
	void SetImage( const char *szImage, bool phonebg = false );
	void SetImageFrame( int frame );
	void SetVideo( const char *szVideo );
	void SetText( const std::string &str );
	void Paint() override;
	void ApplySchemeSettings(vgui::IScheme* pScheme) override;
private:
	void LocalizationFixes( std::string &strMessage );
	bool m_bPhoneBG;
	vgui::ImagePanel* m_Image;
	vgui::RichText* m_Help;
#if defined( BINK_VIDEO )
	BIKMaterial_t	m_VideoHandle;
	static const BIKMaterial_t s_InvalidVideoHandle = BIKHANDLE_INVALID;
	IBik* m_pVideoPlayer;
#elif defined( MPEG_VIDEO )
	FFMPEGMaterial_t	m_VideoHandle;
	static const FFMPEGMaterial_t s_InvalidVideoHandle = BIKHANDLE_INVALID;
	IFFMpeg* m_pVideoPlayer;
#endif
#if defined( VIDEO_ENABLED )
protected:
	bool DrawVideo(int x, int y, int w, int t);
	void CalculateMovieParameters();
	void StopVideo();
	IMaterial* m_pMovieMaterial;
	float m_flU0, m_flV0, m_flU1, m_flV1;
#endif
};

#endif