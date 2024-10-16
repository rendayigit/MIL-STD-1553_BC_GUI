/* Data Device Corp.
 *
 *        Advanced Communication Engine Integrated 1553 Terminal
 *        (ACE) 'C' Software Library
 *
 *        BUS-69082(3) sample  rel 4.0     29-NOV-2000
 *
 *        Copyright (c) 1995,1996,1997,2000 by Data Device Corp.
 *        All Rights Reserved.
 *
 * aceutils.c
 *          Allows user some functions that are not normally support
 *	    in Linux.  
 */

#include <stdio.h>
#include <termios.h>
/* #include <term.h> */
/* #include <curses.h> */
#include <unistd.h>
#include <stdlib.h>	/* For 'exit()'.  kwh */
#include "aceutils.h"

/* Keyboard interface routines for Linux. */
static struct termios initial_settings, new_settings;
static int peek_character = -1;

int readch()
{
    char ch;

    if(peek_character != -1) {
        ch = peek_character;
        peek_character = -1;
        return ch;
    }
    read(0,&ch,1);
    return ch;
}

void AceUtilsStart()
{
    tcgetattr(0,&initial_settings);
    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ISIG;
    new_settings.c_cc[VMIN] = 1;
    new_settings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_settings);
}

/*	Used to reset keyboard to initial state in Linux. */
void close_keyboard()
{
	tcsetattr(0, TCSANOW, &initial_settings);
}

/*	Cleanup function. */
void AceUtilsExit()
{
	close_keyboard();
	exit(0);
}

/* End of file test. */
/*int eof( int *fp )
{
	return feof(fp);
}
*/
