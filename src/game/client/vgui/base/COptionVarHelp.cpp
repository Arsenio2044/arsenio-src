#include "COptionVarHelp.h"

#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/RichText.h>

#include <regex>
#include <vgui/ILocalize.h>
#include "materialsystem/imesh.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// From consoledialog.cpp
std::string ReplaceColors(const char *str, std::string strDefaultColor);
bool FStrEq_VGUI(const char* sz1, const char* sz2);
std::string ColorToHEX(int num);

COptionVarHelp::COptionVarHelp(vgui::Panel* parent, char const* panelName) :
	BaseClass( parent, panelName )
{
	m_bPhoneBG = false;

	m_Image = new vgui::ImagePanel( this, "img" );
	m_Image->SetShouldScaleImage( true );
	m_Image->SetVisible( false );

	m_Image->SetPos(0, 0);
	m_Image->SetSize(0, 0);

	m_Help = new vgui::RichText( this, "text" );
	m_Help->SetVisible( false );

#if defined( VIDEO_ENABLED )
	m_VideoHandle = s_InvalidVideoHandle;
#endif
}

COptionVarHelp::~COptionVarHelp()
{
#if defined( VIDEO_ENABLED )
	StopVideo();
#endif
}

void COptionVarHelp::SetImage(const char* szImage, bool phonebg )
{
	if ( !szImage )
	{
		m_Image->SetVisible( false );
		return;
	}
	if ( Q_stricmp( "", szImage ) == 0 )
	{
		m_Image->SetVisible( false );
		return;
	}
	m_Image->SetImage( szImage );
	m_Image->SetVisible( true );
	m_bPhoneBG = phonebg;
}

void COptionVarHelp::SetImageFrame(int frame)
{
	m_Image->SetFrame( frame );
}

void COptionVarHelp::SetVideo(const char* szVideo)
{
	if ( !szVideo )
	{
#if defined( VIDEO_ENABLED )
		StopVideo();
#endif
		return;
	}
	if ( Q_stricmp( "", szVideo ) == 0 )
	{
#if defined( VIDEO_ENABLED )
		StopVideo();
#endif
		return;
	}
#if defined( VIDEO_ENABLED )
#if defined ( BINK_VIDEO )
	m_pVideoPlayer = g_pBIK;
#elif defined ( MPEG_VIDEO )
	m_pVideoPlayer = g_pMPEG;
#endif

	if ( !m_pVideoPlayer )
	{
		m_VideoHandle = s_InvalidVideoHandle;
		return;
	}

	if ( m_VideoHandle != s_InvalidVideoHandle )
	{
		m_pVideoPlayer->DestroyMaterial( m_VideoHandle );
		m_VideoHandle = s_InvalidVideoHandle;
	}

	m_VideoHandle = m_pVideoPlayer->CreateMaterial( "VideoOptionHelpMaterial", szVideo, "GAME" );

	if ( m_VideoHandle == s_InvalidVideoHandle ) return;
	CalculateMovieParameters();
#endif
}

void COptionVarHelp::SetText(const std::string& str)
{
	std::string szMsg( str );
	LocalizationFixes( szMsg );

	// Allow for colored text
	char strmsg[4096];

	// Erase all previous text
	m_Help->SetText( "" );

	///=========================================
	// Add our initial color text
	///=========================================

	Color clr( 255, 255, 255, 255 );

	int r, g, b;
	r = clr.r();
	g = clr.g();
	b = clr.b();

	// If black colored, become grayish-white instead
	if ( r == 0 && g == 0 && b == 0 )
		r = g = b = 80;

	int rgbNum = ((r & 0xff) << 16)
		| ((g & 0xff) << 8)
		| (b & 0xff);
	std::string strInitialColor = "\x07";
	strInitialColor += ColorToHEX( rgbNum );

	if ( FStrEq_VGUI( szMsg.c_str(), "" ) )
		strInitialColor.clear();

	///=========================================
	///=========================================

	V_strcpy( strmsg, ReplaceColors( szMsg.c_str(), strInitialColor ).c_str() );

	// override it
	char *pmsg = strmsg;
	int bufSize = (strlen( pmsg ) + 1 ) * sizeof(wchar_t);
	wchar_t *wbuf = static_cast<wchar_t *>( _alloca( bufSize ) );
	if ( wbuf )
	{
		g_pVGuiLocalize->ConvertANSIToUnicode( pmsg, wbuf, bufSize );
		m_Help->InsertAndColorizeString( wbuf, false );
	}

	m_Help->SetVisible( true );
}

