#ifndef lint
static char sccsid[] = "@(#)main.c	1.3 (Lucasfilm) %G%";
#endif

#include "systat.h"

struct nlist nlst[] = {
#define X_PROC          0
        { "_proc" },
#define X_NPROC         1
        { "_nproc" },
#define X_CCPU          2
        { "_ccpu" },
#define X_AVENRUN       3
        { "_avenrun" },
#define X_USRPTMAP      4
        { "_Usrptmap" },
#define X_USRPT         5
        { "_usrpt" },
#define X_NSWAP         6
        { "_nswap" },
#define X_SWAPMAP       7
        { "_swapmap" },
#define X_NSWAPMAP      8
        { "_nswapmap" },
#define X_DMMIN         9
        { "_dmmin" },
#define X_DMMAX         10
        { "_dmmax" },
#define X_NSWDEV        11
        { "_nswdev" },
#define	X_SWDEVT	12
	{ "_swdevt" },
#define	X_NTEXT		13
	{ "_ntext" },
#define	X_TEXT		14
	{ "_text" },
#define	X_DMTEXT	15
	{ "_dmtext" },
#define	X_MBSTAT	16
	{ "_mbstat" },
        { "" }
};

int     kmem = -1;
int     mem = -1;
int     swap = -1;
int	naptime = 5;

int     die();
int     display();
int     suspend();

int     showpigs(), openpigs(), fetchpigs(), labelpigs(), initpigs();
int     showswap(), fetchswap(), labelswap(), initswap();
int	showmbufs(), fetchmbufs(), labelmbufs(), initmbufs();
#ifdef notdef
int     showuser(), openuser(), fetchuser(), labeluser(), inituser();
int     shownet(), opennet(), fetchnet(), labelnet(), initnet();
#endif

struct  cmdtab {
        char    *c_name;
        int     (*c_refresh)();
        int     (*c_open)();
        int     (*c_fetch)();
        int     (*c_label)();
	int	(*c_init)();
	char	c_flags;
} cmdtab[] = {
        { "pigs",       showpigs,       openpigs,       fetchpigs,
          labelpigs,	initpigs },
        { "swap",       showswap,       openpigs,       fetchswap,
          labelswap,	initswap },
        { "mbufs",	showmbufs,	openpigs,       fetchmbufs,
          labelmbufs,	initmbufs },
#ifdef notdef
        { "user",       showuser,       openuser,       fetchuser,
          labeluser,	inituser },
        { "net",        shownet,        opennet,        fetchnet,
          labelnet,	initnet },
#endif
        { "" }
};
struct  cmdtab *curcmd = &cmdtab[0];

main(argc, argv)
        int argc;
        char **argv;
{
        char ch, line[80];

	argc--, argv++;
	while (argc > 0) {
		if (argv[0][0] == '-') {
			struct cmdtab *p;

			for (p = cmdtab; *p->c_name; p++)
				if (strcmp(p->c_name, &argv[0][1]) == 0)
					break;
			if (*p->c_name == 0) {
				fprintf(stderr, "%s: unknown request\n",
				    &argv[0][1]);
				exit(1);
			}
			curcmd = p;
		} else {
			naptime = atoi(argv[1]);
			if (naptime < 5)
				naptime = 5;
		}
		argc--, argv++;
	}
        nlist("/vmunix", nlst);
        (*curcmd->c_open)();
        signal(SIGINT, die);
        signal(SIGQUIT, die);
        signal(SIGTERM, die);

        /* Initialize curses. */
        initscr();
        wnd = newwin(20, 70, 3, 5);

#ifdef notdef
        gethostname(hostname, sizeof (hostname));
#endif
        lseek(kmem, nlst[X_CCPU].n_value, 0);
        read(kmem, &ccpu, sizeof (ccpu));
        lccpu = log(ccpu);
        labels();

        known[0].k_uid = -1;
        strcpy(known[0].k_name, "<idle>");
        numknown = 1;
        dellave = 0.0;

        signal(SIGALRM, display);
        signal(SIGTSTP, suspend);
        display();
        noecho();
        crmode();
        for (;;) {
                col = 0;
                move(22, 0);
                do {
                        refresh();
                        ch = getch() & 0177;
                        if (ch == 0177 && ferror(stdin)) {
                                clearerr(stdin);
                                continue;
                        }
                        if (ch >= 'A' && ch <= 'Z')
                                ch += 'a' - 'A';
                        if (col == 0) {
#define	mask(s)	(1 << ((s) - 1))
                                if (ch == CTRL(l)) {
					int oldmask = sigblock(mask(SIGALRM));

					wrefresh(curscr);
					sigsetmask(oldmask);
                                        continue;
                                }
                                if (ch != ':')
                                        continue;
                                move(22, 0);
                                clrtoeol();
                        }
                        if (ch == _tty.sg_erase && col > 0) {
                                if (col == 1 && line[0] == ':')
                                        continue;
                                col--;
                                goto doerase;
                        }
                        if (ch == CTRL(w) && col > 0) {
                                while (--col >= 0 && isspace(line[col]))
                                        ;
                                col++;
                                while (--col >= 0 && !isspace(line[col]))
                                        if (col == 0 && line[0] == ':')
                                                break;
                                col++;
                                goto doerase;
                        }
                        if (ch == _tty.sg_kill && col > 0) {
                                col = 0;
                                if (line[0] == ':')
                                        col++;
                doerase:
                                move(22, col);
                                clrtoeol();
                                continue;
                        }
                        if (isprint(ch)) {
                                line[col] = ch;
                                mvaddch(22, col, ch);
                                col++;
                        }
                } while (col == 0 || (ch != '\r' && ch != '\n'));
                line[col] = '\0';
                command(line + 1);
        }
}

