#ifndef lint
static	char *sccsid = "@(#)wwerror.c	3.1 83/09/13";
#endif

#include "ww.h"

char *
wwerror()
{
	extern errno;
	extern char *sys_errlist[];

	switch (wwerrno) {
	case WWE_NOERR:
		return "No error";
	case WWE_SYS:
		return sys_errlist[errno];
	case WWE_NOMEM:
		return "Out of memory";
	case WWE_TOOMANY:
		return "Too many windows";
	case WWE_NOPTY:
		return "Out of pseudo-terminals";
	case WWE_SIZE:
		return "Bad window size";
	case WWE_BADTERM:
		return "Unknown terminal type";
	case WWE_CANTDO:
		return "Can't run window on this terminal";
	default:
		return "Unknown error";
	}
}
