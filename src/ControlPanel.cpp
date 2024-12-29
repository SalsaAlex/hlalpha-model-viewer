//=============================================================================
//
// Copyright (c) 1999 by Mete Ciragan <mete@swissquake.ch>
// Copyright (c) 1999-2013 Tomas Slavotinek (aka baso88, GeckoN[CZ])
// E-mail: slavotinek@gmail.com
//
// GoldSrc Model Viewer (MDL v6)
//
// Purpose:
//
//=============================================================================

#include "ControlPanel.h"

#include "ViewerSettings.h"
#include "StudioRender.h"
#include "GlWindow.h"
#include "mdlviewer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "safestr.h"
#include "pathutils.h"
#include "mdlmath.h"

#include <mx/mx.h>
#include <mx/mxBmp.h>

bool swap3dfxgl( bool b );

ControlPanel * g_ControlPanel = 0;
bool g_bStopPlaying = false;
static int g_nCurrFrame = 0;


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
ControlPanel::ControlPanel( mxWindow * parent )
	: mxWindow( parent, 0, 0, 0, 0, "Control Panel", mxWindow::Normal )
{
	InitViewerSettings();

	// create tabcontrol with subdialog windows
	tab = new mxTab( this, 0, 0, 0, 0, IDC_TAB );
#ifdef WIN32
	SetWindowLong( ( HWND ) tab->getHandle(), GWL_EXSTYLE, WS_EX_CLIENTEDGE );
#endif

	mxWindow * wRender = new mxWindow( this, 0, 0, 0, 0 );
	tab->add( wRender, "Render" );
	cRenderMode = new mxChoice( wRender, 5, 5, 100, 22, IDC_RENDERMODE );
	cRenderMode->add( "Wireframe" );
	cRenderMode->add( "Flatshaded" );
	cRenderMode->add( "Smoothshaded" );
	cRenderMode->add( "Textured" );
	cRenderMode->select( 3 );
	mxToolTip::add( cRenderMode, "Select Render Mode" );
	slTransparency = new mxSlider( wRender, 5, 28, 100, 18, IDC_TRANSPARENCY );
	slTransparency->setValue( 100 );
	mxToolTip::add( slTransparency, "Model Transparency" );
	cbGround = new mxCheckBox( wRender, 110, 5, 150, 20, "Ground", IDC_GROUND );
	cbMirror = new mxCheckBox( wRender, 110, 25, 150, 20, "Mirror Model On Ground", IDC_MIRROR );
	cbBackground = new mxCheckBox( wRender, 110, 45, 150, 20, "Background", IDC_BACKGROUND );
	//mxCheckBox *cbHitBoxes = new mxCheckBox (wRender, 5, 65, 150, 20, "Hit Boxes", IDC_HITBOXES);
	mxCheckBox * cbBones = new mxCheckBox( wRender, 5, 45, 100, 20, "Bones", IDC_BONES );
	//mxCheckBox *cbAttachments = new mxCheckBox (wRender, 5, 45, 100, 20, "Attachments", IDC_ATTACHMENTS);

	mxWindow * wSequence = new mxWindow( this, 0, 0, 0, 0 );
	tab->add( wSequence, "Sequence" );
	cSequence = new mxChoice( wSequence, 5, 5, 200, 22, IDC_SEQUENCE );
	mxToolTip::add( cSequence, "Select Sequence" );
	slSpeedScale = new mxSlider( wSequence, 5, 32, 200, 18, IDC_SPEEDSCALE );
	slSpeedScale->setRange( 0, 200 );
	slSpeedScale->setValue( 40 );
	mxToolTip::add( slSpeedScale, "Speed Scale" );
	tbStop = new mxToggleButton( wSequence, 5, 55, 60, 22, "Stop", IDC_STOP );
	mxToolTip::add( tbStop, "Stop Playing" );
	bPrevFrame = new mxButton( wSequence, 70, 55, 30, 22, "<<", IDC_PREVFRAME );
	bPrevFrame->setEnabled( false );
	mxToolTip::add( bPrevFrame, "Prev Frame" );
	leFrame = new mxLineEdit( wSequence, 105, 55, 50, 22, "", IDC_FRAME );
	leFrame->setEnabled( false );
	mxToolTip::add( leFrame, "Set Frame" );
	bNextFrame = new mxButton( wSequence, 160, 55, 30, 22, ">>", IDC_NEXTFRAME );
	bNextFrame->setEnabled( false );
	mxToolTip::add( bNextFrame, "Next Frame" );

	mxWindow * wBody = new mxWindow( this, 0, 0, 0, 0 );
	tab->add( wBody, "Body" );
	cBodypart = new mxChoice( wBody, 5, 5, 100, 22, IDC_BODYPART );
	mxToolTip::add( cBodypart, "Choose a bodypart" );
	cSubmodel = new mxChoice( wBody, 110, 5, 100, 22, IDC_SUBMODEL );
	mxToolTip::add( cSubmodel, "Choose a submodel of current bodypart" );
	cController = new mxChoice( wBody, 5, 30, 100, 22, IDC_CONTROLLER );
	mxToolTip::add( cController, "Choose a bone controller" );
	slController = new mxSlider( wBody, 105, 32, 100, 18, IDC_CONTROLLERVALUE );
	slController->setRange( 0, 45 );
	mxToolTip::add( slController, "Change current bone controller value" );
	lModelInfo1 = new mxLabel( wBody, 220, 5, 120, 100, "No Model." );
	lModelInfo2 = new mxLabel( wBody, 340, 5, 120, 100, "" );
	cSkin = new mxChoice( wBody, 5, 55, 100, 22, IDC_SKINS );
	mxToolTip::add( cSkin, "Choose a skin family" );

	mxWindow * wTexture = new mxWindow( this, 0, 0, 0, 0 );
	tab->add( wTexture, "Texture" );
	cTextures = new mxChoice( wTexture, 5, 5, 150, 22, IDC_TEXTURES );
	mxToolTip::add( cTextures, "Choose a texture" );

	new mxButton( wTexture, 160, 5, 75, 18, "Export", IDC_EXPORTTEXTURE );
	new mxButton( wTexture, 160, 25, 75, 18, "Import", IDC_IMPORTTEXTURE );
	new mxButton( wTexture, 160, 45, 75, 18, "Save Model", IDC_SAVEMODEL );
	lTexSize = new mxLabel( wTexture, 162, 70, 150, 18, "Width x Height" );
	cbChrome = new mxCheckBox( wTexture, 5, 30, 150, 22, "Chrome Effect", IDC_CHROME );
	mxToolTip::add( new mxSlider( wTexture, 5, 57, 150, 18, IDC_TEXTURESCALE ), "Scale texture size" );

#ifdef WIN32
	mxWindow * wFullscreen = new mxWindow( this, 0, 0, 0, 0 );
	tab->add( wFullscreen, "Fullscreen" );

	// Create widgets for the Fullscreen Tab
	mxLabel * lResolution = new mxLabel( wFullscreen, 5, 7, 50, 18, "Resolution" );
	leWidth = new mxLineEdit( wFullscreen, 5, 5, 50, 22, "800" );
	mxLabel * lX = new mxLabel( wFullscreen, 65, 7, 22, 22, "x" );
	leHeight = new mxLineEdit( wFullscreen, 82, 5, 50, 22, "600" );
	//cb3dfxOpenGL = new mxCheckBox (wFullscreen, 5, 30, 130, 22, "3Dfx OpenGL");
	mxButton * bView = new mxButton( wFullscreen, 140, 5, 75, 22, "Fullscreen!", IDC_FULLSCREEN );
#endif

	g_ControlPanel = this;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
ControlPanel::~ControlPanel( void )
{
	// TOMAS: Nothing to destruct? Hmm...
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int ControlPanel::handleEvent( mxEvent * event )
{
	static char str[128];

	if ( event->event == mxEvent::Size )
	{
		tab->setBounds( 0, 0, event->width, event->height );
		return 1;
	}

	switch ( event->action )
	{
	case IDC_TAB:
	{
		bool bTexTab = tab->getSelectedIndex() == 3;

		if ( bTexTab )
		{
			setTexture( g_viewerSettings.texture );
		}

		g_viewerSettings.showTexture = bTexTab;
	}
	break;

	case IDC_RENDERMODE:
	{
		int index = cRenderMode->getSelectedIndex();
		if ( index >= 0 )
		{
			setRenderMode( index );
		}
	}
	break;

	case IDC_TRANSPARENCY:
	{
		int value = slTransparency->getValue();
		g_viewerSettings.transparency = ( float ) value / 100.0f;
	}
	break;

	case IDC_GROUND:
		setShowGround( ( ( mxCheckBox * ) event->widget )->isChecked() );
		break;

	case IDC_MIRROR:
		setMirror( ( ( mxCheckBox * ) event->widget )->isChecked() );
		break;

	case IDC_BACKGROUND:
		setShowBackground( ( ( mxCheckBox * ) event->widget )->isChecked() );
		break;

		/*case IDC_HITBOXES:
			g_viewerSettings.showHitBoxes = ((mxCheckBox *) event->widget)->isChecked ();
			break;*/

	case IDC_BONES:
		g_viewerSettings.showBones = ( ( mxCheckBox * ) event->widget )->isChecked();
		break;

		/*case IDC_ATTACHMENTS:
			g_viewerSettings.showAttachments = ((mxCheckBox *) event->widget)->isChecked ();
			break;*/

	case IDC_SEQUENCE:
	{
		int index = cSequence->getSelectedIndex();
		if ( index >= 0 )
		{
			setSequence( index );
		}
	}
	break;

	case IDC_SPEEDSCALE:
	{
		int v = ( ( mxSlider * ) event->widget )->getValue();
		g_viewerSettings.speedScale = ( float )( v * 5 ) / 200.0f;
	}
	break;

	case IDC_STOP:
	{
		if ( tbStop->isChecked() )
		{
			tbStop->setLabel( "Play" );
			g_bStopPlaying = true;
			g_nCurrFrame = g_studioRender.SetFrame( -1 );
			snprintf( str, sizeof( str ), "%d", g_nCurrFrame );
			leFrame->setLabel( str );
			bPrevFrame->setEnabled( true );
			leFrame->setEnabled( true );
			bNextFrame->setEnabled( true );
		}
		else
		{
			tbStop->setLabel( "Stop" );
			g_bStopPlaying = false;
			bPrevFrame->setEnabled( false );
			leFrame->setEnabled( false );
			bNextFrame->setEnabled( false );
		}
	}
	break;

	case IDC_PREVFRAME:
	{
		g_nCurrFrame = g_studioRender.SetFrame( g_nCurrFrame - 1 );
		snprintf( str, sizeof( str ), "%d", g_nCurrFrame );
		leFrame->setLabel( str );
	}
	break;

	case IDC_FRAME:
	{
		g_nCurrFrame = atoi( leFrame->getLabel() );
		g_nCurrFrame = g_studioRender.SetFrame( g_nCurrFrame );
	}
	break;

	case IDC_NEXTFRAME:
	{
		g_nCurrFrame = g_studioRender.SetFrame( g_nCurrFrame + 1 );
		snprintf( str, sizeof( str ), "%d", g_nCurrFrame );
		leFrame->setLabel( str );
	}
	break;

	case IDC_BODYPART:
	{
		int index = cBodypart->getSelectedIndex();
		if ( index >= 0 )
		{
			setBodypart( index );

		}
	}
	break;

	case IDC_SUBMODEL:
	{
		int index = cSubmodel->getSelectedIndex();
		if ( index >= 0 )
		{
			setSubmodel( index );

		}
	}
	break;

	case IDC_CONTROLLER:
	{
		int index = cController->getSelectedIndex();
		if ( index >= 0 )
			setBoneController( index );
	}
	break;

	case IDC_CONTROLLERVALUE:
	{
		int index = cController->getSelectedIndex();
		if ( index >= 0 )
			setBoneControllerValue( index, ( float )slController->getValue() );
	}
	break;

	case IDC_SKINS:
	{
		int index = cSkin->getSelectedIndex();
		if ( index >= 0 )
		{
			g_studioRender.SetSkin( index );
			g_viewerSettings.skin = index;
			d_GlWindow->redraw();
		}
	}
	break;

	case IDC_TEXTURES:
	{
		int index = cTextures->getSelectedIndex();
		if ( index >= 0 )
		{
			setTexture( index );
		}
	}
	break;

	case IDC_CHROME:
	{
		studiohdr_t * hdr = g_studioRender.getTextureHeader();
		if ( hdr )
		{
			mstudiotexture_t * ptexture = ( mstudiotexture_t * )( ( byte * ) hdr + hdr->textureindex ) + g_viewerSettings.texture;
			if ( cbChrome->isChecked() )
				ptexture->flags |= STUDIO_NF_CHROME;
			else
				ptexture->flags &= ~STUDIO_NF_CHROME;
		}
	}
	break;

	case IDC_EXPORTTEXTURE:
	{
		char * ptr = ( char * )mxGetSaveFileName( this, "", "*.bmp" );
		if ( !ptr )
			break;

		char filename[256];
		char ext[16];

		strlcpy( filename, ptr, sizeof( filename ) );
		strlcpy( ext, mx_getextension( filename ), sizeof( ext ) );
		if ( mx_strcasecmp( ext, ".bmp" ) )
			strlcat( filename, ".bmp", sizeof( filename ) );

		if ( !g_studioRender.WriteBitmap( g_viewerSettings.texture, filename ) )
		{
			mxMessageBox( this, "Error writing .BMP texture.", g_appTitle, MX_MB_OK | MX_MB_ERROR );
		}
	}
	break;

	case IDC_IMPORTTEXTURE:
	{
		char * ptr = ( char * ) mxGetOpenFileName( this, "", "*.bmp" );
		if ( !ptr )
			break;

		char filename[256];
		char ext[16];

		strlcpy( filename, ptr, sizeof( filename ) );
		strlcpy( ext, mx_getextension( filename ), sizeof( ext ) );
		if ( mx_strcasecmp( ext, ".bmp" ) )
			strlcat( filename, ".bmp", sizeof( filename ) );

		mxImage * image = mxBmpRead( filename );
		if ( !image )
		{
			mxMessageBox( this, "Error loading .BMP texture.", g_appTitle, MX_MB_OK | MX_MB_ERROR );
			return 1;
		}

		if ( !image->palette )
		{
			delete image;
			mxMessageBox( this, "Error loading .BMP texture.  Must be 8-bit!", g_appTitle, MX_MB_OK | MX_MB_ERROR );
			return 1;
		}

		studiohdr_t * phdr = g_studioRender.getTextureHeader();
		if ( phdr )
		{
			mstudiotexture_t * ptexture = ( mstudiotexture_t * )( ( byte * ) phdr + phdr->textureindex ) + g_viewerSettings.texture;
			if ( image->width == ptexture->width && image->height == ptexture->height )
			{
				memcpy( ( byte * ) phdr + ptexture->index, image->data, image->width * image->height );
				memcpy( ( byte * ) phdr + ptexture->index + image->width * image->height, image->palette, 768 );

				g_studioRender.UploadTexture( ptexture, ( byte * ) phdr + ptexture->index, ( byte * ) phdr + ptexture->index + image->width * image->height, g_viewerSettings.texture + 3 );
			}
			else
				mxMessageBox( this, "Texture must be of same size.", g_appTitle, MX_MB_OK | MX_MB_ERROR );
		}

		delete image;
		d_GlWindow->redraw();
	}
	break;

	case IDC_SAVEMODEL:
	{
		char * ptr = ( char * ) mxGetSaveFileName( this, "", "*.mdl" );
		if ( !ptr )
			break;

		char filename[256];
		char ext[16];

		strlcpy( filename, ptr, sizeof( filename ) );
		strlcpy( ext, mx_getextension( filename ), sizeof( ext ) );
		if ( mx_strcasecmp( ext, ".mdl" ) )
			strlcat( filename, ".mdl", sizeof( filename ) );

		if ( !g_studioRender.SaveModel( filename ) )
			mxMessageBox( this, "Error saving model.", g_appTitle, MX_MB_OK | MX_MB_ERROR );
		else
			strlcpy( g_viewerSettings.modelFile, filename, sizeof( g_viewerSettings.modelFile ) );
	}
	break;

	case IDC_TEXTURESCALE:
	{
		g_viewerSettings.textureScale =  1.0f + ( float )( ( mxSlider * ) event->widget )->getValue() * 4.0f / 100.0f;
		d_GlWindow->redraw();
	}
	break;

#ifdef WIN32
	case IDC_FULLSCREEN:
		fullscreen();
		break;
#endif
	}

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
// TOMAS: UNDONE: Update this?
void ControlPanel::dumpModelInfo( void )
{

}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ControlPanel::loadModel( const char * filename )
{
	g_MDLViewer->setLabelFile( NULL );
	g_viewerSettings.modelFile[ 0 ] = 0;

	if ( !filename || !filename[ 0 ] )
	{
		return;
	}

	g_studioRender.FreeModel();
	g_studioRender.UnloadTextures();
	if ( g_studioRender.LoadModel( ( char * )filename ) )
	{
		g_studioRender.UploadTextures();
		if ( g_studioRender.PostLoadModel( ( char * )filename ) )
		{
			g_MDLViewer->setLabelFile( filename );
			strlcpy( g_viewerSettings.modelFile, filename,
				sizeof( g_viewerSettings.modelFile ) );

			initSequences();
			initBodyparts();
			initBoneControllers();
			initSkins();
			initTextures();
			initSubModels();

			centerView();
			setModelInfo();

			g_viewerSettings.sequence = 0;
			g_viewerSettings.speedScale = 1.0f;
			slSpeedScale->setValue( 40 );

			mx_setcwd( mx_getpath( filename ) );
		}
		else
		{
			mxMessageBox( this, "Error post-loading model.", g_appTitle, MX_MB_ERROR | MX_MB_OK );
		}
	}
	else
		mxMessageBox( this, "Error loading model.", g_appTitle, MX_MB_ERROR | MX_MB_OK );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ControlPanel::setRenderMode( int mode )
{
	g_viewerSettings.renderMode = mode;
	d_GlWindow->redraw();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ControlPanel::setShowGround( bool b )
{
	g_viewerSettings.showGround = b;
	cbGround->setChecked( b );
	if ( !b )
	{
		cbMirror->setChecked( b );
		g_viewerSettings.mirror = b;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ControlPanel::setMirror( bool b )
{
	g_viewerSettings.useStencil = ( !g_viewerSettings.use3dfx && b );
	g_viewerSettings.mirror = b;
	cbMirror->setChecked( b );
	if ( b )
	{
		cbGround->setChecked( b );
		g_viewerSettings.showGround = b;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ControlPanel::setShowBackground( bool b )
{
	g_viewerSettings.showBackground = b;
	cbBackground->setChecked( b );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ControlPanel::initSubModels( void )
{
	for ( int i = 0; i < MAXSTUDIOMODELS; i++ )
	{
		g_viewerSettings.submodels[i] = 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ControlPanel::initSequences( void )
{
	studiohdr_t * hdr = g_studioRender.getStudioHeader();
	if ( hdr )
	{
		cSequence->removeAll();
		for ( int i = 0; i < hdr->numseq; i++ )
		{
			mstudioseqdesc_t * pseqdescs = ( mstudioseqdesc_t * )( ( byte * ) hdr + hdr->seqindex );
			cSequence->add( pseqdescs[i].label );
		}

		cSequence->select( 0 );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ControlPanel::setSequence( int index )
{
	cSequence->select( index );
	g_studioRender.SetSequence( index );
	g_viewerSettings.sequence = index;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ControlPanel::initBodyparts( void )
{
	studiohdr_t * hdr = g_studioRender.getStudioHeader();
	if ( hdr )
	{
		int i;
		mstudiobodyparts_t * pbodyparts = ( mstudiobodyparts_t * )( ( byte * ) hdr + hdr->bodypartindex );

		cBodypart->removeAll();
		if ( hdr->numbodyparts > 0 )
		{
			for ( i = 0; i < hdr->numbodyparts; i++ )
				cBodypart->add( pbodyparts[i].name );

			cBodypart->select( 0 );

			cSubmodel->removeAll();
			for ( i = 0; i < pbodyparts[0].nummodels; i++ )
			{
				char str[64];
				snprintf( str, sizeof( str ), "Submodel %d", i + 1 );
				cSubmodel->add( str );
			}
			cSubmodel->select( 0 );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ControlPanel::setBodypart( int index )
{
	studiohdr_t * hdr = g_studioRender.getStudioHeader();
	if ( hdr )
	{
		//cBodypart->setEn
		cBodypart->select( index );
		if ( index < hdr->numbodyparts )
		{
			mstudiobodyparts_t * pbodyparts = ( mstudiobodyparts_t * )( ( byte * ) hdr + hdr->bodypartindex );
			cSubmodel->removeAll();

			for ( int i = 0; i < pbodyparts[index].nummodels; i++ )
			{
				char str[64];
				snprintf( str, sizeof( str ), "Submodel %d", i + 1 );
				cSubmodel->add( str );
			}
			cSubmodel->select( 0 );
			//g_studioRender.SetBodygroup (index, 0);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ControlPanel::setSubmodel( int index )
{
	g_studioRender.SetBodygroup( cBodypart->getSelectedIndex(), index );
	g_viewerSettings.submodels[cBodypart->getSelectedIndex()] = index;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ControlPanel::initBoneControllers( void )
{
	studiohdr_t * hdr = g_studioRender.getStudioHeader();
	if ( !hdr )
	{
		return;
	}

	cController->setEnabled( hdr->numbonecontrollers > 0 );
	slController->setEnabled( hdr->numbonecontrollers > 0 );
	cController->removeAll();

	if ( hdr->numbonecontrollers <= 0 )
	{
		return;
	}

	mstudiobonecontroller_t * pbonecontroller =
		( mstudiobonecontroller_t * )( ( byte * )hdr +
		hdr->bonecontrollerindex );

	for ( int i = 0; i < hdr->numbonecontrollers; i++ )
	{
		char str[ 32 ];
		snprintf( str, sizeof( str ), "Controller %d", i );
		cController->add( str );

		// SET START/END VALUES

		float fStart = pbonecontroller[ i ].start;
		float fEnd = pbonecontroller[ i ].end;
		if ( fStart > fEnd )
		{
			float fTmp = fStart;
			fStart = fEnd;
			fEnd = fTmp;
		}

		g_viewerSettings.ctrlStart[ i ] = fStart;
		g_viewerSettings.ctrlEnd[ i ] = fEnd;

		// SET INITIAL VALUE

		// TOMAS: UGLY HACK to get "correct" default value
		float fValue = ( ( fStart + fEnd ) == 0 ) ? 0 : fStart;
		//float fValue = BOUND( fStart, 0, fEnd );
		setBoneControllerValue( i, fValue );

	}

	// Select controller 0
	cController->select( 0 );
	setBoneController( 0 );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ControlPanel::setBoneController( int index )
{
	int nStart = ( int )g_viewerSettings.ctrlStart[ index ];
	int nEnd = ( int )g_viewerSettings.ctrlEnd[ index ];
	int nValue = ( int )g_viewerSettings.ctrlValue[ index ];

	slController->setRange( nStart, nEnd );
	slController->setValue( nValue );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ControlPanel::setBoneControllerValue( int index, float value )
{
	studiohdr_t * hdr = g_studioRender.getStudioHeader();
	if ( !hdr )
	{
		return;
	}

	g_studioRender.SetController( index, value );

	g_viewerSettings.ctrlValue[ index ] = value;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ControlPanel::initSkins( void )
{
	studiohdr_t * hdr = g_studioRender.getStudioHeader();
	if ( hdr )
	{
		cSkin->setEnabled( hdr->numskinfamilies > 0 );
		cSkin->removeAll();

		for ( int i = 0; i < hdr->numskinfamilies; i++ )
		{
			char str[32];
			snprintf( str, sizeof( str ), "Skin %d", i + 1 );
			cSkin->add( str );
		}

		cSkin->select( 0 );
		g_studioRender.SetSkin( 0 );
		g_viewerSettings.skin = 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ControlPanel::setModelInfo( void )
{
	static char str[2048];
	studiohdr_t * hdr = g_studioRender.getStudioHeader();

	if ( !hdr )
		return;

	snprintf( str, sizeof( str ),
	          "Bones: %d\n"
	          "Bone Controllers: %d\n"
	          "Sequences: %d\n",
	          hdr->numbones,
	          hdr->numbonecontrollers,
	          hdr->numseq
	        );

	lModelInfo1->setLabel( str );

	snprintf( str, sizeof( str ),
	          "Textures: %d\n"
	          "Skin Families: %d\n"
	          "Bodyparts: %d\n",
	          hdr->numtextures,
	          hdr->numskinfamilies,
	          hdr->numbodyparts
	        );

	lModelInfo2->setLabel( str );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ControlPanel::initTextures( void )
{
	studiohdr_t * hdr = g_studioRender.getTextureHeader();
	if ( hdr )
	{
		cTextures->removeAll();
		mstudiotexture_t * ptextures = ( mstudiotexture_t * )( ( byte * ) hdr + hdr->textureindex );
		for ( int i = 0; i < hdr->numtextures; i++ )
			cTextures->add( ptextures[i].name );
		cTextures->select( 0 );
		g_viewerSettings.texture = 0;
		/*if (hdr->numtextures > 0)
			cbChrome->setChecked ((ptextures[0].flags & STUDIO_NF_CHROME) == STUDIO_NF_CHROME);*/
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ControlPanel::setTexture( int index )
{
	if ( index >= 0 )
	{
		g_viewerSettings.texture = index;
		studiohdr_t * hdr = g_studioRender.getTextureHeader();
		if ( hdr )
		{
			mstudiotexture_t * ptexture = ( mstudiotexture_t * )( ( byte * ) hdr + hdr->textureindex ) + index;
			char str[32];
			snprintf( str, sizeof( str ), "Size: %d x %d", ptexture->width, ptexture->height );
			lTexSize->setLabel( str );
			cbChrome->setChecked( ( ptexture->flags & STUDIO_NF_CHROME ) == STUDIO_NF_CHROME );
		}
		d_GlWindow->redraw();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ControlPanel::centerView( void )
{
	// TOMAS: HACK: Some awful constant will do the trick for now...
	float min[3] = { -50, -50, -50};
	float max[3] = {50, 50, 50};

	// TOMAS: UNDONE: Find another way to do this, we dont have bbox in alpha models...
	//g_studioRender.ExtractBbox (min, max);
	g_studioRender.CalcBbox( 0, min, max );

	float dx = max[0] - min[0];
	float dy = max[1] - min[1];
	float dz = max[2] - min[2];
	float d = dx;
	if ( dy > d )
		d = dy;
	if ( dz > d )
		d = dz;

	g_viewerSettings.trans[0] = 0;
	g_viewerSettings.trans[1] = min[2] + dz / 2;
	g_viewerSettings.trans[2] = d * 1.0f;
	g_viewerSettings.rot[0] = -90.0f;
	g_viewerSettings.rot[1] = -90.0f;
	g_viewerSettings.rot[2] = 0.0f;
	d_GlWindow->redraw();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
#ifdef WIN32
void ControlPanel::fullscreen( void )
{
	studiohdr_t * hdr = g_studioRender.getStudioHeader();
	if ( !hdr )
		return;

	//g_viewerSettings.use3dfx = cb3dfxOpenGL->isChecked ();
	swap3dfxgl( g_viewerSettings.use3dfx );

	char szName[256];

	GetModuleFileName( NULL, szName, 256 );
	char * ptr = strrchr( szName, '\\' );
	*ptr = '\0';
	SetCurrentDirectory( szName );

	g_viewerSettings.width = atoi( leWidth->getLabel() );
	g_viewerSettings.height = atoi( leHeight->getLabel() );
	g_viewerSettings.cds = true;
	//g_viewerSettings.use3dfx = cb3dfxOpenGL->isChecked ();

	if ( SaveViewerSettings( "gsmv.cfg" ) )
	{
		g_viewerSettings.pause = true;
		g_viewerSettings.use3dfx = false;
		WinExec( "gsmv.exe -fullscreen", SW_SHOW );
	}
}
#endif
