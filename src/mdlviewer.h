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

#ifndef INCLUDED_MDLVIEWER
#define INCLUDED_MDLVIEWER

#ifndef INCLUDED_MXWINDOW
#include <mx/mxWindow.h>
#endif

#define IDC_FILE_LOADMODEL			1001
#define IDC_FILE_DECOMPILEMODEL		1002
#define IDC_FILE_LOADBACKGROUNDTEX	1003
#define IDC_FILE_LOADGROUNDTEX		1004
#define IDC_FILE_UNLOADGROUNDTEX	1005
#define IDC_FILE_RECENTMODELS1		1008
#define IDC_FILE_RECENTMODELS2		1009
#define IDC_FILE_RECENTMODELS3		1010
#define IDC_FILE_RECENTMODELS4		1011
#define IDC_FILE_RECENTMODELS5		1012
#define IDC_FILE_RECENTMODELS6		1013
#define IDC_FILE_RECENTMODELS7		1014
#define IDC_FILE_RECENTMODELS8		1015
#define IDC_FILE_EXIT				1016

#define IDC_OPTIONS_COLORBACKGROUND	1101
#define IDC_OPTIONS_COLORGROUND		1102
#define IDC_OPTIONS_COLORLIGHT		1103
#define IDC_OPTIONS_CENTERVIEW		1104
#define IDC_OPTIONS_MAKESCREENSHOT	1105
#define IDC_OPTIONS_FILTERTEXTURES	1106
#define IDC_OPTIONS_DUMP			1107

#define IDC_VIEW_FILEASSOCIATIONS	1201

//#define IDC_HELP_GOTOHOMEPAGE		1301
#define IDC_HELP_ABOUT				1302
#define IDC_HELP_CONTROLS			1303


class mxMenuBar;
class GlWindow;
class ControlPanel;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class MDLViewer : public mxWindow
{
	mxMenuBar * mb;
	GlWindow * d_GlWindow;
	ControlPanel * d_cpl;

	void loadRecentFiles();
	void saveRecentFiles();
	void initRecentFiles();

public:
	// CREATORS
	MDLViewer( void );
	~MDLViewer( void );

	// MANIPULATORS
	virtual int handleEvent( mxEvent * event );
	void redraw( void );

	void setLabelFile( const char * filename );

	// ACCESSORS
	mxMenuBar * getMenuBar( void ) const { return mb;	}
	GlWindow * getGlWindow( void ) const { return d_GlWindow; }
};

extern MDLViewer * g_MDLViewer;
extern char g_appTitle[];

#endif // INCLUDED_MDLVIEWER
