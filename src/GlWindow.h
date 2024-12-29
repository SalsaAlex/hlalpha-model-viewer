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

#ifndef INCLUDED_GLWINDOW
#define INCLUDED_GLWINDOW

#ifndef INCLUDED_MXGLWINDOW
#include <mx/mxGlWindow.h>
#endif

#ifndef INCLUDED_VIEWERSETTINGS
#include "ViewerSettings.h"
#endif

enum // texture names
{
    TEXTURE_GROUND = 1,
    TEXTURE_BACKGROUND = 2
};

class StudioModel;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class GlWindow : public mxGlWindow
{
	int d_textureNames[2];

public:
	// CREATORS
	GlWindow( mxWindow * parent, int x, int y, int w, int h, const char * label, int style );
	~GlWindow();

	// MANIPULATORS
	virtual int handleEvent( mxEvent * event );
	virtual void draw();

	int loadTexture( const char * filename, int name );
	void dumpViewport( const char * filename );

	// ACCESSORS
};

extern GlWindow * g_GlWindow;

#endif // INCLUDED_GLWINDOW
