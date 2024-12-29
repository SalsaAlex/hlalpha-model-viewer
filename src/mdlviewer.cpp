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

#include "mdlviewer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mx/mx.h>
#include <mx/gl.h>
#include <mx/mxTga.h>

#include "safestr.h"
#include "pathutils.h"
#include "GlWindow.h"
#include "ControlPanel.h"
#include "StudioRender.h"
#include "FileAssociation.h"


extern bool bFilterTextures;

MDLViewer * g_MDLViewer = 0;
char g_appTitle[] = "GoldSrc Model Viewer (v0.3a BETA2) MDL v6";

#define RECENT_FILE_CNT 8
static char recentFiles[RECENT_FILE_CNT][256] = { "", "", "", "", "", "", "", "" };

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void MDLViewer::initRecentFiles( void )
{
	for ( int i = 0; i < RECENT_FILE_CNT; i++ )
	{
		if ( strlen( recentFiles[i] ) )
		{
			mb->modify( IDC_FILE_RECENTMODELS1 + i, IDC_FILE_RECENTMODELS1 + i, recentFiles[i] );
		}
		else
		{
			mb->modify( IDC_FILE_RECENTMODELS1 + i, IDC_FILE_RECENTMODELS1 + i, "(empty)" );
			mb->setEnabled( IDC_FILE_RECENTMODELS1 + i, false );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void MDLViewer::loadRecentFiles( void )
{
	char path[256];
	strlcpy( path, mx::getApplicationPath(), sizeof( path ) );
	strlcat( path, "/gsmv.rf", sizeof( path ) );
	FILE * file = fopen( path, "rb" );
	if ( file )
	{
		fread( recentFiles, sizeof recentFiles, 1, file );
		fclose( file );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void MDLViewer::saveRecentFiles( void )
{
	char path[256];

	strlcpy( path, mx::getApplicationPath(), sizeof( path ) );
	strlcat( path, "/gsmv.rf", sizeof( path ) );

	FILE * file = fopen( path, "wb" );
	if ( file )
	{
		fwrite( recentFiles, sizeof recentFiles, 1, file );
		fclose( file );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool swap3dfxgl( bool b )
{
#ifdef WIN32
	//
	// rename opengl drivers to 3dfxgl
	//
	char szName[256], szPath[256];

	strlcpy( szName, mx::getApplicationPath(), sizeof( szName ) );
	GetCurrentDirectory( 256, szPath );
	SetCurrentDirectory( szName );

	BOOL ret = TRUE;
	if ( b )
	{
		ret &= CopyFile( "3dfxgl.dll", "opengl32.dll", FALSE );
		ret &= DeleteFile( "3dfxgl.dll" );
	}
	else
	{
		ret &= CopyFile( "opengl32.dll", "3dfxgl.dll", FALSE );
		ret &= DeleteFile( "opengl32.dll" );
	}

	SetCurrentDirectory( szPath );

	return ( ret == TRUE );
#else
	return TRUE;
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
MDLViewer::MDLViewer( void )
	: mxWindow( 0, 0, 0, 0, 0, g_appTitle, mxWindow::Normal )
{
	// create menu stuff
	mb = new mxMenuBar( this );
	mxMenu * menuFile = new mxMenu();
	mxMenu * menuOptions = new mxMenu();
	mxMenu * menuTools = new mxMenu();
	mxMenu * menuHelp = new mxMenu();

	mb->addMenu( "File", menuFile );
	mb->addMenu( "Options", menuOptions );
	mb->addMenu( "Tools", menuTools );
	mb->addMenu( "Help", menuHelp );

	mxMenu * menuRecentModels = new mxMenu();
	menuRecentModels->add( "(empty)", IDC_FILE_RECENTMODELS1 );
	menuRecentModels->add( "(empty)", IDC_FILE_RECENTMODELS2 );
	menuRecentModels->add( "(empty)", IDC_FILE_RECENTMODELS3 );
	menuRecentModels->add( "(empty)", IDC_FILE_RECENTMODELS4 );
	menuRecentModels->add( "(empty)", IDC_FILE_RECENTMODELS5 );
	menuRecentModels->add( "(empty)", IDC_FILE_RECENTMODELS6 );
	menuRecentModels->add( "(empty)", IDC_FILE_RECENTMODELS7 );
	menuRecentModels->add( "(empty)", IDC_FILE_RECENTMODELS8 );

	menuFile->add( "Load Model...", IDC_FILE_LOADMODEL );
	menuFile->addSeparator();
#ifdef WIN32
	menuFile->add( "Decompile Model", IDC_FILE_DECOMPILEMODEL );
	menuFile->addSeparator();
#endif
	menuFile->add( "Load Background Texture...", IDC_FILE_LOADBACKGROUNDTEX );
	menuFile->add( "Load Ground Texture...", IDC_FILE_LOADGROUNDTEX );
	menuFile->addSeparator();
	menuFile->add( "Unload Ground Texture", IDC_FILE_UNLOADGROUNDTEX );
	menuFile->addSeparator();
	menuFile->addMenu( "Recent Models", menuRecentModels );
	menuFile->addSeparator();
	menuFile->add( "Exit", IDC_FILE_EXIT );

	menuOptions->add( "Background Color...", IDC_OPTIONS_COLORBACKGROUND );
	menuOptions->add( "Ground Color...", IDC_OPTIONS_COLORGROUND );
	menuOptions->add( "Light Color...", IDC_OPTIONS_COLORLIGHT );
	menuOptions->addSeparator();
	menuOptions->add( "Filter Textures", IDC_OPTIONS_FILTERTEXTURES );
	menuOptions->addSeparator();
	menuOptions->add( "Center View", IDC_OPTIONS_CENTERVIEW );
#ifdef WIN32
	menuOptions->addSeparator();
	menuOptions->add( "Make Screenshot...", IDC_OPTIONS_MAKESCREENSHOT );
	//menuOptions->add ("Dump Model Info", IDC_OPTIONS_DUMP);
#endif

	menuTools->add( "File Associations...", IDC_VIEW_FILEASSOCIATIONS );

	/*#ifdef WIN32
		menuHelp->add ("Goto Homepage...", IDC_HELP_GOTOHOMEPAGE);
		menuHelp->addSeparator ();
	#endif*/
	menuHelp->add( "Controls...", IDC_HELP_CONTROLS );
	menuHelp->add( "About...", IDC_HELP_ABOUT );

	mb->setChecked( IDC_OPTIONS_FILTERTEXTURES, bFilterTextures );

	// create the OpenGL window
	d_GlWindow = new GlWindow( this, 0, 0, 0, 0, "", mxWindow::Normal );
#ifdef WIN32
	SetWindowLong( ( HWND ) d_GlWindow->getHandle(), GWL_EXSTYLE, WS_EX_CLIENTEDGE );
#endif

	d_cpl = new ControlPanel( this );
	d_cpl->setGlWindow( d_GlWindow );
	g_GlWindow = d_GlWindow;

	g_FileAssociation = new FileAssociation();

	loadRecentFiles();
	initRecentFiles();

	setBounds( 20, 20, 640, 540 );
	setVisible( true );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
MDLViewer::~MDLViewer( void )
{
	saveRecentFiles();
	//SaveViewerSettings ("gsmv.cfg");

	swap3dfxgl( false );

#ifdef WIN32
	//DeleteFile ("gsmv.cfg");
	DeleteFile( "midump.txt" );
#endif
}
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void MDLViewer::setLabelFile( const char * filename )
{
	char szTitle[ 1024 ];

	if ( !filename || !filename[ 0 ] )
	{
		snprintf( szTitle, sizeof( szTitle ), "%s", g_appTitle );
	}
	else
	{
		char * szShortName = SkipPath( ( char * )filename );
		snprintf( szTitle, sizeof( szTitle ), "%s - %s", szShortName,
			g_appTitle );
	}
	
	setLabel( szTitle );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int MDLViewer::handleEvent( mxEvent * event )
{
	switch ( event->event )
	{
	case mxEvent::Action:
	{
		switch ( event->action )
		{
		case IDC_FILE_LOADMODEL:
		{
			const char * ptr = mxGetOpenFileName( this, 0, "*.mdl" );
			if ( ptr )
			{
				int i;
				d_cpl->loadModel( ptr );

				for ( i = 0; i < RECENT_FILE_CNT; i++ )
				{
					if ( !mx_strcasecmp( recentFiles[i], ptr ) )
						break;
				}

				// swap existing recent file
				if ( i < RECENT_FILE_CNT )
				{
					char tmp[256];
					strlcpy( tmp, recentFiles[0], sizeof( tmp ) );
					strlcpy( recentFiles[0], recentFiles[i], sizeof( recentFiles[0] ) );
					strlcpy( recentFiles[i], tmp, sizeof( recentFiles[i] ) );
				}

				// insert recent file
				else
				{
					for ( i = RECENT_FILE_CNT - 1; i > 0; i-- )
						strlcpy( recentFiles[i], recentFiles[i - 1], sizeof( recentFiles[i] ) );

					strlcpy( recentFiles[0], ptr, sizeof( recentFiles[0] ) );
				}

				initRecentFiles();
			}
		}
		break;
#ifdef WIN32
		case IDC_FILE_DECOMPILEMODEL:
		{
			char * szFileName = g_viewerSettings.modelFile;

			// Get file path
			char szPath[ 256 ];
			GetPath( szFileName, szPath );

			// Build subfolder name
			char szDirName[ 256 ];
			char * szShortFileName = SkipPath( szFileName );
			StripExtension( szShortFileName, szDirName );

			if ( szFileName[ 0 ] != 0 )
			{

				char szCmd[ 1024 ];
				snprintf( szCmd, sizeof( szCmd ), "%s %s -p %s/%s",
					"mdl6dec.exe", szFileName, szPath, szDirName );
				
				int nResult;
				nResult = WinExec( szCmd, SW_SHOW );

				if ( nResult <= 31 )
				{
					mxMessageBox( this, "Error executing model decompiler.",
						g_appTitle, MX_MB_OK | MX_MB_ERROR );
				}
			}
		}
		break;
#endif
		case IDC_FILE_LOADBACKGROUNDTEX:
		case IDC_FILE_LOADGROUNDTEX:
		{
			const char * ptr = mxGetOpenFileName( this, 0, "*.*" );
			if ( ptr )
			{
				if ( d_GlWindow->loadTexture( ptr, event->action - IDC_FILE_LOADBACKGROUNDTEX ) )
				{
					if ( event->action == IDC_FILE_LOADBACKGROUNDTEX )
						d_cpl->setShowBackground( true );
					else
						d_cpl->setShowGround( true );

				}
				else
					mxMessageBox( this, "Error loading texture.", g_appTitle, MX_MB_OK | MX_MB_ERROR );
			}
		}
		break;

		case IDC_FILE_UNLOADGROUNDTEX:
		{
			d_GlWindow->loadTexture( 0, 1 );
			d_cpl->setShowGround( false );
		}
		break;

		case IDC_FILE_RECENTMODELS1:
		case IDC_FILE_RECENTMODELS2:
		case IDC_FILE_RECENTMODELS3:
		case IDC_FILE_RECENTMODELS4:
		case IDC_FILE_RECENTMODELS5:
		case IDC_FILE_RECENTMODELS6:
		case IDC_FILE_RECENTMODELS7:
		case IDC_FILE_RECENTMODELS8:
		{
			int i = event->action - IDC_FILE_RECENTMODELS1;
			d_cpl->loadModel( recentFiles[i] );

			char tmp[256];
			strlcpy( tmp, recentFiles[0], sizeof( tmp ) );
			strlcpy( recentFiles[0], recentFiles[i], sizeof( recentFiles[0] ) );
			strlcpy( recentFiles[i], tmp, sizeof( recentFiles[i] ) );

			initRecentFiles();

			redraw();
		}
		break;

		case IDC_FILE_EXIT:
		{
			redraw();
			mx::quit();
		}
		break;

		case IDC_OPTIONS_COLORBACKGROUND:
		case IDC_OPTIONS_COLORGROUND:
		case IDC_OPTIONS_COLORLIGHT:
		{
			float * cols[3] = { g_viewerSettings.bgColor, g_viewerSettings.gColor, g_viewerSettings.lColor };
			float * col = cols[event->action - IDC_OPTIONS_COLORBACKGROUND];
			int r = ( int )( col[0] * 255.0f );
			int g = ( int )( col[1] * 255.0f );
			int b = ( int )( col[2] * 255.0f );
			if ( mxChooseColor( this, &r, &g, &b ) )
			{
				col[0] = ( float ) r / 255.0f;
				col[1] = ( float ) g / 255.0f;
				col[2] = ( float ) b / 255.0f;
			}
		}
		break;

		case IDC_OPTIONS_CENTERVIEW:
			d_cpl->centerView();
			break;

		case IDC_OPTIONS_MAKESCREENSHOT:
		{
			char * ptr = ( char * ) mxGetSaveFileName( this, "", "*.tga" );
			if ( ptr )
			{
				if ( !strstr( ptr, ".tga" ) )
					strlcat( ptr, ".tga", sizeof( ptr ) );
				d_GlWindow->dumpViewport( ptr );
			}
		}
		break;

		case IDC_OPTIONS_FILTERTEXTURES:
			bFilterTextures = !mb->isChecked( IDC_OPTIONS_FILTERTEXTURES );
			mb->setChecked( IDC_OPTIONS_FILTERTEXTURES, bFilterTextures );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bFilterTextures ? GL_LINEAR : GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, bFilterTextures ? GL_LINEAR : GL_NEAREST );
			break;

		case IDC_OPTIONS_DUMP:
			d_cpl->dumpModelInfo();
			break;

		case IDC_VIEW_FILEASSOCIATIONS:
			g_FileAssociation->setAssociation( 0 );
			g_FileAssociation->setVisible( true );
			break;

		case IDC_HELP_ABOUT:
			char szText[ 4096 ];

			snprintf( szText, sizeof( szText ),
				"%s\n"
				"Copyright (c) 1999-2013 Tomas Slavotinek (aka baso88, GeckoN[CZ])\n"
				"Email:\tslavotinek@gmail.com\n\n"

				"Based on Half-Life Model Viewer v1.25\n"
				"Copyright (c) 2002 by Mete Ciragan\n"
				"Email:\tmete@swissquake.ch\n"
				"Web:\thttp://www.swissquake.ch/chumbalum-soft/\n\n"

				"Build:\t" __DATE__ " "__TIME__"\n",

				g_appTitle );

			mxMessageBox( this,
				szText,

				"About GoldSrc Model Viewer\n",

				MX_MB_OK | MX_MB_INFORMATION );
			break;
		case IDC_HELP_CONTROLS:
			mxMessageBox( this,
				"Viewer controls:\n\n"

				"Left-drag\t\trotate model\n"
				"Right-drag\tzoom in/out\n"
				"Shift-left-drag\tx/y-pan\n",

				"About GoldSrc Model Viewer\n",

				MX_MB_OK | MX_MB_INFORMATION );
			break;

		} //switch (event->action)

	} // mxEvent::Action
	break;

	case mxEvent::Size:
	{
		int w = event->width;
		int h = event->height;
		int y = mb->getHeight();
#ifdef WIN32
#define HEIGHT 120
#else
#define HEIGHT 140
		h -= 40;
#endif
		d_GlWindow->setBounds( 0, y, w, h - HEIGHT );
		d_cpl->setBounds( 0, y + h - HEIGHT, w, HEIGHT );
	}
	break;
	} // event->event

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void MDLViewer::redraw( void )
{
	mxEvent event;
	event.event = mxEvent::Size;
	event.width = w2();
	event.height = h2();
	handleEvent( &event );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int main( int argc, char * argv[] )
{
	//
	// make sure, we start in the right directory
	//
	char szName[256];

	strlcpy( szName, mx::getApplicationPath(), sizeof( szName ) );
	mx_setcwd( szName );

	char cmdline[1024] = "";
	if ( argc > 1 )
	{
		strlcpy( cmdline, argv[1], sizeof( cmdline ) );
		for ( int i = 2; i < argc; i++ )
		{
			strlcat( cmdline, " ", sizeof( cmdline ) );
			strlcat( cmdline, argv[i], sizeof( cmdline ) );
		}
	}

	if ( !strcmp( cmdline, "-fullscreen" ) )
	{
#ifdef WIN32
		mxGlWindow::setFormat( mxGlWindow::FormatDouble, 32, 24 );
		mx::init( argc, argv );

		if ( !LoadViewerSettings( "gsmv.cfg" ) )
		{
			mxMessageBox( 0, "Error loading configuration.", g_appTitle, MX_MB_OK | MX_MB_ERROR );
			return 0;
		}

		if ( g_viewerSettings.cds )
			mx::setDisplayMode( g_viewerSettings.width, g_viewerSettings.height, 16 );

		mxGlWindow::setFormat( mxGlWindow::FormatDouble, 16, 16 );
		GlWindow * glw = new GlWindow( 0, 0, 0, g_viewerSettings.width, g_viewerSettings.height, "", mxWindow::Popup );

		glw->loadTexture( g_viewerSettings.backgroundTexFile, 0 );
		glw->loadTexture( g_viewerSettings.groundTexFile, 1 );

		g_studioRender.FreeModel();
		g_studioRender.UnloadTextures();
		if ( !g_studioRender.LoadModel( g_viewerSettings.modelFile ) )
		{
			if ( g_viewerSettings.cds )
				mx::setDisplayMode( 0, 0, 0 );
			mxMessageBox( 0, "Error loading model.", g_appTitle, MX_MB_OK | MX_MB_ERROR );
			exit( -1 );
		}
		g_studioRender.UploadTextures();
		if ( !g_studioRender.PostLoadModel( g_viewerSettings.modelFile ) )
		{
			if ( g_viewerSettings.cds )
				mx::setDisplayMode( 0, 0, 0 );
			mxMessageBox( 0, "Error post-loading model.", g_appTitle, MX_MB_OK | MX_MB_ERROR );
			exit( -1 );
		}

		g_studioRender.SetSequence( g_viewerSettings.sequence );

		int i;
		studiohdr_t * hdr = g_studioRender.getStudioHeader();
		for ( i = 0; i < hdr->numbodyparts; i++ )
			g_studioRender.SetBodygroup( i, g_viewerSettings.submodels[i] );

		for ( i = 0; i < hdr->numbonecontrollers; i++ )
		{
			mstudiobonecontroller_t * pbonecontrollers = ( mstudiobonecontroller_t * )( ( byte * ) hdr + hdr->bonecontrollerindex );
			/*if (pbonecontrollers[i].index == 4)
				g_studioRender.SetMouth (g_viewerSettings.controllers[i]);
			else
				g_studioRender.SetController (pbonecontrollers[i].index, g_viewerSettings.controllers[i]);*/
			g_studioRender.SetController( i, g_viewerSettings.ctrlValue[i] );
		}

		g_studioRender.SetSkin( g_viewerSettings.skin );

		if ( g_viewerSettings.speedScale == 0.0f )
			g_viewerSettings.speedScale = 1.0f;

		if ( g_viewerSettings.use3dfx )
			g_viewerSettings.useStencil = false;

		mx::setIdleWindow( glw );
		int ret = mx::run();

		g_studioRender.FreeModel();
		g_studioRender.UnloadTextures();

		if ( g_viewerSettings.cds )
			mx::setDisplayMode( 0, 0, 0 );

		return ret;
#endif
	}

	//mx::setDisplayMode (0, 0, 0);
	mx::init( argc, argv );
	g_MDLViewer = new MDLViewer();
	g_MDLViewer->setMenuBar( g_MDLViewer->getMenuBar() );
	g_MDLViewer->setBounds( 20, 20, 640, 540 );
	g_MDLViewer->setVisible( true );

	//LoadViewerSettings ("gsmv.cfg");

	if ( strstr( cmdline, ".mdl" ) )
	{
		g_ControlPanel->loadModel( cmdline );
	}

	int ret = mx::run();

	mx::cleanup();

	return ret;
}
