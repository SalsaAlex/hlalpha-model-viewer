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

#ifndef INCLUDED_VIEWERSETTINGS
#define INCLUDED_VIEWERSETTINGS

#include "mathlib.h"
#include "studio.h"


enum // render modes
{
    RM_WIREFRAME,
    RM_FLATSHADED,
    RM_SMOOTHSHADED,
    RM_TEXTURED
};

typedef struct
{
	// model
	float rot[3];
	float trans[3];

	// render
	int renderMode;
	float transparency;
	bool showBackground;
	bool showGround;
	bool showBones;
	bool showTexture;
	int texture;
	float textureScale;
	int skin;
	bool mirror;
	bool useStencil;	// if 3dfx fullscreen set false

	// animation
	int sequence;
	float speedScale;

	// bodyparts and bonecontrollers
	int submodels[MAXSTUDIOMODELS];
	float ctrlValue[MAXSTUDIOCONTROLLERS];
	float ctrlStart[MAXSTUDIOCONTROLLERS];
	float ctrlEnd[MAXSTUDIOCONTROLLERS];

	// fullscreen
	int width, height;
	bool use3dfx;
	bool cds;

	// colors
	float bgColor[4];
	float lColor[4];
	float gColor[4];

	// misc
	int textureLimit;
	bool pause;

	char modelFile[256];
	char backgroundTexFile[256];
	char groundTexFile[256];
} ViewerSettings;



extern ViewerSettings g_viewerSettings;



#ifdef __cplusplus
extern "C" {
#endif

	void InitViewerSettings( void );
	int LoadViewerSettings( const char * filename );
	int SaveViewerSettings( const char * filename );

#ifdef __cplusplus
}
#endif



#endif // INCLUDED_VIEWERSETTINGS