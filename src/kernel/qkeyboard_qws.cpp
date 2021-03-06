/****************************************************************************
** $Id: qt/src/kernel/qkeyboard_qws.cpp   2.3.10   edited 2005-01-24 $
**
** Implementation of Qt/Embedded keyboard drivers
**
** Created : 991025
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qwindowsystem_qws.h"
#include "qwsutils_qws.h"
#include "qgfx_qws.h"

#include <qapplication.h>
#include <qsocketnotifier.h>
#include <qnamespace.h>
#include <qtimer.h>

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <termios.h>
#ifdef _OS_LINUX_
#include <linux/kd.h>
#include <linux/keyboard.h>
#endif

#ifdef QT_QWS_TIP2
#include <qcopchannel_qws.h>
#endif

//#define QT_QWS_USE_KEYCODES

#ifndef QT_NO_QWS_KEYBOARD

#ifdef QT_QWS_YOPY
#include <qwidgetlist.h>
#include <linux/kd.h>
#include <linux/fb.h>
#include <linux/yopy_button.h>
extern "C" {
    int getpgid(int);
}
#endif

#if !defined(_OS_QNX6_)

#include <termios.h>
#if !defined(_OS_FREEBSD_) && !defined(_OS_SOLARIS_)
#include <sys/kd.h>
#include <sys/vt.h>
#endif

static int dir_keyrot = -1;

static int xform_dirkey(int key)
{
    if (dir_keyrot < 0) {
	// get the rotation
	char *kerot = getenv("QWS_CURSOR_ROTATION");
	if (kerot) {
	    if (strcmp(kerot, "90") == 0)
		dir_keyrot = 1;
	    else if (strcmp(kerot, "180") == 0)
		dir_keyrot = 2;
	    else if (strcmp(kerot, "270") == 0)
		dir_keyrot = 3;
	    else
		dir_keyrot = 0;
	} else {
	    dir_keyrot = 0;
	}
    }
    int xf = qt_screen->transformOrientation() + dir_keyrot;
    return (key-Qt::Key_Left+xf)%4+Qt::Key_Left;
}

#define VTSWITCHSIG SIGUSR2

static bool vtActive = true;
static int  vtQws = 0;
static int  kbdFD = -1;

class QWSyopyButtonsHandler : public QWSKeyboardHandler
{
    Q_OBJECT
public:
    QWSyopyButtonsHandler();
    virtual ~QWSyopyButtonsHandler();

    bool isOpen() { return buttonFD > 0; }

private slots:
    void readKeyboardData();

private:
    QString terminalName;
    int buttonFD;
    struct termios newT, oldT;
    QSocketNotifier *notifier;
};

#endif // QNX6


class QWSKeyboardRepeater : public QObject {
    Q_OBJECT
public:
    static QWSKeyboardRepeater *current;

    QWSKeyboardRepeater(QWSKeyboardHandler* parent) :
	QObject(parent)
    {
	current = this;
	repeatdelay = 400;
	repeatperiod = 80;
	repeater = new QTimer(this);
	connect(repeater, SIGNAL(timeout()), this, SLOT(autoRepeat()));
    }

    ~QWSKeyboardRepeater()
    {
	if ( current == this )
	    current = 0;
    }

    void setAutoRepeat(int d, int p) { if ( d > 0 ) repeatdelay=d; 
				       if ( p > 0 ) repeatperiod=p;}
    void getAutoRepeat(int *d ,int *p ) { if (d) *d=repeatdelay; 
    					  if (p) *p=repeatperiod; }

    void stop()
    {
	repeater->stop();
    }

    void start(int uni, int key, int mod)
    {
	runi = uni;
	rkey = key;
	rmod = mod;
	repeater->start(repeatdelay,TRUE);
    }

private slots:
    void autoRepeat();

private:
    int runi;
    int rkey;
    int rmod;

    QTimer* repeater;
    int repeatdelay, repeatperiod;
};

QWSKeyboardRepeater *QWSKeyboardRepeater::current=0;

void QWSKeyboardRepeater::autoRepeat()
{
#ifdef QT_QWS_AUTOREPEAT_MANUALLY
    qwsServer->processKeyEvent( runi, rkey, rmod, FALSE, TRUE );
    qwsServer->processKeyEvent( runi, rkey, rmod, TRUE, TRUE );
    repeater->start(repeatperiod);
#endif
}


#ifdef QT_QWS_SL5XXX
static const QWSServer::KeyMap keyM[] = {
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 00
    {	Qt::Key_A,		'a'     , 'A'     , 'A'-64  }, // 01
    {	Qt::Key_B,		'b'     , 'B'     , 'B'-64  }, // 02
    {	Qt::Key_C,		'c'     , 'C'     , 'C'-64  }, // 03
    {	Qt::Key_D,		'd'     , 'D'     , 'D'-64  }, // 04
    {	Qt::Key_E,		'e'     , 'E'     , 'E'-64  }, // 05
    {	Qt::Key_F,		'f'     , 'F'     , 'F'-64  }, // 06
    {	Qt::Key_G,		'g'     , 'G'     , 'G'-64  }, // 07
    {	Qt::Key_H,		'h'     , 'H'     , 'H'-64  }, // 08
    {	Qt::Key_I,		'i'     , 'I'     , 'I'-64  }, // 09
    {	Qt::Key_J,		'j'     , 'J'     , 'J'-64  }, // 0a 10
    {	Qt::Key_K,		'k'     , 'K'     , 'K'-64  }, // 0b
    {	Qt::Key_L,		'l'     , 'L'     , 'L'-64  }, // 0c
    {	Qt::Key_M,		'm'     , 'M'     , 'M'-64  }, // 0d
    {	Qt::Key_N,		'n'     , 'N'     , 'N'-64  }, // 0e
    {	Qt::Key_O,		'o'     , 'O'     , 'O'-64  }, // 0f
    {	Qt::Key_P,		'p'     , 'P'     , 'P'-64  }, // 10
    {	Qt::Key_Q,		'q'     , 'Q'     , 'Q'-64  }, // 11
    {	Qt::Key_R,		'r'     , 'R'     , 'R'-64  }, // 12
    {	Qt::Key_S,		's'     , 'S'     , 'S'-64  }, // 13
    {	Qt::Key_T,		't'     , 'T'     , 'T'-64  }, // 14 20
    {	Qt::Key_U,		'u'     , 'U'     , 'U'-64  }, // 15
    {	Qt::Key_V,		'v'     , 'V'     , 'V'-64  }, // 16
    {	Qt::Key_W,		'w'     , 'W'     , 'W'-64  }, // 17
    {	Qt::Key_X,		'x'     , 'X'     , 'X'-64  }, // 18
    {	Qt::Key_Y,		'y'     , 'Y'     , 'Y'-64  }, // 19
    {	Qt::Key_Z,		'z'     , 'Z'     , 'Z'-64  }, // 1a
    {	Qt::Key_Shift,		0xffff  , 0xffff  , 0xffff  }, // 1b
    {	Qt::Key_Return,		13      , 13      , 0xffff  }, // 1c
    {	Qt::Key_F11,	        0xffff  , 0xffff  , 0xffff  }, // 1d todo
    {	Qt::Key_F22,		0xffff  , 0xffff  , 0xffff  }, // 1e 30
    {	Qt::Key_Backspace,	8       , 8       , 0xffff  }, // 1f
    {	Qt::Key_F31,		0xffff  , 0xffff  , 0xffff  }, // 20
    {	Qt::Key_F35,		0xffff  , 0xffff  , 0xffff  }, // 21 light
    {	Qt::Key_Escape,		0xffff  , 0xffff  , 0xffff  }, // 22

    // Direction key code are for *UNROTATED* display.
    {	Qt::Key_Up,		0xffff  , 0xffff  , 0xffff  }, // 23
    {	Qt::Key_Right,		0xffff  , 0xffff  , 0xffff  }, // 24
    {	Qt::Key_Left,		0xffff  , 0xffff  , 0xffff  }, // 25
    {	Qt::Key_Down,		0xffff  , 0xffff  , 0xffff  }, // 26

    {	Qt::Key_F33,		0xffff  , 0xffff  , 0xffff  }, // 27 OK
    {	Qt::Key_F12,		0xffff  , 0xffff  , 0xffff  }, // 28 40 home
    {	Qt::Key_1,		'1'     , 'q'     , 'Q'-64  }, // 29
    {	Qt::Key_2,		'2'     , 'w'     , 'W'-64  }, // 2a
    {	Qt::Key_3,		'3'     , 'e'     , 'E'-64  }, // 2b
    {	Qt::Key_4,		'4'     , 'r'     , 'R'-64  }, // 2c
    {	Qt::Key_5,		'5'     , 't'     , 'T'-64  }, // 2d
    {	Qt::Key_6,		'6'     , 'y'     , 'Y'-64  }, // 2e
    {	Qt::Key_7,		'7'     , 'u'     , 'U'-64  }, // 2f
    {	Qt::Key_8,		'8'     , 'i'     , 'I'-64  }, // 30
    {	Qt::Key_9,		'9'     , 'o'     , 'O'-64  }, // 31
    {	Qt::Key_0,		'0'     , 'p'     , 'P'-64  }, // 32 50
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 33
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 34
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 35
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 36
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 37
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 38
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 39
    {	Qt::Key_Minus,		'-'     , 'b'     , 'B'-64  }, // 3a
    {	Qt::Key_Plus,		'+'     , 'n'     , 'N'-64  }, // 3b
    {	Qt::Key_CapsLock,	0xffff  , 0xffff  , 0xffff  }, // 3c 60
    {	Qt::Key_At,		'@'     , 's'     , 'S'-64  }, // 3d
    {	Qt::Key_Question,	'?'     , '?'     , 0xffff  }, // 3e
    {	Qt::Key_Comma,		','     , ','     , 0xffff  }, // 3f
    {	Qt::Key_Period,		'.'     , '.'     , 0xffff  }, // 40
    {	Qt::Key_Tab,		9       , '\\'    , 0xffff  }, // 41
    {	Qt::Key_X,		0xffff 	, 'x'  	  , 'X'-64  }, // 42
    {	Qt::Key_C,		0xffff 	, 'c'     , 'C'-64  }, // 43
    {	Qt::Key_V,		0xffff 	, 'v'     , 'V'-64  }, // 44
    {	Qt::Key_Slash,		'/'     , '/'     , 0xffff  }, // 45
    {	Qt::Key_Apostrophe,	'\''    , '\''    , 0xffff  }, // 46 70
    {	Qt::Key_Semicolon,	';'     , ';'     , 0xffff  }, // 47
    {	Qt::Key_QuoteDbl,	'\"'    , '\"'    , 0xffff  }, // 48
    {	Qt::Key_Colon,		':'     , ':'     , 0xffff  }, // 49
    {	Qt::Key_NumberSign,	'#'     , 'd'     , 'D'-64  }, // 4a
    {	Qt::Key_Dollar,		'$'     , 'f'     , 'F'-64  }, // 4b
    {	Qt::Key_Percent,	'%'     , 'g'     , 'G'-64  }, // 4c
    {	Qt::Key_Underscore,	'_'     , 'h'     , 'H'-64  }, // 4d
    {	Qt::Key_Ampersand,	'&'     , 'j'     , 'J'-64  }, // 4e
    {	Qt::Key_Asterisk,	'*'     , 'k'     , 'K'-64  }, // 4f
    {	Qt::Key_ParenLeft,	'('     , 'l'     , 'L'-64  }, // 50 80
    {	Qt::Key_Delete,		'['     , '['     , '['     }, // 51
    {	Qt::Key_Z,		0xffff 	, 'z'     , 'Z'-64  }, // 52
    {	Qt::Key_Equal,		'='     , 'm'     , 'M'-64  }, // 53
    {	Qt::Key_ParenRight,	')'     , ']'     , ']'     }, // 54
    {	Qt::Key_AsciiTilde,	'~'     , '^'     , '^'     }, // 55
    {	Qt::Key_Less,		'<'     , '{'     , '{'     }, // 56
    {	Qt::Key_Greater,	'>'     , '}'     , '}'     }, // 57
    {	Qt::Key_F9,		0xffff  , 0xffff  , 0xffff  }, // 58 datebook
    {	Qt::Key_F10,		0xffff  , 0xffff  , 0xffff  }, // 59 address
    {	Qt::Key_F13,	        0xffff  , 0xffff  , 0xffff  }, // 5a 90 email
    {	Qt::Key_F30,		' '      , ' '    , 0xffff  }, // 5b select
    {	Qt::Key_Space,		' '     , '|'     , '`'     }, // 5c
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 5d
    {	Qt::Key_Exclam,		'!'     , 'a'     , 'A'-64  }, // 5e
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 5f
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 60
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 61
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 62
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 63
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 64
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 65
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 66
    {	Qt::Key_Meta,		0xffff  , 0xffff  , 0xffff  }, // 67
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 68
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 69
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 6a
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 6b
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 6c
    {	Qt::Key_F34,		0xffff  , 0xffff  , 0xffff  }, // 6d power
    {	Qt::Key_F13,		0xffff  , 0xffff  , 0xffff  }, // 6e mail long
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 6f
    {	Qt::Key_NumLock,	0xffff  , 0xffff  , 0xffff  }, // 70
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 71
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 72
    { 	0x20ac,	0xffff  , 0x20ac , 0x20ac }, // 73 Euro sign
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 74
    {	Qt::Key_F32,		0xffff  , 0xffff  , 0xffff  }, // 75 Sync
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 76
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 77
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 78
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 79
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // 7a
    {	Qt::Key_Return,		0xffff  , 0xffff  , 0xffff  }, // 7b
    {	Qt::Key_Escape,		0xffff  , 0xffff  , 0xffff  }, // 7c
    {	Qt::Key_Up,			0xffff  , 0xffff  , 0xffff  }, // 7d
    {	Qt::Key_Down,		0xffff  , 0xffff  , 0xffff  }, // 7e    
    {	0,			0xffff  , 0xffff  , 0xffff  }
};
#else
// Standard PC101
static const QWSServer::KeyMap keyM[] = {
    {   Qt::Key_unknown,    0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_Escape,     27      , 27      , 0xffff  },
    {   Qt::Key_1,      '1'     , '!'     , 0xffff  },
    {   Qt::Key_2,      '2'     , '@'     , 0xffff  },
    {   Qt::Key_3,      '3'     , '#'     , 0xffff  },
    {   Qt::Key_4,      '4'     , '$'     , 0xffff  },
    {   Qt::Key_5,      '5'     , '%'     , 0xffff  },
    {   Qt::Key_6,      '6'     , '^'     , 0xffff  },
    {   Qt::Key_7,      '7'     , '&'     , 0xffff  },
    {   Qt::Key_8,      '8'     , '*'     , 0xffff  },
    {   Qt::Key_9,      '9'     , '('     , 0xffff  },  // 10
    {   Qt::Key_0,      '0'     , ')'     , 0xffff  },
    {   Qt::Key_Minus,      '-'     , '_'     , 0xffff  },
    {   Qt::Key_Equal,      '='     , '+'     , 0xffff  },
    {   Qt::Key_Backspace,  8       , 8       , 0xffff  },
    {   Qt::Key_Tab,        9       , 9       , 0xffff  },
    {   Qt::Key_Q,      'q'     , 'Q'     , 'Q'-64  },
    {   Qt::Key_W,      'w'     , 'W'     , 'W'-64  },
    {   Qt::Key_E,      'e'     , 'E'     , 'E'-64  },
    {   Qt::Key_R,      'r'     , 'R'     , 'R'-64  },
    {   Qt::Key_T,      't'     , 'T'     , 'T'-64  },  // 20
    {   Qt::Key_Y,      'y'     , 'Y'     , 'Y'-64  },
    {   Qt::Key_U,      'u'     , 'U'     , 'U'-64  },
    {   Qt::Key_I,      'i'     , 'I'     , 'I'-64  },
    {   Qt::Key_O,      'o'     , 'O'     , 'O'-64  },
    {   Qt::Key_P,      'p'     , 'P'     , 'P'-64  },
    {   Qt::Key_BraceLeft,  '['     , '{'     , 0xffff  },
    {   Qt::Key_BraceRight, ']'     , '}'     , 0xffff  },
    {   Qt::Key_Return,     13      , 13      , 0xffff  },
    {   Qt::Key_Control,    0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_A,      'a'     , 'A'     , 'A'-64  },  // 30
    {   Qt::Key_S,      's'     , 'S'     , 'S'-64  },
    {   Qt::Key_D,      'd'     , 'D'     , 'D'-64  },
    {   Qt::Key_F,      'f'     , 'F'     , 'F'-64  },
    {   Qt::Key_G,      'g'     , 'G'     , 'G'-64  },
    {   Qt::Key_H,      'h'     , 'H'     , 'H'-64  },
    {   Qt::Key_J,      'j'     , 'J'     , 'J'-64  },
    {   Qt::Key_K,      'k'     , 'K'     , 'K'-64  },
    {   Qt::Key_L,      'l'     , 'L'     , 'L'-64  },
    {   Qt::Key_Semicolon,  ';'     , ':'     , 0xffff  },
    {   Qt::Key_Apostrophe, '\''    , '"'     , 0xffff  },  // 40
    {   Qt::Key_QuoteLeft,  '`'     , '~'     , 0xffff  },
    {   Qt::Key_Shift,      0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_Backslash,  '\\'    , '|'     , 0xffff  },
    {   Qt::Key_Z,      'z'     , 'Z'     , 'Z'-64  },
    {   Qt::Key_X,      'x'     , 'X'     , 'X'-64  },
    {   Qt::Key_C,      'c'     , 'C'     , 'C'-64  },
    {   Qt::Key_V,      'v'     , 'V'     , 'V'-64  },
    {   Qt::Key_B,      'b'     , 'B'     , 'B'-64  },
    {   Qt::Key_N,      'n'     , 'N'     , 'N'-64  },
    {   Qt::Key_M,      'm'     , 'M'     , 'M'-64  },  // 50
    {   Qt::Key_Comma,      ','     , '<'     , 0xffff  },
    {   Qt::Key_Period,     '.'     , '>'     , 0xffff  },
    {   Qt::Key_Slash,      '/'     , '?'     , 0xffff  },
    {   Qt::Key_Shift,      0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_Asterisk,   '*'     , '*'     , 0xffff  },
    {   Qt::Key_Alt,        0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_Space,      ' '     , ' '     , 0xffff  },
    {   Qt::Key_CapsLock,   0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_F1,     0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_F2,     0xffff  , 0xffff  , 0xffff  },  // 60
    {   Qt::Key_F3,     0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_F4,     0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_F5,     0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_F6,     0xffff  , 0xffff  , 0xffff  },
#if defined(QT_KEYPAD_MODE)
    {   Qt::Key_Menu,     0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_Back,     0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_Yes,     0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_No,        0xffff  , 0xffff  , 0xffff  },
#else
    {   Qt::Key_F7,     0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_F8,     0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_F9,     0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_F10,        0xffff  , 0xffff  , 0xffff  },
#endif
    {   Qt::Key_NumLock,    0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_ScrollLock, 0xffff  , 0xffff  , 0xffff  },  // 70
    {   Qt::Key_7,      '7'     , '7'     , 0xffff  },
    {   Qt::Key_8,      '8'     , '8'     , 0xffff  },
    {   Qt::Key_9,      '9'     , '9'     , 0xffff  },
    {   Qt::Key_Minus,      '-'     , '-'     , 0xffff  },
    {   Qt::Key_4,      '4'     , '4'     , 0xffff  },
    {   Qt::Key_5,      '5'     , '5'     , 0xffff  },
    {   Qt::Key_6,      '6'     , '6'     , 0xffff  },
    {   Qt::Key_Plus,       '+'     , '+'     , 0xffff  },
    {   Qt::Key_1,      '1'     , '1'     , 0xffff  },
    {   Qt::Key_2,      '2'     , '2'     , 0xffff  },  // 80
    {   Qt::Key_3,      '3'     , '3'     , 0xffff  },
    {   Qt::Key_0,      '0'     , '0'     , 0xffff  },
    {   Qt::Key_Period,     '.'     , '.'     , 0xffff  },
    {   Qt::Key_unknown,    0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_unknown,    0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_Less,   '<'     , '>'  , 0xffff  },
#if defined(QT_KEYPAD_MODE)
    {   Qt::Key_Call,        0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_Hangup,        0xffff  , 0xffff  , 0xffff  },
#else
    {   Qt::Key_F11,        0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_F12,        0xffff  , 0xffff  , 0xffff  },
#endif
    {   Qt::Key_unknown,    0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_unknown,    0xffff  , 0xffff  , 0xffff  }, // 90
    {   Qt::Key_unknown,    0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_unknown,    0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_unknown,    0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_unknown,    0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_unknown,    0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_Enter,      13      , 13      , 0xffff  },
    {   Qt::Key_Control,    0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_Slash,		'/'     , '/'     , 0xffff  },
    {   Qt::Key_unknown,    0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_Meta,		0xffff  , 0xffff  , 0xffff  }, // 100
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // break
    {	Qt::Key_Home,	    0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_Up,		0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_Prior,		0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_Left,		0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_Right,		0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_End,		0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_Down,		0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_Next,		0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_Insert,		0xffff  , 0xffff  , 0xffff  }, // 110
    {	Qt::Key_Delete,		0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // macro
    {   Qt::Key_F13,        0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_F14,        0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_Help,       0xffff  , 0xffff  , 0xffff  },
#if defined(QT_KEYPAD_MODE)
    {	Qt::Key_Select,	    0xffff  , 0xffff  , 0xffff  }, // do
#else
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  }, // do
#endif
    {   Qt::Key_F17,        0xffff  , 0xffff  , 0xffff  },
    {   Qt::Key_Plus,       '+'     , '-'     , 0xffff  },
    {	Qt::Key_Pause,		0xffff  , 0xffff  , 0xffff  }, // 120
    {	Qt::Key_F31,	0xffff  , 0xffff  , 0xffff  }, // IM toggle
    {	Qt::Key_F32,	0xffff  , 0xffff  , 0xffff  }, // Sync
    {	Qt::Key_F34,	0xffff  , 0xffff  , 0xffff  }, // Power
    {	Qt::Key_F35,	0xffff  , 0xffff  , 0xffff  }, // Backlight
#if defined(QT_KEYPAD_MODE)
    {	Qt::Key_Context1,	0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_Context2,	0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_Context3,	0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_Context4,	0xffff  , 0xffff  , 0xffff  },
#else
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  },
    {	Qt::Key_unknown,	0xffff  , 0xffff  , 0xffff  },
#endif
    {   0,          0xffff  , 0xffff  , 0xffff  }
};
#endif
static const int keyMSize = sizeof(keyM)/sizeof(QWSServer::KeyMap)-1;
static QIntDict<QWSServer::KeyMap> *overrideMap = 0;

/*!
  Changes the mapping of the keyboard; adding the scancode to Unicode
  mappings from \a map. The server takes over ownership of \a map
  and will delete it. Use QCollection::setAutoDelete() to control
  whether the contents of \a map should be deleted.

  Passing a null pointer for \a map will revert to the default keymap.
*/

