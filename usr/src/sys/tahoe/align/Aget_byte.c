/*	Aget_byte.c	1.2	90/12/04	*/

#include	"align.h"
int get_byte (infop, address)
process_info	*infop;
char		*address;
/*
/*	Fetch the byte at the given 'address' from memory.
/*	Caveat: It's quite difficult to find a pte reference
/*		fault.  So I took the easy way out and just signal
/*		an illegal access.
/*	
/**************************************************/
{
	register long code;

	code = readable(infop, (long)address, 1);
	if ( code == TRUE ) {
		return(*address);
	} else exception (infop, ILL_ACCESS, address, code);
}
