/*
 * Copyright (c) 1995
 *	Bill Paul <wpaul@ctr.columbia.edu>.  All rights reserved.
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
 *	This product includes software developed by Bill Paul.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Bill Paul AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Bill Paul OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$Id: yp_dblookup.c,v 1.4 1996/07/07 19:04:33 wpaul Exp $
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <db.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <errno.h>
#include <paths.h>
#include <rpcsvc/yp.h>
#include "yp_extern.h"

#ifndef lint
static const char rcsid[] = "$Id: yp_dblookup.c,v 1.4 1996/07/07 19:04:33 wpaul Exp $";
#endif

int ypdb_debug = 0;
int yp_errno = YP_TRUE;

#define PERM_SECURE (S_IRUSR|S_IWUSR)
HASHINFO openinfo = {
	4096,		/* bsize */
	32,		/* ffactor */
	256,		/* nelem */
	2048 * 512, 	/* cachesize */
	NULL,		/* hash */
	0,		/* lorder */
};

#ifdef DB_CACHE
#include <sys/queue.h>

#ifndef MAXDBS
#define MAXDBS 20
#endif

static int numdbs = 0;

struct dbent {
	DB *dbp;
	char *name;
	char *key;
	int size;
};

static CIRCLEQ_HEAD(circlehead, circleq_entry) qhead;

struct circleq_entry {
	struct dbent *dbptr;
	CIRCLEQ_ENTRY(circleq_entry) links;
};

/*
 * Initialize the circular queue.
 */
void yp_init_dbs()
{
	CIRCLEQ_INIT(&qhead);
	return;
}

/*
 * Dynamically allocate an entry for the circular queue.
 * Return a NULL pointer on failure.
 */
static struct circleq_entry *yp_malloc_qent()
{
	register struct circleq_entry *q;

	q = (struct circleq_entry *)malloc(sizeof(struct circleq_entry));
	if (q == NULL) {
		yp_error("failed to malloc() circleq entry: %s",
							strerror(errno));
		return(NULL);
	}
	bzero((char *)q, sizeof(struct circleq_entry));
	q->dbptr = (struct dbent *)malloc(sizeof(struct dbent));
	if (q->dbptr == NULL) {
		yp_error("failed to malloc() circleq entry: %s",
							strerror(errno));
		free(q);
		return(NULL);
	}
	bzero((char *)q->dbptr, sizeof(struct dbent));

	return(q);
}

/*
 * Free a previously allocated circular queue
 * entry.
 */
static void yp_free_qent(q)
	struct circleq_entry *q;
{
	/*
	 * First, close the database. In theory, this is also
	 * supposed to free the resources allocated by the DB
	 * package, including the memory pointed to by q->dbptr->key.
	 * This means we don't have to free q->dbptr->key here.
	 */
	if (q->dbptr->dbp) {
		(void)(q->dbptr->dbp->close)(q->dbptr->dbp);
		q->dbptr->dbp = NULL;
	}
	/*
	 * Then free the database name, which was strdup()'ed.
	 */
	free(q->dbptr->name);

	/*
	 * Free the rest of the dbent struct.
	 */
	free(q->dbptr);
	q->dbptr = NULL;

	/*
	 * Free the circleq struct.
	 */
	free(q);
	q = NULL;

	return;
}

/*
 * Zorch a single entry in the dbent queue and release
 * all its resources. (This always removes the last entry
 * in the queue.)
 */
static void yp_flush()
{
	register struct circleq_entry *qptr;

	qptr = qhead.cqh_last;
	CIRCLEQ_REMOVE(&qhead, qptr, links);
	yp_free_qent(qptr);
	numdbs--;

	return;
}

/*
 * Close all databases, erase all database names and empty the queue.
 */