void QWSServer::setOverrideKeys( QIntDict<QWSServer::KeyMap> *map )
{
    delete overrideMap;
    overrideMap = map;
}

#if !defined(_OS_QNX6_)

/*!
  \class QWSKeyboardHandler qkeyboard_qws.h
  \brief Keyboard driver/handler for Qt/Embedded

  The keyboard driver/handler handles events from system devices and
  generates key events.

  A QWSKeyboardHandler will usually open some system device in its
  constructor, create a QSocketNotifier on that opened device and when
  it receives data, it will call processKeyEvent() to send the event
  to Qt/Embedded for relaying to clients.
*/

/*!
  Subclasses call this to send a key event. The server may additionally
  filter it before sending it on to applications.

  <ul>
  <li>\a unicode is the Unicode value for the key, or 0xffff if none is appropriate.
  <li>\a keycode is the Qt keycode for the key (see Qt::Key).
       for the list of codes).
  <li>\a modifiers is the set of modifier keys (see Qt::Modifier).
  <li>\a isPress says whether this is a press or a release.
  <li>\a autoRepeat says whether this event was generated by an auto-repeat
	    mechanism, or an actual key press.
  </ul>
*/
void QWSKeyboardHandler::processKeyEvent(int unicode, int keycode, int modifiers,
			bool isPress, bool autoRepeat)
{
    qwsServer->processKeyEvent( unicode, keycode, modifiers, isPress, autoRepeat );
}

