/*
 * Copyright (c) 1981 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 */

#ifndef lint
static char sccsid[] = "@(#)curses.c	5.4 (Berkeley) %G%";
#endif /* not lint */

/*
 * Define global variables
 *
 */
# include	"curses.h"

bool	_echoit		= TRUE,	/* set if stty indicates ECHO		*/
	_rawmode	= FALSE,/* set if stty indicates RAW mode	*/
	My_term		= FALSE,/* set if user specifies terminal type	*/
	_endwin		= FALSE;/* set if endwin has been called	*/

char	ttytype[50],		/* long name of tty			*/
	*Def_term	= "unknown";	/* default terminal type	*/

int	_tty_ch		= 1,	/* file channel which is a tty		*/
	LINES,			/* number of lines allowed on screen	*/
	COLS,			/* number of columns allowed on screen	*/
	_res_flg;		/* sgtty flags for reseting later	*/

WINDOW	*stdscr		= NULL,
	*curscr		= NULL;

# ifdef DEBUG
FILE	*outf;			/* debug output file			*/
# endif

SGTTY	_tty;			/* tty modes				*/

bool	AM, BS, CA, DA, DB, EO, HC, HZ, IN, MI, MS, NC, NS, OS, UL, XB, XN,
	XT, XS, XX;
char	*AL, *BC, *BT, *CD, *CE, *CL, *CM, *CR, *CS, *DC, *DL, *DM,
	*DO, *ED, *EI, *K0, *K1, *K2, *K3, *K4, *K5, *K6, *K7, *K8,
	*K9, *HO, *IC, *IM, *IP, *KD, *KE, *KH, *KL, *KR, *KS, *KU,
	*LL, *MA, *ND, *NL, *RC, *SC, *SE, *SF, *SO, *SR, *TA, *TE,
	*TI, *UC, *UE, *UP, *US, *VB, *VS, *VE, *AL_PARM, *DL_PARM,
	*UP_PARM, *DOWN_PARM, *LEFT_PARM, *RIGHT_PARM;
char	PC;

/*
 * From the tty modes...
 */

bool	GT, NONL, UPPERCASE, normtty, _pfast;
