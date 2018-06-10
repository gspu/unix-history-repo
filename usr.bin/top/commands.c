/*
 *  Top users/processes display for Unix
 *
 *  This program may be freely redistributed,
 *  but this entire comment MUST remain intact.
 *
 *  Copyright (c) 1984, 1989, William LeFebvre, Rice University
 *  Copyright (c) 1989, 1990, 1992, William LeFebvre, Northwestern University
 *
 * $FreeBSD$
 */

/*
 *  This file contains the routines that implement some of the interactive
 *  mode commands.  Note that some of the commands are implemented in-line
 *  in "main".  This is necessary because they change the global state of
 *  "top" (i.e.:  changing the number of processes to display).
 */

#include <sys/resource.h>
#include <sys/signal.h>

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "commands.h"
#include "top.h"
#include "machine.h"

static int err_compar(const void *p1, const void *p2);

struct errs		/* structure for a system-call error */
{
    int  errnum;	/* value of errno (that is, the actual error) */
    char *arg;		/* argument that caused the error */
};

static char *err_string(void);
static int str_adderr(char *str, int len, int err);
static int str_addarg(char *str, int len, char *arg, bool first);

/*
 *  show_help() - display the help screen; invoked in response to
 *		either 'h' or '?'.
 */

static const struct command all_commands[] =
{
	{'C', "toggle the displaying of weighted CPU percentage", false },
	{'d', "change number of displays to show", false},
	{'e', "list errors generated by last \"kill\" or \"renice\" command", false},
	{'H', "toggle the displaying of threads", false},
	{'h', "show this help text", true},
	{'?', "show this help text", true},
	{'i', "toggle the displaying of idle processes", false},
	{'I', "toggle the displaying of idle processes", false},
	{'j', "toggle the displaying of jail ID", false},
	{'J', "display processes for only one jail (+ selects all jails)", false},
	{'k', "kill processes; send a signal to a list of processes", false},
	{'q', "quit" , true},
	{'m', "toggle the display between 'cpu' and 'io' modes", false},
	{'n', "change number of processes to display", false},
	{'#', "change number of processes to display", false},
	{'o', "specify the sort order", false},
	{'p', "display one process (+ selects all processes)", false},
	{'P', "toggle the displaying of per-CPU statistics", false},
	{'r', "renice a process", false},
	{'s', "change number of seconds to delay between updates", false},
	{'S', "toggle the displaying of system processes", false},
	{'a', "toggle the displaying of process titles", false},
	{'T', "toggle the displaying of thread IDs", false},
	{'t', "toggle the display of this process", false},
	{'u', "display processes for only one user (+ selects all users)", false},
	{'w', "toggle the display of swap use for each process", false},
	{'z', "toggle the displaying of the system idle process", false },
	{0, NULL, true}
};
/* XXX: eventually remove command_chars, but assert they are the same for now */

void
show_help(void)
{
	const struct command *curcmd;

    printf("Top version FreeBSD, %s\n", copyright);
	curcmd = all_commands;
	while (curcmd->c != 0) {
		if (overstrike && !curcmd->available_to_dumb) {
			++curcmd;
			continue;
		}
		printf("%c\t- %s\n", curcmd->c, curcmd->desc);
		++curcmd;
	}
    if (overstrike)
    {
		fputs("\
				Other commands are also available, but this terminal is not\n\
				sophisticated enough to handle those commands gracefully.\n", stdout);
    }
}

/*
 *  Utility routines that help with some of the commands.
 */

static char *
next_field(char *str)
{
    if ((str = strchr(str, ' ')) == NULL)
    {
	return(NULL);
    }
    *str = '\0';
    while (*++str == ' ') /* loop */;

    /* if there is nothing left of the string, return NULL */
    /* This fix is dedicated to Greg Earle */
    return(*str == '\0' ? NULL : str);
}

static int
scanint(char *str, int *intp)
{
    int val = 0;
    char ch;

    /* if there is nothing left of the string, flag it as an error */
    /* This fix is dedicated to Greg Earle */
    if (*str == '\0')
    {
	return(-1);
    }

    while ((ch = *str++) != '\0')
    {
	if (isdigit(ch))
	{
	    val = val * 10 + (ch - '0');
	}
	else if (isspace(ch))
	{
	    break;
	}
	else
	{
	    return(-1);
	}
    }
    *intp = val;
    return(0);
}