class QWSPC101KeyboardHandler : public QWSKeyboardHandler
{
    Q_OBJECT
public:
    QWSPC101KeyboardHandler();
    virtual ~QWSPC101KeyboardHandler();

    void doKey(uchar scancode);

    
    void restoreLeds();
    
private:
    bool shift;
    bool alt;
    bool ctrl;
#if defined(QT_QWS_SL5XXX)
    bool meta;
    bool fn;
    bool numLock;
#endif
    bool caps;
#if defined(QT_QWS_IPAQ)
    uint ipaq_return_pressed:1;
#endif
#ifndef QT_QWS_USE_KEYCODES
    int extended;
#endif
    int modifiers;
    int prevuni;
    int prevkey;

#ifdef QT_QWS_AUTOREPEAT_MANUALLY
    QWSKeyboardRepeater *rep;
#endif
};

void QWSPC101KeyboardHandler::restoreLeds()
{
    char leds;
    ioctl(0, KDGETLED, &leds);
    leds = leds & ~LED_CAP;
    if ( caps ) leds |= LED_CAP;
    ioctl(0, KDSETLED, leds);
}

/*
 * This class was extended by:
 * 
 * Copyright 2002 Chris Larson
 * Copyright 2004, 2005 Holger Hans Peter Freyther
 */