void yp_flush_all()
{
	register struct circleq_entry *qptr;

	while(qhead.cqh_first != (void *)&qhead) {
		qptr = qhead.cqh_first; /* save this */
		CIRCLEQ_REMOVE(&qhead, qhead.cqh_first, links);
		yp_free_qent(qptr); 
	}
	numdbs = 0;

	return;
}


/*
 * Add a DB handle and database name to the cache. We only maintain
 * fixed number of entries in the cache, so if we're asked to store
 * a new entry when all our slots are already filled, we have to kick
 * out the entry in the last slot to make room.
 */
static int yp_cache_db(dbp, name, size)
	DB *dbp;
	char *name;
	int size;
{
	register struct circleq_entry *qptr;

	if (numdbs == MAXDBS) {
		if (ypdb_debug)
			yp_error("queue overflow -- releasing last slot");
		yp_flush();
	}

	/*
	 * Allocate a new queue entry.
	 */

	if ((qptr = yp_malloc_qent()) == NULL) {
		yp_error("failed to allocate a new cache entry");
		return(1);
	}

	qptr->dbptr->dbp = dbp;
	qptr->dbptr->name = strdup(name);
	qptr->dbptr->size = size;
	qptr->dbptr->key = NULL;

	CIRCLEQ_INSERT_HEAD(&qhead, qptr, links);
	numdbs++;

	return(0);
}

/*
 * Search the list for a database matching 'name.' If we find it,
 * move it to the head of the list and return its DB handle. If
 * not, just fail: yp_open_db_cache() will subsequently try to open
 * the database itself and call yp_cache_db() to add it to the
 * list.
 *
 * The search works like this:
 *
 * - The caller specifies the name of a database to locate. We try to
 *   find an entry in our queue with a matching name.
 *
 * - If the caller doesn't specify a key or size, we assume that the
 *   first entry that we encounter with a matching name is returned.
 *   This will result in matches regardless of the key/size values
 *   stored in the queue entry.
 *
 * - If the caller also specifies a key and length, we check to see
 *   if the key and length saved in the queue entry also matches.
 *   This lets us return a DB handle that's already positioned at the
 *   correct location within a database.
 *
 * - Once we have a match, it gets migrated to the top of the queue
 *   so that it will be easier to find if another request for
 *   the same database comes in later.
 */
static DB *yp_find_db(name, key, size)
	char *name;
	char *key;
	int size;
{
	register struct circleq_entry *qptr;

	for (qptr = qhead.cqh_first; qptr != (void *)&qhead;
						qptr = qptr->links.cqe_next) {
		if (!strcmp(qptr->dbptr->name, name)) {
			if (size) {
				if (size != qptr->dbptr->size ||
				   strncmp(qptr->dbptr->key, key, size))
					continue;
			} else {
				if (qptr->dbptr->size)
					continue;
			}
			if (qptr != qhead.cqh_first) {
				CIRCLEQ_REMOVE(&qhead, qptr, links);
				CIRCLEQ_INSERT_HEAD(&qhead, qptr, links);
			}
			return(qptr->dbptr->dbp);
		}
	}

	return(NULL);
}

/*
 * Open a DB database and cache the handle for later use. We first
 * check the cache to see if the required database is already open.
 * If so, we fetch the handle from the cache. If not, we try to open
 * the database and save the handle in the cache for later use.
 */
DB *yp_open_db_cache(domain, map, key, size)
	const char *domain;
	const char *map;
	const char *key;
	const int size;
{
	DB *dbp = NULL;
	char buf[MAXPATHLEN + 2];
/*
	snprintf(buf, sizeof(buf), "%s/%s", domain, map);
*/
	yp_errno = YP_TRUE;

	strcpy(buf, domain);
	strcat(buf, "/");
	strcat(buf, map);

	if ((dbp = yp_find_db((char *)&buf, key, size)) != NULL) {
		return(dbp);
	} else {
		if ((dbp = yp_open_db(domain, map)) != NULL) {
			if (yp_cache_db(dbp, (char *)&buf, size)) {
				(void)(dbp->close)(dbp);
				yp_errno = YP_YPERR;
				return(NULL);
			}
		}
	}

	return (dbp);
}
#endif