/*
 *  Some of the commands make system calls that could generate errors.
 *  These errors are collected up in an array of structures for later
 *  contemplation and display.  Such routines return a string containing an
 *  error message, or NULL if no errors occurred.  The next few routines are
 *  for manipulating and displaying these errors.  We need an upper limit on
 *  the number of errors, so we arbitrarily choose 20.
 */

#define ERRMAX 20

static struct errs errs[ERRMAX];
static int errcnt;
static char err_toomany[] = " too many errors occurred";
static char err_listem[] = 
	" Many errors occurred.  Press `e' to display the list of errors.";

/* These macros get used to reset and log the errors */
#define ERR_RESET   errcnt = 0
#define ERROR(p, e) if (errcnt >= ERRMAX) \
		    { \
			return(err_toomany); \
		    } \
		    else \
		    { \
			errs[errcnt].arg = (p); \
			errs[errcnt++].errnum = (e); \
		    }

/*
 *  err_string() - return an appropriate error string.  This is what the
 *	command will return for displaying.  If no errors were logged, then
 *	return NULL.  The maximum length of the error string is defined by
 *	"STRMAX".
 */

#define STRMAX 80

char *err_string(void)
{
    struct errs *errp;
    int cnt = 0;
    bool first = true;
    int currerr = -1;
    int stringlen;		/* characters still available in "string" */
    static char string[STRMAX];

    /* if there are no errors, return NULL */
    if (errcnt == 0)
    {
	return(NULL);
    }

    /* sort the errors */
    qsort((char *)errs, errcnt, sizeof(struct errs), err_compar);

    /* need a space at the front of the error string */
    string[0] = ' ';
    string[1] = '\0';
    stringlen = STRMAX - 2;

    /* loop thru the sorted list, building an error string */
    while (cnt < errcnt)
    {
	errp = &(errs[cnt++]);
	if (errp->errnum != currerr)
	{
	    if (currerr != -1)
	    {
		if ((stringlen = str_adderr(string, stringlen, currerr)) < 2)
		{
		    return(err_listem);
		}
		strcat(string, "; ");	  /* we know there's more */
	    }
	    currerr = errp->errnum;
	    first = true;
	}
	if ((stringlen = str_addarg(string, stringlen, errp->arg, first)) ==0)
	{
	    return(err_listem);
	}
	first = false;
    }

    /* add final message */
    stringlen = str_adderr(string, stringlen, currerr);

    /* return the error string */
    return(stringlen == 0 ? err_listem : string);
}

/*
 *  str_adderr(str, len, err) - add an explanation of error "err" to
 *	the string "str".
 */

static int
str_adderr(char *str, int len, int err)
{
    const char *msg;
    int msglen;

    msg = err == 0 ? "Not a number" : strerror(err);
    msglen = strlen(msg) + 2;
    if (len <= msglen)
    {
	return(0);
    }
    strcat(str, ": ");
    strcat(str, msg);
    return(len - msglen);
}

/*
 *  str_addarg(str, len, arg, first) - add the string argument "arg" to
 *	the string "str".  This is the first in the group when "first"
 *	is set (indicating that a comma should NOT be added to the front).
 */

static int
str_addarg(char str[], int len, char arg[], bool first)
{
    int arglen;

    arglen = strlen(arg);
    if (!first)
    {
	arglen += 2;
    }
    if (len <= arglen)
    {
	return(0);
    }
    if (!first)
    {
	strcat(str, ", ");
    }
    strcat(str, arg);
    return(len - arglen);
}

/*
 *  err_compar(p1, p2) - comparison routine used by "qsort"
 *	for sorting errors.
 */

static int
err_compar(const void *p1, const void *p2)
{
    int result;
    const struct errs * const g1 = (const struct errs * const)p1;
    const struct errs * const g2 = (const struct errs * const)p2;



    if ((result = g1->errnum - g2->errnum) == 0)
    {
	return(strcmp(g1->arg, g2->arg));
    }
    return(result);
}