class QWSTtyKeyboardHandler : public QWSPC101KeyboardHandler
{
    Q_OBJECT
public:
    QWSTtyKeyboardHandler(const QString&);
    virtual ~QWSTtyKeyboardHandler();
    void readKeyboardMap();
    void readUnicodeMap();
    void handleKey(unsigned int key, bool release);

private slots:
    void readKeyboardData();

private:
    void modifyModifier( int map, int modify, bool release );
    void modifyLock( unsigned int lock, bool release );
    void handleExtra( unsigned int key, bool release );
    static void restoreLeds();
    static void toggleLed(unsigned int);
    int  map_to_modif ();

private:
    struct termios origTermData;
    unsigned short acm[E_TABSZ];
    struct KeyMap {
        enum ExtraKey{
            Key_AltGr    = 0x01ffff,
            Key_Console1  = 0x02ffff,
            Key_Console2  = 0x03ffff,
            Key_Console3  = 0x04ffff,
            Key_Console4  = 0x05ffff,
            Key_Console5  = 0x06ffff,
            Key_Console6  = 0x07ffff,
            Key_Console7  = 0x08ffff,
            Key_Console8  = 0x09ffff,
            Key_Console9  = 0x0affff,
            Key_Console10 = 0x0bffff,
            Key_Console11 = 0x0cffff,
            Key_Console12 = 0x0dffff,
            Key_NumLock   = 0x0effff,
            Key_ShiftLock = 0x0fffff,
            Key_CtrlLock  = 0x10ffff,
            Key_AltLock   = 0x11ffff,
            Key_AltGrLock = 0x12ffff,
	    Key_ShiftL    = 0x130000,
	    Key_ShiftR    = 0x130001,
	    Key_IncConsole= 0x130002,
	    Key_DecConsole= 0x130003,
	    Key_NotMapped = 0x130004,
        };

