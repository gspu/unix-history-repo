/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)subr_log.c	7.10 (Berkeley) %G%
 */

/*
 * Error log buffer for kernel printf's.
 */

#include "param.h"
#include "user.h"
#include "proc.h"
#include "vnode.h"
#include "ioctl.h"
#include "msgbuf.h"
#include "file.h"
#include "errno.h"

#define LOG_RDPRI	(PZERO + 1)

#define LOG_ASYNC	0x04
#define LOG_RDWAIT	0x08

struct logsoftc {
	int	sc_state;		/* see above for possibilities */
	struct	proc *sc_selp;		/* process waiting on select call */
	int	sc_pgid;		/* process/group for async I/O */
} logsoftc;

int	log_open;			/* also used in log() */

/*ARGSUSED*/
logopen(dev)
	dev_t dev;
{
	register struct msgbuf *mbp = msgbufp;

	if (log_open)
		return (EBUSY);
	log_open = 1;
	logsoftc.sc_pgid = u.u_procp->p_pid;	/* signal process only */
	/*
	 * Potential race here with putchar() but since putchar should be
	 * called by autoconf, msg_magic should be initialized by the time
	 * we get here.
	 */
	if (mbp->msg_magic != MSG_MAGIC) {
		register int i;

		mbp->msg_magic = MSG_MAGIC;
		mbp->msg_bufx = mbp->msg_bufr = 0;
		for (i=0; i < MSG_BSIZE; i++)
			mbp->msg_bufc[i] = 0;
	}
	return (0);
}

/*ARGSUSED*/
logclose(dev, flag)
	dev_t dev;
{
	log_open = 0;
	logsoftc.sc_state = 0;
	logsoftc.sc_selp = 0;
}

/*ARGSUSED*/
logread(dev, uio, flag)
	dev_t dev;
	struct uio *uio;
	int flag;
{
	register struct msgbuf *mbp = msgbufp;
	register long l;
	register int s;
	int error = 0;

	s = splhigh();
	while (mbp->msg_bufr == mbp->msg_bufx) {
		if (flag & IO_NDELAY) {
			splx(s);
			return (EWOULDBLOCK);
		}
		logsoftc.sc_state |= LOG_RDWAIT;
		if (error = tsleep((caddr_t)mbp, LOG_RDPRI | PCATCH,
		    "klog", 0)) {
			splx(s);
			return (error);
		}
	}
	splx(s);
	logsoftc.sc_state &= ~LOG_RDWAIT;

	while (uio->uio_resid > 0) {
		l = mbp->msg_bufx - mbp->msg_bufr;
		if (l < 0)
			l = MSG_BSIZE - mbp->msg_bufr;
		l = MIN(l, uio->uio_resid);
		if (l == 0)
			break;
		error = uiomove((caddr_t)&mbp->msg_bufc[mbp->msg_bufr],
			(int)l, uio);
		if (error)
			break;
		mbp->msg_bufr += l;
		if (mbp->msg_bufr < 0 || mbp->msg_bufr >= MSG_BSIZE)
			mbp->msg_bufr = 0;
	}
	return (error);
}

/*ARGSUSED*/
logselect(dev, rw)
	dev_t dev;
	int rw;
{
	int s = splhigh();

	switch (rw) {

	case FREAD:
		if (msgbufp->msg_bufr != msgbufp->msg_bufx) {
			splx(s);
			return (1);
		}
		logsoftc.sc_selp = u.u_procp;
		break;
	}
	splx(s);
	return (0);
}

logwakeup()
{
	struct proc *p;

	if (!log_open)
		return;
	if (logsoftc.sc_selp) {
		selwakeup(logsoftc.sc_selp, 0);
		logsoftc.sc_selp = 0;
	}
	if (logsoftc.sc_state & LOG_ASYNC) {
		if (logsoftc.sc_pgid < 0)
			gsignal(-logsoftc.sc_pgid, SIGIO); 
		else if (p = pfind(logsoftc.sc_pgid))
			psignal(p, SIGIO);
	}
	if (logsoftc.sc_state & LOG_RDWAIT) {
		wakeup((caddr_t)msgbufp);
		logsoftc.sc_state &= ~LOG_RDWAIT;
	}
}

/*ARGSUSED*/
logioctl(dev, com, data, flag)
	caddr_t data;
{
	long l;
	int s;

	switch (com) {

	/* return number of characters immediately available */
	case FIONREAD:
		s = splhigh();
		l = msgbufp->msg_bufx - msgbufp->msg_bufr;
		splx(s);
		if (l < 0)
			l += MSG_BSIZE;
		*(off_t *)data = l;
		break;

	case FIONBIO:
		break;

	case FIOASYNC:
		if (*(int *)data)
			logsoftc.sc_state |= LOG_ASYNC;
		else
			logsoftc.sc_state &= ~LOG_ASYNC;
		break;

	case TIOCSPGRP:
		logsoftc.sc_pgid = *(int *)data;
		break;

	case TIOCGPGRP:
		*(int *)data = logsoftc.sc_pgid;
		break;

	default:
		return (-1);
	}
	return (0);
}