/*
 *  error_count() - return the number of errors currently logged.
 */

int
error_count(void)
{
    return(errcnt);
}

/*
 *  show_errors() - display on stdout the current log of errors.
 */

void
show_errors(void)
{
    int cnt = 0;
    struct errs *errp = errs;

    printf("%d error%s:\n\n", errcnt, errcnt == 1 ? "" : "s");
    while (cnt++ < errcnt)
    {
	printf("%5s: %s\n", errp->arg,
	    errp->errnum == 0 ? "Not a number" : strerror(errp->errnum));
	errp++;
    }
}

static const char no_proc_specified[] = " no processes specified";
static const char invalid_signal_number[] = " invalid_signal_number";
static const char bad_signal_name[] = " bad signal name";
static const char bad_pri_value[] = " bad priority value";

static int
signame_to_signum(const char * sig)
{
        int n;

        if (strncasecmp(sig, "SIG", 3) == 0)
                sig += 3;
        for (n = 1; n < sys_nsig; n++) {
            if (!strcasecmp(sys_signame[n], sig))
                return (n);
        }
        return (-1);
}

/*
 *  kill_procs(str) - send signals to processes, much like the "kill"
 *		command does; invoked in response to 'k'.
 */

const char *
kill_procs(char *str)
{
    char *nptr;
    int signum = SIGTERM;	/* default */
    int procnum;
    int uid;

    /* reset error array */
    ERR_RESET;

    /* remember our uid */
    uid = getuid();

    /* skip over leading white space */
    while (isspace(*str)) str++;

    if (str[0] == '-')
    {
	/* explicit signal specified */
	if ((nptr = next_field(str)) == NULL)
	{
	    return(no_proc_specified);
	}

	if (isdigit(str[1]))
	{
	    scanint(str + 1, &signum);
	    if (signum <= 0 || signum >= NSIG)
	    {
		return(invalid_signal_number);
	    }
	}
	else 
	{
		signum = signame_to_signum(str + 1);

	    /* was it ever found */
	    if (signum == -1 )
	    {
			return(bad_signal_name);
	    }
	}
	/* put the new pointer in place */
	str = nptr;
    }

    /* loop thru the string, killing processes */
    do
    {
	if (scanint(str, &procnum) == -1)
	{
	    ERROR(str, 0);
	}
	else
	{
	    /* check process owner if we're not root */
	    if (uid && (uid != proc_owner(procnum)))
	    {
		ERROR(str, EACCES);
	    }
	    /* go in for the kill */
	    else if (kill(procnum, signum) == -1)
	    {
		/* chalk up an error */
		ERROR(str, errno);
	    }
	}
    } while ((str = next_field(str)) != NULL);

    /* return appropriate error string */
    return(err_string());
}

/*
 *  renice_procs(str) - change the "nice" of processes, much like the
 *		"renice" command does; invoked in response to 'r'.
 */

const char *
renice_procs(char *str)
{
    char negate;
    int prio;
    int procnum;
    int uid;

    ERR_RESET;
    uid = getuid();

    /* allow for negative priority values */
    if ((negate = (*str == '-')) != 0)
    {
	/* move past the minus sign */
	str++;
    }

    /* use procnum as a temporary holding place and get the number */
    procnum = scanint(str, &prio);

    /* negate if necessary */
    if (negate)
    {
	prio = -prio;
    }

    /* check for validity */
    if (procnum == -1 || prio < PRIO_MIN || prio > PRIO_MAX)
    {
	return(bad_pri_value);
    }

    /* move to the first process number */
    if ((str = next_field(str)) == NULL)
    {
	return(no_proc_specified);
    }

    /* loop thru the process numbers, renicing each one */
    do
    {
	if (scanint(str, &procnum) == -1)
	{
	    ERROR(str, 0);
	}

	/* check process owner if we're not root */
	else if (uid && (uid != proc_owner(procnum)))
	{
	    ERROR(str, EACCES);
	}
	else if (setpriority(PRIO_PROCESS, procnum, prio) == -1)
	{
	    ERROR(str, errno);
	}
    } while ((str = next_field(str)) != NULL);

    /* return appropriate error string */
    return(err_string());
}