/*
 * Open a DB database.
 */
DB *yp_open_db(domain, map)
	const char *domain;
	const char *map;
{
	DB *dbp = NULL;
	char buf[MAXPATHLEN + 2];

	yp_errno = YP_TRUE;

	if (map[0] == '.' || strchr(map, '/')) {
		yp_errno = YP_BADARGS;
		return (NULL);
	}

#ifdef DB_CACHE
	if (yp_validdomain(domain)) {
		yp_errno = YP_NODOM;
		return(NULL);
	}
#endif
	snprintf(buf, sizeof(buf), "%s/%s/%s", yp_dir, domain, map);

#ifdef DB_CACHE
again:
#endif
	dbp = dbopen(buf,O_RDONLY, PERM_SECURE, DB_HASH, NULL);

	if (dbp == NULL) {
		switch(errno) {
#ifdef DB_CACHE
		case ENFILE:
			/*
			 * We ran out of file descriptors. Nuke an
			 * open one and try again.
			 */
			yp_error("ran out of file descriptors");
			yp_flush();
			goto again;
			break;
#endif
		case ENOENT:
			yp_errno = YP_NOMAP;
			break;
		case EFTYPE:
			yp_errno = YP_BADDB;
			break;
		default:
			yp_errno = YP_YPERR;
			break;
		}
	}

	return (dbp);
}

/*
 * Database access routines.
 *
 * - yp_get_record(): retrieve an arbitrary key/data pair given one key
 *                 to match against.
 *
 * - yp_first_record(): retrieve first key/data base in a database.
 * 
 * - yp_next_record(): retrieve key/data pair that sequentially follows
 *                   the supplied key value in the database.
 */