        KeyMap( Qt::Key _key = Qt::Key_unknown, unsigned short _code = 0 )
            : key( _key ), code( _code )
            {}
        KeyMap( ExtraKey _key,  unsigned short _code )
            : key( _key ), code( _code )
            {}
        unsigned int    key; // 16 Bit
        unsigned short code;
    };

    KeyMap kernel_map[(1<<KG_CAPSSHIFT)][NR_KEYS];
    int current_map;
    int modifier;
    bool numlock  : 1;
    bool capslock : 1;

protected:
    void processKeyEvent(int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat );
};


// can't ifdef this out because moc runs on this file
class QWSSamsungKeypadHandler : public QWSPC101KeyboardHandler
{
    Q_OBJECT
public:
    QWSSamsungKeypadHandler(const QString&);
    virtual ~QWSSamsungKeypadHandler();

private slots:
    void readKeyboardData();
};

class QWSUsbKeyboardHandler : public QWSPC101KeyboardHandler
{
    Q_OBJECT
public:
    QWSUsbKeyboardHandler(const QString& device);
    virtual ~QWSUsbKeyboardHandler();

private slots:
    void readKeyboardData();

private:
    int fd;
    QWSServer::KeyMap *lastPress;
};

class QWSVr41xxButtonsHandler : public QWSKeyboardHandler
{
    Q_OBJECT
public:
    QWSVr41xxButtonsHandler();
    virtual ~QWSVr41xxButtonsHandler();

    bool isOpen() { return buttonFD > 0; }

private slots:
    void readKeyboardData();

private:
    QString terminalName;
    int buttonFD;
    int kbdIdx;
    int kbdBufferLen;
    unsigned char *kbdBuffer;
    QSocketNotifier *notifier;
};

class QWSVFbKeyboardHandler : public QWSKeyboardHandler
{
    Q_OBJECT
public:
    QWSVFbKeyboardHandler();
    virtual ~QWSVFbKeyboardHandler();

    bool isOpen() { return fd > 0; }

private slots:
    void readKeyboardData();

private:
    QString terminalName;
    int fd;
    int kbdIdx;
    int kbdBufferLen;
    unsigned char *kbdBuffer;
    QSocketNotifier *notifier;
};


static void vtSwitchHandler(int /*sig*/)
{
#if !defined(_OS_FREEBSD_) && !defined(_OS_SOLARIS_)
    if (vtActive) {
	qwsServer->enablePainting(false);
	qt_screen->save();
	if (ioctl(kbdFD, VT_RELDISP, 1) == 0) {
	    vtActive = false;
	    qwsServer->closeMouse();
	}
	else {
	    qwsServer->enablePainting(true);
	}
	usleep(200000);
    }
    else {
	if (ioctl(kbdFD, VT_RELDISP, VT_ACKACQ) == 0) {
	    qwsServer->enablePainting(true);
	    vtActive = true;
	    qt_screen->restore();
	    qwsServer->openMouse();
	    qwsServer->refresh();
	}
    }
    signal(VTSWITCHSIG, vtSwitchHandler);
#endif
}

//
// PC-101 type keyboards
//



static QWSPC101KeyboardHandler *currentPC101=0;

bool qwsSetKeyboardAutoRepeat( int delay, int period )
{
#ifdef QT_QWS_AUTOREPEAT_MANUALLY
    if ( QWSKeyboardRepeater::current )
	QWSKeyboardRepeater::current->setAutoRepeat( delay, period );
    return QWSKeyboardRepeater::current != 0;
#else
    Q_UNUSED(delay);
    Q_UNUSED(period);
    return FALSE;
#endif
}

