/*	$NetBSD: tftp.c,v 1.4 1997/09/17 16:57:07 drochner Exp $	 */

/*
 * Copyright (c) 1996
 *	Matthias Drochner.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed for the NetBSD Project
 *	by Matthias Drochner.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * Simple TFTP implementation for libsa.
 * Assumes:
 *  - socket descriptor (int) at open_file->f_devdata
 *  - server host IP in global servip
 * Restrictions:
 *  - read only
 *  - lseek only with SEEK_SET or SEEK_CUR
 *  - no big time differences between transfers (<tftp timeout)
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/in_systm.h>
#include <arpa/tftp.h>

#include <string.h>

#include "stand.h"
#include "net.h"
#include "netif.h"

#include "tftp.h"

struct tftp_handle;

static int	tftp_open(const char *path, struct open_file *f);
static int	tftp_close(struct open_file *f);
static void	tftp_parse_oack(struct tftp_handle *h, char *buf, size_t len);
static int	tftp_read(struct open_file *f, void *buf, size_t size, size_t *resid);
static int	tftp_write(struct open_file *f, void *buf, size_t size, size_t *resid);
static off_t	tftp_seek(struct open_file *f, off_t offset, int where);
static int	tftp_set_blksize(struct tftp_handle *h, const char *str);
static int	tftp_stat(struct open_file *f, struct stat *sb);
static ssize_t sendrecv_tftp(struct tftp_handle *h, 
    ssize_t (*sproc)(struct iodesc *, void *, size_t),
    void *sbuf, size_t ssize,
    ssize_t (*rproc)(struct tftp_handle *h, void *, ssize_t, time_t, unsigned short *),
    void *rbuf, size_t rsize, unsigned short *rtype);

struct fs_ops tftp_fsops = {
	"tftp",
	tftp_open,
	tftp_close,
	tftp_read,
	tftp_write,
	tftp_seek,
	tftp_stat,
	null_readdir
};

extern struct in_addr servip;

static int      tftpport = 2000;
static int	is_open = 0;

/*
 * The legacy TFTP_BLKSIZE value was 512.
 * TFTP_REQUESTED_BLKSIZE of 1428 is (Ethernet MTU, less the TFTP, UDP and
 * IP header lengths).
 */
#define TFTP_REQUESTED_BLKSIZE 1428

/*
 * Choose a blksize big enough so we can test with Ethernet
 * Jumbo frames in the future.
 */ 
#define TFTP_MAX_BLKSIZE 9008

struct tftp_handle {
	struct iodesc  *iodesc;
	int             currblock;	/* contents of lastdata */
	int             islastblock;	/* flag */
	int             validsize;
	int             off;
	char           *path;	/* saved for re-requests */
	unsigned int	tftp_blksize;
	unsigned long	tftp_tsize; 
	struct {
		u_char header[HEADER_SIZE];
		struct tftphdr t;
		u_char space[TFTP_MAX_BLKSIZE];
	} __packed __aligned(4) lastdata;
};

static const int tftperrors[8] = {
	0,			/* ??? */
	ENOENT,
	EPERM,
	ENOSPC,
	EINVAL,			/* ??? */
	EINVAL,			/* ??? */
	EEXIST,
	EINVAL			/* ??? */
};

static ssize_t 
recvtftp(struct tftp_handle *h, void *pkt, ssize_t len, time_t tleft,
    unsigned short *rtype)
{
	struct iodesc *d = h->iodesc;
	struct tftphdr *t;

	errno = 0;

	len = readudp(d, pkt, len, tleft);

	if (len < 4)
		return (-1);

	t = (struct tftphdr *) pkt;
	*rtype = ntohs(t->th_opcode);
	switch (ntohs(t->th_opcode)) {
	case DATA: {
		int got;

		if (htons(t->th_block) != d->xid) {
			/*
			 * Expected block?
			 */
			return (-1);
		}
		if (d->xid == 1) {
			/*
			 * First data packet from new port.
			 */
			struct udphdr *uh;
			uh = (struct udphdr *) pkt - 1;
			d->destport = uh->uh_sport;
		} /* else check uh_sport has not changed??? */
		got = len - (t->th_data - (char *) t);
		return got;
	}
	case ERROR:
		if ((unsigned) ntohs(t->th_code) >= 8) {
			printf("illegal tftp error %d\n", ntohs(t->th_code));
			errno = EIO;
		} else {
#ifdef TFTP_DEBUG
			printf("tftp-error %d\n", ntohs(t->th_code));
#endif
			errno = tftperrors[ntohs(t->th_code)];
		}
		return (-1);
	case OACK: {
		struct udphdr *uh;
		int tftp_oack_len = len - sizeof(t->th_opcode); 
		tftp_parse_oack(h, t->th_u.tu_stuff, tftp_oack_len);
		/*
		 * Remember which port this OACK came from,
		 * because we need to send the ACK back to it.
		 */
		uh = (struct udphdr *) pkt - 1;
		d->destport = uh->uh_sport;
		return (0);
	}
	default:
#ifdef TFTP_DEBUG
		printf("tftp type %d not handled\n", ntohs(t->th_opcode));
#endif
		return (-1);
	}
}

