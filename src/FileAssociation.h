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

#ifndef INCLUDED_FILEASSOCIATION
#define INCLUDED_FILEASSOCIATION

#ifndef INCLUDED_MXWINDOW
#include <mx/mxWindow.h>
#endif

#define IDC_EXTENSION			1001
#define IDC_ADD					1002
#define IDC_REMOVE				1003
#define IDC_ACTION1				1004
#define IDC_ACTION2				1005
#define IDC_ACTION3				1006
#define IDC_ACTION4				1007
#define IDC_PROGRAM				1008
#define IDC_CHOOSEPROGRAM		1009
#define IDC_OK					1010
#define IDC_CANCEL				1011


typedef struct
{
	char extension[16];
	char program[256];
	int association;
} association_t;


class mxChoice;
class mxRadioButton;
class mxLineEdit;
class mxButton;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class FileAssociation : public mxWindow
{
	mxChoice * cExtension;
	mxRadioButton * rbAction[4];
	mxLineEdit * leProgram;
	mxButton * bChooseProgram;
	association_t d_associations[16];

	void initAssociations();
	void saveAssociations();

public:
	// CREATORS
	FileAssociation();
	virtual ~FileAssociation();

	// MANIPULATORS
	int handleEvent( mxEvent * event );
	void setAssociation( int index );

	// ACCESSORS
	int getMode( char * extension );
	char * getProgram( char * extension );
};

extern FileAssociation * g_FileAssociation;

#endif // INCLUDED_FILEASSOCIATION