bool qwsGetKeyboardAutoRepeat( int *delay, int *period )
{
#ifdef QT_QWS_AUTOREPEAT_MANUALLY
    if ( QWSKeyboardRepeater::current )
	QWSKeyboardRepeater::current->getAutoRepeat( delay, period );
    return QWSKeyboardRepeater::current != 0;
#else
    Q_UNUSED(delay);
    Q_UNUSED(period);
    return FALSE;
#endif
}

void qwsRestoreKeyboardLeds()
{
    if ( currentPC101 )
	currentPC101->restoreLeds();
}



QWSPC101KeyboardHandler::QWSPC101KeyboardHandler()
{
    shift = false;
    alt   = false;
    ctrl  = false;
#ifndef QT_QWS_USE_KEYCODES
    extended = 0;
#endif
    prevuni = 0;
    prevkey = 0;
    caps = FALSE;
#if defined(QT_QWS_SL5XXX)
    meta = FALSE;
    fn = FALSE;

    numLock = FALSE;
#if 0
    sharp_kbdctl_modifstat  st;
    int dev = ::open("/dev/sharp_kbdctl", O_RDWR);
    if( dev >= 0 ) {
	memset(&st, 0, sizeof(st));
	st.which = 3;
	int ret = ioctl(dev, SHARP_KBDCTL_GETMODIFSTAT, (char*)&st);
	if( !ret )
	    numLock = (bool)st.stat;
	::close(dev);
    }
#endif
#endif
#if defined(QT_QWS_IPAQ)
    // iPAQ Action Key has ScanCode 0x60: 0x60|0x80 = 0xe0 == extended mode 1 !
    ipaq_return_pressed = FALSE;
#endif
#ifdef QT_QWS_AUTOREPEAT_MANUALLY
    rep = new QWSKeyboardRepeater(this);
#endif
    currentPC101 = this;
}

QWSPC101KeyboardHandler::~QWSPC101KeyboardHandler()
{
    if ( currentPC101 == this )
	currentPC101 = 0;
}

void QWSPC101KeyboardHandler::doKey(uchar code)
{
}


//
// Tty keyboard
//
/*
 * Include the alternative implementation
 */
#include "kernelkeyboard.cpp"

typedef struct {
    unsigned short key;
    unsigned short status;
} SamsungKeypadInput;

QWSSamsungKeypadHandler::QWSSamsungKeypadHandler(const QString& device)
{
    kbdFD=open(device.isEmpty() ? "/dev/keypad/0" : device.latin1(), O_RDONLY, 0);
    if( kbdFD < 0 )
	qWarning("could not open keypad device");

    if ( kbdFD >= 0 ) {
	QSocketNotifier *notifier;
	notifier = new QSocketNotifier( kbdFD, QSocketNotifier::Read, this );
	connect( notifier, SIGNAL(activated(int)),this,
		 SLOT(readKeyboardData()) );
    }
}

QWSSamsungKeypadHandler::~QWSSamsungKeypadHandler()
{
    if (kbdFD >= 0)
    {
	::close(kbdFD);
	kbdFD = -1;
    }
}

void QWSSamsungKeypadHandler::readKeyboardData()
{
    SamsungKeypadInput input;
    int n = ::read(kbdFD, &input, sizeof(SamsungKeypadInput) );
    if( n < sizeof(SamsungKeypadInput) )
    {
	qWarning("Error reading input from keypad device.");
	return;
    }
    unsigned short key = input.key;
    unsigned short unicode = 0;
    int modifiers = 0;
    QWSServer::KeyMap *km = (overrideMap ? overrideMap->find( input.key ) : 0);
    if( km ) {
	key = km->key_code;
	unicode = km->unicode;
    }
    switch( key ) {
#ifdef QT_KEYPAD_MODE
	case Key_Menu:
	case Key_Back:
	case Key_Yes:
	case Key_No:
	case Key_Call:
	case Key_Hangup:
	case Key_Select:
	case Key_Context1:
	case Key_Context2:
	case Key_Context3:
	case Key_Context4:
	    modifiers |= Qt::Keypad;
	    break;
#endif
    }
    processKeyEvent( unicode, key, modifiers, input.status != 0, FALSE );
}

/* USB driver */

QWSUsbKeyboardHandler::QWSUsbKeyboardHandler(const QString& device)
{
    lastPress = 0;
    fd = ::open(device.isEmpty()?"/dev/input/event0":device.latin1(),O_RDONLY, 0);
    if ( fd >= 0 ) {
	QSocketNotifier *notifier;
	notifier = new QSocketNotifier( fd, QSocketNotifier::Read, this );
	connect( notifier, SIGNAL(activated(int)),this,
		 SLOT(readKeyboardData()) );
    }
}

QWSUsbKeyboardHandler::~QWSUsbKeyboardHandler()
{
    ::close(fd);
}

struct Myinputevent {

    unsigned int dummy1;
    unsigned int dummy2;
    unsigned short type;
    unsigned short code;
    unsigned int value;

};

void QWSUsbKeyboardHandler::readKeyboardData()
{
    Myinputevent event;
    int n = ::read(fd, &event, sizeof(Myinputevent) );
    if ( n != 16 )
	return;
#ifdef QT_QWS_TIP2
    // custom scan codes - translate them and create a key event immediately
    if( overrideMap && event.value == 0 || overrideMap->find( event.value ) ) 
    {
	if( event.value )
	{
	    int modifiers = 0;
	    QWSServer::KeyMap *km = overrideMap->find( event.value );
	    switch( km->unicode ) 
	    {
		case Key_Menu:
		case Key_Back:
		case Key_Yes:
		case Key_No:
		case Key_Call:
		case Key_Hangup:
		case Key_Select:
		case Key_Context1:
		case Key_Context2:
		case Key_Context3:
		case Key_Context4:
		{
		    modifiers |= Keypad;
		    break;
		}
		default:
		    break;
	    }
	    if( km->key_code == Key_F10 && QCopChannel::isRegistered( "QPE/System" ) ) {
		//hardcoded for now
		QCopChannel::send( "QPE/System", "showHomeScreen()" );
	    } else {
		processKeyEvent( km->unicode, km->key_code, modifiers,
							TRUE, FALSE );
	    }
	    lastPress = km;
	} 
	else if( lastPress ) 
	{
	    processKeyEvent( lastPress->unicode, lastPress->key_code, 0, 
							    FALSE, FALSE );
	    lastPress = 0;
	}
    } 
    else
#endif
    {
	int key=event.code;
	if(key==103) {
	    processKeyEvent( 0, Qt::Key_Up, 0, event.value!=0, false );
	} else if(key==106) {
	    processKeyEvent( 0, Qt::Key_Right, 0, event.value!=0, false  );
	} else if(key==108) {
	    processKeyEvent( 0, Qt::Key_Down, 0, event.value!=0, false );
	} else if(key==105) {
	    processKeyEvent( 0, Qt::Key_Left, 0, event.value!=0, false );
	} else {
	    if(event.value==0) {
		key=key | 0x80;
	    }
	    doKey(key);
	}
    }
}