/* send request, expect first block (or error) */
static int 
tftp_makereq(struct tftp_handle *h)
{
	struct {
		u_char header[HEADER_SIZE];
		struct tftphdr  t;
		u_char space[FNAME_SIZE + 6];
	} __packed __aligned(4) wbuf;
	char           *wtail;
	int             l;
	ssize_t         res;
	struct tftphdr *t;
	char *tftp_blksize = NULL;
	int blksize_l;
	unsigned short rtype = 0;

	/*
	 * Allow overriding default TFTP block size by setting
	 * a tftp.blksize environment variable.
	 */
	if ((tftp_blksize = getenv("tftp.blksize")) != NULL) {
		tftp_set_blksize(h, tftp_blksize);
	}

	wbuf.t.th_opcode = htons((u_short) RRQ);
	wtail = wbuf.t.th_stuff;
	l = strlen(h->path);
	if (l > FNAME_SIZE)
		return (ENAMETOOLONG);
	bcopy(h->path, wtail, l + 1);
	wtail += l + 1;
	bcopy("octet", wtail, 6);
	wtail += 6;
	bcopy("blksize", wtail, 8);
	wtail += 8;
	blksize_l = sprintf(wtail, "%d", h->tftp_blksize);
	wtail += blksize_l + 1;
	bcopy("tsize", wtail, 6);
	wtail += 6;
	bcopy("0", wtail, 2);
	wtail += 2;

	t = &h->lastdata.t;

	/* h->iodesc->myport = htons(--tftpport); */
	h->iodesc->myport = htons(tftpport + (getsecs() & 0x3ff));
	h->iodesc->destport = htons(IPPORT_TFTP);
	h->iodesc->xid = 1;	/* expected block */

	res = sendrecv_tftp(h, &sendudp, &wbuf.t, wtail - (char *) &wbuf.t,
		       &recvtftp, t, sizeof(*t) + h->tftp_blksize, &rtype);

	if (rtype == OACK) {
		wbuf.t.th_opcode = htons((u_short)ACK);
		wtail = (char *) &wbuf.t.th_block;
		wbuf.t.th_block = htons(0);
		wtail += 2;
		rtype = 0;
		res = sendrecv_tftp(h, &sendudp, &wbuf.t, wtail - (char *) &wbuf.t,
			       &recvtftp, t, sizeof(*t) + h->tftp_blksize, &rtype);
	}

	switch (rtype) {
		case DATA: {
			h->currblock = 1;
			h->validsize = res;
			h->islastblock = 0;
			if (res < h->tftp_blksize)
				h->islastblock = 1;	/* very short file */
			return (0);
		}
		case ERROR:
		default:
			return (errno);
	}

}

/* ack block, expect next */
static int 
tftp_getnextblock(struct tftp_handle *h)
{
	struct {
		u_char header[HEADER_SIZE];
		struct tftphdr t;
	} __packed __aligned(4) wbuf;
	char           *wtail;
	int             res;
	struct tftphdr *t;
	unsigned short rtype = 0;
	wbuf.t.th_opcode = htons((u_short) ACK);
	wtail = (char *) &wbuf.t.th_block;
	wbuf.t.th_block = htons((u_short) h->currblock);
	wtail += 2;

	t = &h->lastdata.t;

	h->iodesc->xid = h->currblock + 1;	/* expected block */

	res = sendrecv_tftp(h, &sendudp, &wbuf.t, wtail - (char *) &wbuf.t,
		       &recvtftp, t, sizeof(*t) + h->tftp_blksize, &rtype);

	if (res == -1)		/* 0 is OK! */
		return (errno);

	h->currblock++;
	h->validsize = res;
	if (res < h->tftp_blksize)
		h->islastblock = 1;	/* EOF */

	if (h->islastblock == 1) {
		/* Send an ACK for the last block */ 
		wbuf.t.th_block = htons((u_short) h->currblock);
		sendudp(h->iodesc, &wbuf.t, wtail - (char *)&wbuf.t);
	}

	return (0);
}

