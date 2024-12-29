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

#include "ViewerSettings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


ViewerSettings g_viewerSettings;

void InitViewerSettings( void )
{
	memset( &g_viewerSettings, 0, sizeof( ViewerSettings ) );
	g_viewerSettings.rot[0] = -90.0f;
	g_viewerSettings.trans[3] = 50.0f;
	g_viewerSettings.renderMode = RM_TEXTURED;
	g_viewerSettings.transparency = 1.0f;

	g_viewerSettings.bgColor[0] = 0.25f;
	g_viewerSettings.bgColor[1] = 0.5f;
	g_viewerSettings.bgColor[2] = 0.5f;

	g_viewerSettings.gColor[0] = 0.85f;
	g_viewerSettings.gColor[1] = 0.85f;
	g_viewerSettings.gColor[2] = 0.69f;

	g_viewerSettings.lColor[0] = 1.0f;
	g_viewerSettings.lColor[1] = 1.0f;
	g_viewerSettings.lColor[2] = 1.0f;

	g_viewerSettings.speedScale = 1.0f;
	g_viewerSettings.textureLimit = 256;

	g_viewerSettings.textureScale = 1.0f;
}



int
LoadViewerSettings( const char * filename )
{
	FILE * file = fopen( filename, "rb" );

	if ( !file )
		return 0;

	fread( &g_viewerSettings, sizeof( ViewerSettings ), 1, file );
	fclose( file );

	return 1;
}



int
SaveViewerSettings( const char * filename )
{
	FILE * file = fopen( filename, "wb" );

	if ( !file )
		return 0;

	fwrite( &g_viewerSettings, sizeof( ViewerSettings ), 1, file );
	fclose( file );

	return 1;
}