void COptionVarHelp::Paint()
{
	BaseClass::Paint();

	int w, t;
	GetSize(w, t);

	int xVid = vgui::scheme()->GetProportionalScaledValue( 10 );
	int yVid = vgui::scheme()->GetProportionalScaledValue( 10 );
	int wideVid = w - vgui::scheme()->GetProportionalScaledValue( 20 );
	int tallVid = vgui::scheme()->GetProportionalScaledValue( 190 );

	bool bDrawVideo =
#if defined( VIDEO_ENABLED )
		DrawVideo( xVid, yVid, wideVid, tallVid );
#else
		false;
#endif

	// Draw Video (if invalid, draw image)
	if ( !bDrawVideo )
	{
		bDrawVideo = m_Image->IsVisible();
		if ( bDrawVideo )
		{
			// If phone BG, then make it small, and center it too
			if ( m_bPhoneBG )
			{
				int phonesize = vgui::scheme()->GetProportionalScaledValue( 150 );
				int phonepos = (w / 2) - (phonesize / 2);
				m_Image->SetPos( phonepos, yVid );
				m_Image->SetSize( phonesize, tallVid );
			}
			else
			{
				m_Image->SetPos( xVid, yVid );
				m_Image->SetSize( wideVid, tallVid );
			}
		}
	}

	// if we draw the video (or image) move the richtext down a bit.
	if ( bDrawVideo )
	{
		yVid = tallVid + vgui::scheme()->GetProportionalScaledValue( 10 );
		tallVid = t - yVid - vgui::scheme()->GetProportionalScaledValue( 10 );
	}
	else
		tallVid = t - vgui::scheme()->GetProportionalScaledValue( 20 );

	m_Help->SetPos( xVid, yVid );
	m_Help->SetSize( wideVid, tallVid );
}

void COptionVarHelp::ApplySchemeSettings(vgui::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_Help->SetDrawTextOnly();
	m_Help->SetBgColor(Color(0, 0, 0, 0));
	m_Help->SetVerticalScrollbar( true, false );
	m_Help->SetUnusedScrollbarInvisible( true );
}

// Backported from Zombie Panic! Source, and then updated to suit Contagion lengthy messages
static void ZPUTILS_ReplaceLocalizationString( std::string &str, const std::string &input )
{
	std::string strFind( "<" + input + ">" );
	std::string newStr( g_pVGuiLocalizeV2->GetTranslationKey( input.c_str() ) );
	std::string::size_type pos = 0u;
	while ( (pos = str.find(strFind, pos) ) != std::string::npos ) {
		str.replace(pos, strFind.length(), newStr);
		pos += newStr.length();
	}
}

void COptionVarHelp::LocalizationFixes( std::string& strMessage )
{
	const std::string localization = strMessage;
	std::regex regValue( "<#(.*?)>" );
	std::smatch matches;

	std::string::const_iterator text_iter = localization.cbegin();
	while ( std::regex_search( text_iter, localization.end(), matches, regValue ) )
	{
		const std::string strValue( matches[0].first, matches[0].second );
		std::smatch result;
		if ( std::regex_search( strValue.begin(), strValue.end(), result, regValue ) )
		{
			std::string Result( result[1] );
			std::string Localize( "#" + Result.substr( 0, Result.size() ) );
			ZPUTILS_ReplaceLocalizationString( strMessage, Localize );
		}

		text_iter = matches[0].second;
	}
}

