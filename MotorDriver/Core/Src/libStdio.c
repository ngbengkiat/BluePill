// ---------------------------------------------------------------------------------
//	Copyright 2001, 2002 Georges Menie (www.menie.org)
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// ---------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------
//	putchar is the only external dependency for this file,
//	if you have a working putchar, just remove the following
//	define. If the function should be called something else,
//	replace outbyte(c) by your own function call.
// ---------------------------------------------------------------------------------

#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>

int _write(int file, char *ptr, int len);

#ifdef xxx
/*
 sbrk
 Increase program data space.
 Malloc and related functions depend on this
 */
caddr_t _sbrk(int incr) {

    extern char _ebss; // Defined by the linker
    static char *heap_end;
    char *prev_heap_end;

    if (heap_end == 0) {
        heap_end = &_ebss;
    }
    prev_heap_end = heap_end;

char * stack = (char*) __get_MSP();
     if (heap_end + incr >  stack)
     {
         _write (STDERR_FILENO, "Heap and stack collision\n", 25);
         errno = ENOMEM;
         return  (caddr_t) -1;
         //abort ();
     }

    heap_end += incr;
    return (caddr_t) prev_heap_end;

}
#endif

/*
 read
 Read a character to a file. `libc' subroutines will use this system routine for input from all files, including stdin
 Returns -1 on error or blocks until the number of characters have been read.
 */

#ifdef xxx
int _read(int file, char *ptr, int len) {
    int n;
    int num = 0;
    switch (file) {
    case STDIN_FILENO:
        for (n = 0; n < len; n++) {

            char c = GetChar();
            getchar();
            *ptr++ = c;
            num++;
        }
        break;
    default:
        errno = EBADF;
        return -1;
    }
    return num;
}


/*
 write
 Write a character to a file. `libc' subroutines will use this system routine for output to all files, including stdout
 Returns -1 on error or number of bytes sent
 */
int _write(int file, char *ptr, int len) {
    int n;
    switch (file) {
    case STDOUT_FILENO: /*stdout*/
        for (n = 0; n < len; n++) {

            PutChar(*ptr++);
        }
        break;
    case STDERR_FILENO: /* stderr */
        for (n = 0; n < len; n++) {
        	PutChar(*ptr++);
        }
        break;
    default:
        errno = EBADF;
        return -1;
    }
    return len;
}
#endif

#include <string.h>

// ---------------------------------------------------------------------------------
// Terminal screen cursor control functions
// clear screen of WinH8
// ---------------------------------------------------------------------------------
void clrscr( void )
{
        printf( "\x1b[2J" ) ; /* extra ESC char required for 330 */
                                /* It seems that when 330 is set to */
                                /* 0 wait states, it is too FAST ??????*/
}

// ---------------------------------------------------------------------------------
// Put cursor into column-x and row-y of WinH8
// ---------------------------------------------------------------------------------
void gotoxy(int x, int y) {

    x=(x>150)?150:((x<1)?1:x);
    y=(y>150)?150:((y<1)?1:y);

   printf("\x1b[%d;%dh", y, x);

}