command(cmd)
        char *cmd;
{
        register char *cp;
        register struct cmdtab *p;
        char *arg;

        for (cp = cmd; *cp && !isspace(*cp); cp++)
                ;
        if (*cp)
                *cp++ = '\0';
        if (strcmp(cmd, "quit") == 0)
                die();
        if (strcmp(cmd, "status") == 0 || strcmp(cmd, "help") == 0) {
                status();
                return;
        }
	if (strcmp(cmd, "load") == 0) {
		lseek(kmem, nlst[X_AVENRUN].n_value, L_SET);
		read(kmem, &lave, sizeof (lave));
		mvprintw(22, 0, "%4.1f", lave);
		clrtoeol();
		return;
	}
        for (p = cmdtab; *p->c_name; p++)
                if (strcmp(cmd, p->c_name) == 0)
                        break;
        if (*p->c_name) {
                if (curcmd == p)
                        return;
                alarm(0);
                curcmd = p;
		clear(); wclear(wnd);
		labels();
                display();
                status();
                return;
        }
        if (strcmp(cmd, "stop") == 0) {
                alarm(0);
                mvaddstr(22, 0, "Refresh disabled.");
                clrtoeol();
                return;
        }
        /* commands with arguments */
        for (; *cp && isspace(*cp); cp++)
                ;
        if (strcmp(cmd, "start") == 0) {
                int x;

                if (*cp == '\0')
                        x = naptime;
                else
                        x = atoi(cp);
                if (x <= 0) {
                        mvprintw(22, 0, "%d: bad interval.", x);
                        clrtoeol();
                        return;
                }
                alarm(0);
                naptime = x;
                display();
                status();
                return;
        }
	if (*cmd) {
		mvprintw(22, 0, "%s: Unknown command.", cmd);
		clrtoeol();
	}
}

status()
{

        mvprintw(22, 0, "Showing %s, refresh every %d seconds.",
          curcmd->c_name, naptime);
        clrtoeol();
}

suspend()
{
        int oldmask;

	alarm(0);
        move(22, 0);
        refresh();
        echo();
        nocrmode();
        signal(SIGTSTP, SIG_DFL);
        oldmask = sigsetmask(0);
        kill(getpid(), SIGTSTP);
        sigsetmask(oldmask);
        signal(SIGTSTP, suspend);
        crmode();
        noecho();
        move(22, col);
        wrefresh(curscr);
	alarm(naptime);
}

labels()
{

        mvaddstr(2, 20,
                "/0   /1   /2   /3   /4   /5   /6   /7   /8   /9   /10");
        mvwaddstr(wnd, 0, 0, "Load Average");
        (*curcmd->c_label)();
#ifdef notdef
        mvprintw(21, 25, "CPU usage on %s", hostname);
#endif
        refresh();
}

display()
{
        register int i, j;

        /* Get the load average over the last minute. */
        lseek(kmem, nlst[X_AVENRUN].n_value, L_SET);
        read(kmem, &lave, sizeof (lave));
	if (curcmd->c_flags == 0) {
		(*curcmd->c_init)();
		curcmd->c_flags = 1;
	}
        (*curcmd->c_fetch)();
        j = 5.0*lave + 0.5;
        dellave -= lave;
        if (dellave >= 0.0)
                c = '<';
        else {
                c = '>';
                dellave = -dellave;
        }
        if (dellave < 0.1)
                c = '|';
        dellave = lave;
        wmove(wnd, 0, 15);
        wclrtoeol(wnd);
        for (i = (j > 50)? 50 : j; i > 0; i--)
                waddch(wnd, c);
        if (j > 50)
                wprintw(wnd, " %4.1f", lave);
        (*curcmd->c_refresh)();
        wrefresh(wnd);
        move(22, col);
        refresh();
        alarm(naptime);
}

die()
{

        endwin();
        exit(0);
}

error(fmt, a1, a2, a3)
{

	mvprintw(22, 0, fmt, a1, a2, a3);
	clrtoeol();
	refresh();
}