#if defined( VIDEO_ENABLED )
bool COptionVarHelp::DrawVideo(int x, int y, int w, int t)
{
	if ( m_VideoHandle == s_InvalidVideoHandle ) return false;
	if ( m_flU1 == 0.0f || m_flV1 == 0.0f )
		CalculateMovieParameters();
	if ( m_pVideoPlayer->ReadyForSwap( m_VideoHandle ) )
	{
		if ( m_pVideoPlayer->Update( m_VideoHandle ) == false )
		{
			StopVideo();
			return false;
		}
	}

	CMatRenderContextPtr pRenderContext( materials );

	pRenderContext->MatrixMode( MATERIAL_VIEW );
	pRenderContext->PushMatrix();
	pRenderContext->LoadIdentity();

	pRenderContext->MatrixMode( MATERIAL_PROJECTION );
	pRenderContext->PushMatrix();
	pRenderContext->LoadIdentity();

	IMaterial *pMaterial = m_pVideoPlayer->GetMaterial( m_VideoHandle );
	pRenderContext->Bind( pMaterial, NULL );

	CMeshBuilder meshBuilder;
	IMesh* pMesh = pRenderContext->GetDynamicMesh( true );
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

	float flLeftX = x;
	float flRightX = w-1;

	float flTopY = y;
	float flBottomY = t-1;

	float flLeftU = m_flU0;
	float flTopV = m_flV0;

	float flRightU = m_flU1 - ( 1.0f / (float) w );
	float flBottomV = m_flV1 - ( 1.0f / (float) t );

	int vx, vy, vw, vh;
	pRenderContext->GetViewport( vx, vy, vw, vh );

	flRightX = FLerp( -1, 1, 0, vw, flRightX );
	flLeftX = FLerp( -1, 1, 0, vw, flLeftX );
	flTopY = FLerp( 1, -1, 0, vh ,flTopY );
	flBottomY = FLerp( 1, -1, 0, vh, flBottomY );

	for ( int corner=0; corner<4; corner++ )
	{
		bool bLeft = (corner==0) || (corner==3);
		meshBuilder.Position3f( (bLeft) ? flLeftX : flRightX, (corner & 2) ? flBottomY : flTopY, 0.0f );
		meshBuilder.Normal3f( 0.0f, 0.0f, 1.0f );
		meshBuilder.TexCoord2f( 0, (bLeft) ? flLeftU : flRightU, (corner & 2) ? flBottomV : flTopV );
		meshBuilder.TangentS3f( 0.0f, 1.0f, 0.0f );
		meshBuilder.TangentT3f( 1.0f, 0.0f, 0.0f );
		meshBuilder.Color4f( 1.0f, 1.0f, 1.0f, 1.0f );
		meshBuilder.AdvanceVertex();
	}

	meshBuilder.End();
	pMesh->Draw();

	pRenderContext->MatrixMode( MATERIAL_VIEW );
	pRenderContext->PopMatrix();

	pRenderContext->MatrixMode( MATERIAL_PROJECTION );
	pRenderContext->PopMatrix();
	return true;
}


void COptionVarHelp::CalculateMovieParameters()
{
	if ( m_VideoHandle == s_InvalidVideoHandle ) return;

	m_flU0 = m_flV0 = 0.0f;
	m_pVideoPlayer->GetTexCoordRange( m_VideoHandle, &m_flU1, &m_flV1 );

	m_pMovieMaterial = m_pVideoPlayer->GetMaterial( m_VideoHandle );

	int nWidth, nHeight;
	m_pVideoPlayer->GetFrameSize( m_VideoHandle, &nWidth, &nHeight );

	float flFrameRatio = ( (float) GetWide() / (float) GetTall() );
	float flVideoRatio = ( (float) nWidth / (float) nHeight );

	if ( flVideoRatio > flFrameRatio )
	{
		// Width must be adjusted
		float flImageWidth = (float) GetTall() * flVideoRatio;
		const float flSpanScaled = ( m_flU1 - m_flU0 ) * GetWide() / flImageWidth;
		m_flU0 = ( m_flU1 - flSpanScaled ) / 2.0f;
		m_flU1 = m_flU0 + flSpanScaled;
	}
	else if ( flVideoRatio < flFrameRatio )
	{
		// Height must be adjusted
		float flImageHeight = (float) GetWide() * ( (float) nHeight / (float) nWidth );
		const float flSpanScaled = ( m_flV1 - m_flV0 ) * GetTall() / flImageHeight;
		m_flV0 = ( m_flV1 - flSpanScaled ) / 2.0f;
		m_flV1 = m_flV0 + flSpanScaled;
	}
}

void COptionVarHelp::StopVideo()
{
	if ( m_VideoHandle != s_InvalidVideoHandle )
	{
		m_pVideoPlayer->DestroyMaterial( m_VideoHandle );
		m_VideoHandle = s_InvalidVideoHandle;
	}
}
#endif