int yp_get_record(domain,map,key,data,allow)
	const char *domain;
	const char *map;
	const DBT *key;
	DBT *data;
	int allow;
{
	DB *dbp;
	int rval = 0;
#ifndef DB_CACHE
	static unsigned char buf[YPMAXRECORD];
#endif

	if (ypdb_debug)
		yp_error("Looking up key [%.*s] in map [%s]",
			  key->size, key->data, map);

	/*
	 * Avoid passing back magic "YP_*" entries unless
	 * the caller specifically requested them by setting
	 * the 'allow' flag.
	 */
	if (!allow && !strncmp(key->data, "YP_", 3))
		return(YP_NOKEY);

#ifdef DB_CACHE
	if ((dbp = yp_open_db_cache(domain, map, NULL, 0)) == NULL) {
#else
	if ((dbp = yp_open_db(domain, map)) == NULL) {
#endif
		return(yp_errno);
	}

	if ((rval = (dbp->get)(dbp, key, data, 0)) != 0) {
#ifdef DB_CACHE
		qhead.cqh_first->dbptr->size = 0;
#else
		(void)(dbp->close)(dbp);
#endif
		if (rval == 1)
			return(YP_NOKEY);
		else
			return(YP_BADDB);
	}

	if (ypdb_debug)
		yp_error("Result of lookup: key: [%.*s] data: [%.*s]",
			 key->size, key->data, data->size, data->data);

#ifdef DB_CACHE
	if (qhead.cqh_first->dbptr->size) {
		qhead.cqh_first->dbptr->key = key->data;
		qhead.cqh_first->dbptr->size = key->size;
	}
#else
	bcopy((char *)data->data, (char *)&buf, data->size);
	data->data = (void *)&buf;
	(void)(dbp->close)(dbp);
#endif

	return(YP_TRUE);
}

int yp_first_record(dbp,key,data,allow)
	const DB *dbp;
	DBT *key;
	DBT *data;
	int allow;
{
	int rval;
#ifndef DB_CACHE
	static unsigned char buf[YPMAXRECORD];
#endif

	if (ypdb_debug)
		yp_error("Retrieving first key in map.");

	if ((rval = (dbp->seq)(dbp,key,data,R_FIRST)) != 0) {
#ifdef DB_CACHE
		qhead.cqh_first->dbptr->size = 0;
#endif
		if (rval == 1)
			return(YP_NOKEY);
		else 
			return(YP_BADDB);
	}

	/* Avoid passing back magic "YP_*" records. */
	while (!strncmp(key->data, "YP_", 3) && !allow) {
		if ((rval = (dbp->seq)(dbp,key,data,R_NEXT)) != 0) {
#ifdef DB_CACHE
			qhead.cqh_first->dbptr->size = 0;
#endif
			if (rval == 1)
				return(YP_NOKEY);
			else
				return(YP_BADDB);
		}
	}

	if (ypdb_debug)
		yp_error("Result of lookup: key: [%.*s] data: [%.*s]",
			 key->size, key->data, data->size, data->data);

#ifdef DB_CACHE
	if (qhead.cqh_first->dbptr->size) {
		qhead.cqh_first->dbptr->key = key->data;
		qhead.cqh_first->dbptr->size = key->size;
	}
#else
	bcopy((char *)data->data, (char *)&buf, data->size);
	data->data = (void *)&buf;
#endif

	return(YP_TRUE);
}

int yp_next_record(dbp,key,data,all,allow)
	const DB *dbp;
	DBT *key;
	DBT *data;
	int all;
	int allow;
{
	static DBT lkey = { NULL, 0 };
	static DBT ldata = { NULL, 0 };
	int rval;
#ifndef DB_CACHE
	static unsigned char keybuf[YPMAXRECORD];
	static unsigned char datbuf[YPMAXRECORD];
#endif

	if (key == NULL || key->data == NULL) {
		rval = yp_first_record(dbp,key,data,allow);
		if (rval == YP_NOKEY)
			return(YP_NOMORE);
		else
			return(rval);
	}

	if (ypdb_debug)
		yp_error("Retreiving next key, previous was: [%.*s]",
			  key->size, key->data);

	if (!all) {
#ifdef DB_CACHE
		if (qhead.cqh_first->dbptr->key == NULL) {
#endif
			(dbp->seq)(dbp,&lkey,&ldata,R_FIRST);
			while(strncmp((char *)key->data,lkey.data,
				(int)key->size) || key->size != lkey.size)
				if ((dbp->seq)(dbp,&lkey,&ldata,R_NEXT)) {
#ifdef DB_CACHE
					qhead.cqh_first->dbptr->size = 0;
#endif
					return(YP_NOKEY);
				}

#ifdef DB_CACHE					
		}
#endif
	}

	if ((dbp->seq)(dbp,key,data,R_NEXT)) {
#ifdef DB_CACHE
		qhead.cqh_first->dbptr->size = 0;
#endif
		return(YP_NOMORE);
	}

	/* Avoid passing back magic "YP_*" records. */
	while (!strncmp(key->data, "YP_", 3) && !allow)
		if ((dbp->seq)(dbp,key,data,R_NEXT)) {
#ifdef DB_CACHE
		qhead.cqh_first->dbptr->size = 0;
#endif
			return(YP_NOMORE);
		}

	if (ypdb_debug)
		yp_error("Result of lookup: key: [%.*s] data: [%.*s]",
			 key->size, key->data, data->size, data->data);

#ifdef DB_CACHE
	if (qhead.cqh_first->dbptr->size) {
		qhead.cqh_first->dbptr->key = key->data;
		qhead.cqh_first->dbptr->size = key->size;
	}
#else
	bcopy((char *)key->data, (char *)&keybuf, key->size);
	lkey.data = (void *)&keybuf;
	lkey.size = key->size;
	bcopy((char *)data->data, (char *)&datbuf, data->size);
	data->data = (void *)&datbuf;
#endif

	return(YP_TRUE);
}
