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
 * aceutils.h
 *          Allows user some functions that are not normally support
 *	    in Linux.  
 */

/* Functions for keyboard access in Linux. */
#include <unistd.h>
int kbhit();
int readch();
void AceUtilsStart();
void close_keyboard();
void AceUtilsExit();
/*int eof( int * );*/