static int 
tftp_open(const char *path, struct open_file *f)
{
	struct tftp_handle *tftpfile;
	struct iodesc  *io;
	int             res;

#ifndef __i386__
	if (strcmp(f->f_dev->dv_name, "net") != 0)
		return (EINVAL);
#endif

	if (is_open)
		return (EBUSY);

	tftpfile = (struct tftp_handle *) malloc(sizeof(*tftpfile));
	if (!tftpfile)
		return (ENOMEM);

	memset(tftpfile, 0, sizeof(*tftpfile));
	tftpfile->tftp_blksize = TFTP_REQUESTED_BLKSIZE;
	tftpfile->iodesc = io = socktodesc(*(int *) (f->f_devdata));
	if (io == NULL)
		return (EINVAL);

	io->destip = servip;
	tftpfile->off = 0;
	tftpfile->path = strdup(path);
	if (tftpfile->path == NULL) {
	    free(tftpfile);
	    return(ENOMEM);
	}

	res = tftp_makereq(tftpfile);

	if (res) {
		free(tftpfile->path);
		free(tftpfile);
		return (res);
	}
	f->f_fsdata = (void *) tftpfile;
	is_open = 1;
	return (0);
}

static int 
tftp_read(struct open_file *f, void *addr, size_t size,
    size_t *resid /* out */)
{
	struct tftp_handle *tftpfile;
	static int      tc = 0;
	tftpfile = (struct tftp_handle *) f->f_fsdata;

	while (size > 0) {
		int needblock, count;

		if (!(tc++ % 16))
			twiddle();

		needblock = tftpfile->off / tftpfile->tftp_blksize + 1;

		if (tftpfile->currblock > needblock)	/* seek backwards */
			tftp_makereq(tftpfile);	/* no error check, it worked
						 * for open */

		while (tftpfile->currblock < needblock) {
			int res;

			res = tftp_getnextblock(tftpfile);
			if (res) {	/* no answer */
#ifdef TFTP_DEBUG
				printf("tftp: read error\n");
#endif
				return (res);
			}
			if (tftpfile->islastblock)
				break;
		}

		if (tftpfile->currblock == needblock) {
			int offinblock, inbuffer;

			offinblock = tftpfile->off % tftpfile->tftp_blksize;

			inbuffer = tftpfile->validsize - offinblock;
			if (inbuffer < 0) {
#ifdef TFTP_DEBUG
				printf("tftp: invalid offset %d\n",
				    tftpfile->off);
#endif
				return (EINVAL);
			}
			count = (size < inbuffer ? size : inbuffer);
			bcopy(tftpfile->lastdata.t.th_data + offinblock,
			    addr, count);

			addr = (char *)addr + count;
			tftpfile->off += count;
			size -= count;

			if ((tftpfile->islastblock) && (count == inbuffer))
				break;	/* EOF */
		} else {
#ifdef TFTP_DEBUG
			printf("tftp: block %d not found\n", needblock);
#endif
			return (EINVAL);
		}

	}

	if (resid)
		*resid = size;
	return (0);
}

static int 
tftp_close(struct open_file *f)
{
	struct tftp_handle *tftpfile;
	tftpfile = (struct tftp_handle *) f->f_fsdata;

	/* let it time out ... */

	if (tftpfile) {
		free(tftpfile->path);
		free(tftpfile);
	}
	is_open = 0;
	return (0);
}

static int 
tftp_write(struct open_file *f __unused, void *start __unused, size_t size __unused,
    size_t *resid __unused /* out */)
{
	return (EROFS);
}

static int 
tftp_stat(struct open_file *f, struct stat *sb)
{
	struct tftp_handle *tftpfile;
	tftpfile = (struct tftp_handle *) f->f_fsdata;

	sb->st_mode = 0444 | S_IFREG;
	sb->st_nlink = 1;
	sb->st_uid = 0;
	sb->st_gid = 0;
	sb->st_size = -1;
	return (0);
}

static off_t 
tftp_seek(struct open_file *f, off_t offset, int where)
{
	struct tftp_handle *tftpfile;
	tftpfile = (struct tftp_handle *) f->f_fsdata;

	switch (where) {
	case SEEK_SET:
		tftpfile->off = offset;
		break;
	case SEEK_CUR:
		tftpfile->off += offset;
		break;
	default:
		errno = EOFFSET;
		return (-1);
	}
	return (tftpfile->off);
}