/*
 * YOPY buttons driver
 * Contributed by Ron Victorelli (victorrj at icubed.com)
 */

QWSyopyButtonsHandler::QWSyopyButtonsHandler() : QWSKeyboardHandler()
{
#ifdef QT_QWS_YOPY
    terminalName = "/dev/tty1";
    buttonFD = -1;
    notifier = 0;

    if ((buttonFD = ::open(terminalName, O_RDWR | O_NDELAY, 0)) < 0) {
	qFatal("Cannot open %s\n", terminalName.latin1());
    } else {

       tcsetpgrp(buttonFD, getpgid(0));

       /* put tty into "straight through" mode.
       */
       if (tcgetattr(buttonFD, &oldT) < 0) {
           qFatal("Linux-kbd: tcgetattr failed");
       }

       newT = oldT;
       newT.c_lflag &= ~(ICANON | ECHO  | ISIG);
       newT.c_iflag &= ~(ISTRIP | IGNCR | ICRNL | INLCR | IXOFF | IXON);
       newT.c_iflag |= IGNBRK;
       newT.c_cc[VMIN]  = 0;
       newT.c_cc[VTIME] = 0;


       if (tcsetattr(buttonFD, TCSANOW, &newT) < 0) {
           qFatal("Linux-kbd: TCSANOW tcsetattr failed");
       }

       if (ioctl(buttonFD, KDSKBMODE, K_MEDIUMRAW) < 0) {
           qFatal("Linux-kbd: KDSKBMODE tcsetattr failed");
       }

	notifier = new QSocketNotifier( buttonFD, QSocketNotifier::Read, this );
	connect( notifier, SIGNAL(activated(int)),this,
		 SLOT(readKeyboardData()) );
    }
#endif
}

QWSyopyButtonsHandler::~QWSyopyButtonsHandler()
{
#ifdef QT_QWS_YOPY
    if ( buttonFD > 0 ) {
	::close( buttonFD );
	buttonFD = -1;
    }
#endif
}

void QWSyopyButtonsHandler::readKeyboardData()
{
#ifdef QT_QWS_YOPY
    uchar buf[1];
    char c='1';
    int fd;

    int n = ::read(buttonFD,buf,1);
    if (n<0) {
	qDebug("Keyboard read error %s",strerror(errno));
    } else {
	uint code = buf[0]&YPBUTTON_CODE_MASK;
        bool press = !(buf[0]&0x80);
        // printf("Key=%d/%d/%d\n",buf[1],code,press);
        int k=(-1);
        switch(code) {
          case 39:       k=Qt::Key_Up;     break;
          case 44:       k=Qt::Key_Down;   break;
          case 41:       k=Qt::Key_Left;   break;
          case 42:       k=Qt::Key_Right;  break;
          case 56:       k=Qt::Key_F1;     break; //windows
          case 29:       k=Qt::Key_F2;     break; //cycle
          case 24:       k=Qt::Key_F3;     break; //record
          case 23:       k=Qt::Key_F4;     break; //mp3
          case 4:        k=Qt::Key_F5;     break; // PIMS
          case 1:        k=Qt::Key_Escape; break; // Escape
          case 40:       k=Qt::Key_Up;     break; // prev
          case 45:       k=Qt::Key_Down;   break; // next
          case 35:       if( !press ) {
                           fd = ::open("/proc/sys/pm/sleep",O_RDWR,0);
                           if( fd >= 0 ) {
                               ::write(fd,&c,sizeof(c));
                               ::close(fd);
                               //
                               // Updates all widgets.
                               //
                               QWidgetList  *list = QApplication::allWidgets();
                               QWidgetListIt it( *list );          // iterate over the widgets
                               QWidget * w;
                               while ( (w=it.current()) != 0 ) {   // for each widget...
                                 ++it;
                                 w->update();
                               }
                               delete list;
                               // qApp->desktop()->repaint();
                           }
                         }
                         break;

          default: k=(-1); break;
        }

	if ( k >= 0 ) {
		qwsServer->processKeyEvent( 0, k, 0, press, false );
	}
    }
#endif
}


/*
 * vr41xx buttons driver
 */

QWSVr41xxButtonsHandler::QWSVr41xxButtonsHandler() : QWSKeyboardHandler()
{
#ifdef QT_QWS_CASSIOPEIA
    terminalName = "/dev/buttons";
    buttonFD = -1;
    notifier = 0;

    if ((buttonFD = ::open(terminalName, O_RDWR | O_NDELAY, 0)) < 0)
    {
	qWarning("Cannot open %s\n", terminalName.latin1());
    }

    if ( buttonFD >= 0 ) {
	notifier = new QSocketNotifier( buttonFD, QSocketNotifier::Read, this );
	connect( notifier, SIGNAL(activated(int)),this,
		 SLOT(readKeyboardData()) );
    }

    kbdBufferLen = 80;
    kbdBuffer = new unsigned char [kbdBufferLen];
    kbdIdx = 0;
#endif
}

QWSVr41xxButtonsHandler::~QWSVr41xxButtonsHandler()
{
#ifdef QT_QWS_CASSIOPEIA
    if ( buttonFD > 0 ) {
	::close( buttonFD );
	buttonFD = -1;
    }
    delete notifier;
    notifier = 0;
    delete [] kbdBuffer;
#endif
}

void QWSVr41xxButtonsHandler::readKeyboardData()
{
#ifdef QT_QWS_CASSIOPEIA
    int n = 0;
    do {
	n  = ::read(buttonFD, kbdBuffer+kbdIdx, kbdBufferLen - kbdIdx );
	if ( n > 0 )
	    kbdIdx += n;
    } while ( n > 0 );

    int idx = 0;
    while ( kbdIdx - idx >= 2 ) {
	unsigned char *next = kbdBuffer + idx;
	unsigned short *code = (unsigned short *)next;
	int keycode = Qt::Key_unknown;
	switch ( (*code) & 0x0fff ) {
	    case 0x7:
		keycode = Qt::Key_Up;
		break;
	    case 0x9:
		keycode = Qt::Key_Right;
		break;
	    case 0x8:
		keycode = Qt::Key_Down;
		break;
	    case 0xa:
		keycode = Qt::Key_Left;
		break;
	    case 0x3:
		keycode = Qt::Key_Up;
		break;
	    case 0x4:
		keycode = Qt::Key_Down;
		break;
	    case 0x1:
		keycode = Qt::Key_Return;
		break;
	    case 0x2:
		keycode = Qt::Key_F4;
		break;
	    default:
		qDebug("Unrecognised key sequence %d", (int)code );
	}
	if ( (*code) & 0x8000 )
	    processKeyEvent( 0, keycode, 0, FALSE, FALSE );
	else
	    processKeyEvent( 0, keycode, 0, TRUE, FALSE );
/*
	unsigned short t = *code;
	for ( int i = 0; i < 16; i++ ) {
	    keycode = (t & 0x8000) ? Qt::Key_1 : Qt::Key_0;
	    int unicode = (t & 0x8000) ? '1' : '0';
	    processKeyEvent( unicode, keycode, 0, TRUE, FALSE );
	    processKeyEvent( unicode, keycode, 0, FALSE, FALSE );
	    t <<= 1;
	}
	keycode = Qt::Key_Space;
//	processKeyEvent( ' ', keycode, 0, TRUE, FALSE );
//	processKeyEvent( ' ', keycode, 0, FALSE, FALSE );
*/
	idx += 2;
    }

    int surplus = kbdIdx - idx;
    for ( int i = 0; i < surplus; i++ )
	kbdBuffer[i] = kbdBuffer[idx+i];
    kbdIdx = surplus;
#endif
}


/*
 * Virtual framebuffer keyboard driver
 */

#ifndef QT_NO_QWS_VFB
#include "qvfbhdr.h"
extern int qws_display_id;
#endif

QWSVFbKeyboardHandler::QWSVFbKeyboardHandler()
{
    kbdFD = -1;
#ifndef QT_NO_QWS_VFB
    kbdIdx = 0;
    kbdBufferLen = sizeof( QVFbKeyData ) * 5;
    kbdBuffer = new unsigned char [kbdBufferLen];

    terminalName = QString(QT_VFB_KEYBOARD_PIPE).arg(qws_display_id);

    if ((kbdFD = ::open(terminalName.local8Bit().data(), O_RDWR | O_NDELAY)) < 0) {
	qDebug( "Cannot open %s (%s)", terminalName.latin1(),
	strerror(errno));
    } else {
	// Clear pending input
	char buf[2];
	while (::read(kbdFD, buf, 1) > 0) { }

	notifier = new QSocketNotifier( kbdFD, QSocketNotifier::Read, this );
	connect(notifier, SIGNAL(activated(int)),this, SLOT(readKeyboardData()));
    }
#endif
}

QWSVFbKeyboardHandler::~QWSVFbKeyboardHandler()
{
#ifndef QT_NO_QWS_VFB
    if ( kbdFD >= 0 )
	::close( kbdFD );
    delete [] kbdBuffer;
#endif
}


void QWSVFbKeyboardHandler::readKeyboardData()
{
#ifndef QT_NO_QWS_VFB
    int n;
    do {
	n  = ::read(kbdFD, kbdBuffer+kbdIdx, kbdBufferLen - kbdIdx );
	if ( n > 0 )
	    kbdIdx += n;
    } while ( n > 0 );

    int idx = 0;
    while ( kbdIdx - idx >= (int)sizeof( QVFbKeyData ) ) {
	QVFbKeyData *kd = (QVFbKeyData *)(kbdBuffer + idx);
	if ( kd->unicode == 0 && kd->modifiers == 0 && kd->press ) {
	    // magic exit key
	    qWarning( "Instructed to quit by Virtual Keyboard" );
	    qApp->quit();
	}
#ifdef QT_KEYPAD_MODE
	QWSServer::KeyMap *currentKey = overrideMap ? overrideMap->find( (kd->unicode >> 16) ) : 0;
	if ( currentKey )
	    processKeyEvent( currentKey->unicode, currentKey->key_code,
				     kd->modifiers, kd->press, kd->repeat );
	else
#endif
	processKeyEvent( kd->unicode&0xffff, kd->unicode>>16,
				 kd->modifiers, kd->press, kd->repeat );

	idx += sizeof( QVFbKeyData );
    }

    int surplus = kbdIdx - idx;
    for ( int i = 0; i < surplus; i++ )
	kbdBuffer[i] = kbdBuffer[idx+i];
    kbdIdx = surplus;
#endif
}


/*
 * keyboard driver instantiation
 */

QWSKeyboardHandler *QWSServer::newKeyboardHandler( const QString &spec )
{
    QWSKeyboardHandler *handler = 0;

    QString device;
    QString type;
    int colon=spec.find(':');
    if ( colon>=0 ) {
	type = spec.left(colon);
	device = spec.mid(colon+1);
    } else {
	type = spec;
    }

    if ( type == "Buttons" ) {
#if defined(QT_QWS_YOPY)
	handler = new QWSyopyButtonsHandler();
#elif defined(QT_QWS_CASSIOPEIA)
	handler = new QWSVr41xxButtonsHandler();
#endif
    } else if ( type == "QVFbKeyboard" ) {
	handler = new QWSVFbKeyboardHandler();
    } else if ( type == "USB" ) {
	handler = new QWSUsbKeyboardHandler(device);
    } else if ( type == "TTY" ) {
	handler = new QWSTtyKeyboardHandler(device);
    } 
    else if( type == "Samsung" )  {
	handler = new QWSSamsungKeypadHandler(device);
    } 
    else {
	qWarning( "Keyboard type %s:%s unsupported", spec.latin1(), device.latin1() );
    }

    return handler;
}

#include "qkeyboard_qws.moc"

#endif // QNX6

/*!
  \internal
  Returns the map of scancodes to Qt key codes and text
*/
const QWSServer::KeyMap *QWSServer::keyMap()
{
    return keyM;
}

#endif // QT_NO_QWS_KEYBOARD