static ssize_t
sendrecv_tftp(struct tftp_handle *h, 
    ssize_t (*sproc)(struct iodesc *, void *, size_t),
    void *sbuf, size_t ssize,
    ssize_t (*rproc)(struct tftp_handle *, void *, ssize_t, time_t, unsigned short *),
    void *rbuf, size_t rsize, unsigned short *rtype)
{
	struct iodesc *d = h->iodesc;
	ssize_t cc;
	time_t t, t1, tleft;

#ifdef TFTP_DEBUG
	if (debug)
		printf("sendrecv: called\n");
#endif

	tleft = MINTMO;
	t = t1 = getsecs();
	for (;;) {
		if ((getsecs() - t) > MAXTMO) {
			errno = ETIMEDOUT;
			return -1;
		}

		cc = (*sproc)(d, sbuf, ssize);
		if (cc != -1 && cc < ssize)
			panic("sendrecv: short write! (%zd < %zu)",
			    cc, ssize);

		if (cc == -1) {
			/* Error on transmit; wait before retrying */
			while ((getsecs() - t1) < tleft);
			continue;
		}

recvnext:
		/* Try to get a packet and process it. */
		cc = (*rproc)(h, rbuf, rsize, tleft, rtype);
		/* Return on data, EOF or real error. */
		if (cc != -1 || errno != 0)
			return (cc);
		if ((getsecs() - t1) < tleft) {
		    goto recvnext;
		}

		/* Timed out or didn't get the packet we're waiting for */
		tleft += MINTMO;
		if (tleft > (2 * MINTMO)) {
			tleft = (2 * MINTMO);
		}
		t1 = getsecs();
	}
}

static int
tftp_set_blksize(struct tftp_handle *h, const char *str)
{
        char *endptr;
	int new_blksize;
	int ret = 0;

	if (h == NULL || str == NULL)
		return (ret);

	new_blksize =
	    (unsigned int)strtol(str, &endptr, 0);

	/*
	 * Only accept blksize value if it is numeric.
	 * RFC2348 specifies that acceptable valuesare 8-65464
	 * 8-65464 .  Let's choose a limit less than MAXRSPACE
	*/
	if (*endptr == '\0' && new_blksize >= 8
	    && new_blksize <= TFTP_MAX_BLKSIZE) {
		h->tftp_blksize = new_blksize;
		ret = 1;
	}

	return (ret);
}

/*
 * In RFC2347, the TFTP Option Acknowledgement package (OACK)
 * is used to acknowledge a client's option negotiation request.
 * The format of an OACK packet is:
 *    +-------+---~~---+---+---~~---+---+---~~---+---+---~~---+---+
 *    |  opc  |  opt1  | 0 | value1 | 0 |  optN  | 0 | valueN | 0 |
 *    +-------+---~~---+---+---~~---+---+---~~---+---+---~~---+---+
 *
 *    opc
 *       The opcode field contains a 6, for Option Acknowledgment.
 *
 *    opt1
 *       The first option acknowledgment, copied from the original
 *       request.
 *
 *    value1
 *       The acknowledged value associated with the first option.  If
 *       and how this value may differ from the original request is
 *       detailed in the specification for the option.
 *
 *    optN, valueN
 *       The final option/value acknowledgment pair.
 */
static void 
tftp_parse_oack(struct tftp_handle *h, char *buf, size_t len)
{
	/* 
	 *  We parse the OACK strings into an array
	 *  of name-value pairs.
	 *  
	 */
	char *tftp_options[128] = { 0 };
	char *val = buf;
	int i = 0;
	int option_idx = 0;
	int blksize_is_set = 0;
	int tsize = 0;
	
 
	while ( option_idx < 128 && i < len ) {
		 if (buf[i] == '\0') {
		    if (&buf[i] > val) {
		       tftp_options[option_idx] = val;
		       val = &buf[i] + 1;
		       ++option_idx;
		    }
		 }
		 ++i;
	}

	/* 
	 * Parse individual TFTP options.
	 *    * "blksize" is specified in RFC2348.
	 *    * "tsize" is specified in RFC2349.
	 */ 
	for (i = 0; i < option_idx; i += 2) {
	    if (strcasecmp(tftp_options[i], "blksize") == 0) {
		if (i + 1 < option_idx) {
			blksize_is_set =
			    tftp_set_blksize(h, tftp_options[i + 1]);
		}
	    } else if (strcasecmp(tftp_options[i], "tsize") == 0) {
		if (i + 1 < option_idx) {
			tsize = strtol(tftp_options[i + 1], (char **)NULL, 10);
		}
	    }
	}

	if (!blksize_is_set) {
		/*
		 * If TFTP blksize was not set, try defaulting
		 * to the legacy TFTP blksize of 512
		 */
		h->tftp_blksize = 512;
	}

#ifdef TFTP_DEBUG
	printf("tftp_blksize: %u\n", h->tftp_blksize);
	printf("tftp_tsize: %lu\n", h->tftp_tsize);
#endif
}
