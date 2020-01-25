/*
 * System call prototypes.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * $FreeBSD$
 */

#ifndef _IBCS2_XENIX_H_
#define	_IBCS2_XENIX_H_

#include <sys/signal.h>
#include <sys/acl.h>
#include <sys/cpuset.h>
#include <sys/_ffcounter.h>
#include <sys/_semaphore.h>
#include <sys/ucontext.h>
#include <sys/wait.h>

#include <bsm/audit_kevents.h>

struct proc;

struct thread;

#define	PAD_(t)	(sizeof(register_t) <= sizeof(t) ? \
		0 : sizeof(register_t) - sizeof(t))

#if BYTE_ORDER == LITTLE_ENDIAN
#define	PADL_(t)	0
#define	PADR_(t)	PAD_(t)
#else
#define	PADL_(t)	PAD_(t)
#define	PADR_(t)	0
#endif

struct xenix_rdchk_args {
	char fd_l_[PADL_(int)]; int fd; char fd_r_[PADR_(int)];
};
struct xenix_chsize_args {
	char fd_l_[PADL_(int)]; int fd; char fd_r_[PADR_(int)];
	char size_l_[PADL_(long)]; long size; char size_r_[PADR_(long)];
};
struct xenix_ftime_args {
	char tp_l_[PADL_(struct timeb *)]; struct timeb * tp; char tp_r_[PADR_(struct timeb *)];
};
struct xenix_nap_args {
	char millisec_l_[PADL_(int)]; int millisec; char millisec_r_[PADR_(int)];
};
struct xenix_scoinfo_args {
	register_t dummy;
};
struct xenix_eaccess_args {
	char path_l_[PADL_(char *)]; char * path; char path_r_[PADR_(char *)];
	char flags_l_[PADL_(int)]; int flags; char flags_r_[PADR_(int)];
};
struct ibcs2_sigaction_args {
	char sig_l_[PADL_(int)]; int sig; char sig_r_[PADR_(int)];
	char act_l_[PADL_(struct ibcs2_sigaction *)]; struct ibcs2_sigaction * act; char act_r_[PADR_(struct ibcs2_sigaction *)];
	char oact_l_[PADL_(struct ibcs2_sigaction *)]; struct ibcs2_sigaction * oact; char oact_r_[PADR_(struct ibcs2_sigaction *)];
};
struct ibcs2_sigprocmask_args {
	char how_l_[PADL_(int)]; int how; char how_r_[PADR_(int)];
	char set_l_[PADL_(ibcs2_sigset_t *)]; ibcs2_sigset_t * set; char set_r_[PADR_(ibcs2_sigset_t *)];
	char oset_l_[PADL_(ibcs2_sigset_t *)]; ibcs2_sigset_t * oset; char oset_r_[PADR_(ibcs2_sigset_t *)];
};
struct ibcs2_sigpending_args {
	char mask_l_[PADL_(ibcs2_sigset_t *)]; ibcs2_sigset_t * mask; char mask_r_[PADR_(ibcs2_sigset_t *)];
};
struct ibcs2_sigsuspend_args {
	char mask_l_[PADL_(ibcs2_sigset_t *)]; ibcs2_sigset_t * mask; char mask_r_[PADR_(ibcs2_sigset_t *)];
};
struct ibcs2_getgroups_args {
	char gidsetsize_l_[PADL_(int)]; int gidsetsize; char gidsetsize_r_[PADR_(int)];
	char gidset_l_[PADL_(ibcs2_gid_t *)]; ibcs2_gid_t * gidset; char gidset_r_[PADR_(ibcs2_gid_t *)];
};
struct ibcs2_setgroups_args {
	char gidsetsize_l_[PADL_(int)]; int gidsetsize; char gidsetsize_r_[PADR_(int)];
	char gidset_l_[PADL_(ibcs2_gid_t *)]; ibcs2_gid_t * gidset; char gidset_r_[PADR_(ibcs2_gid_t *)];
};
struct ibcs2_sysconf_args {
	char name_l_[PADL_(int)]; int name; char name_r_[PADR_(int)];
};
struct ibcs2_pathconf_args {
	char path_l_[PADL_(char *)]; char * path; char path_r_[PADR_(char *)];
	char name_l_[PADL_(int)]; int name; char name_r_[PADR_(int)];
};
struct ibcs2_fpathconf_args {
	char fd_l_[PADL_(int)]; int fd; char fd_r_[PADR_(int)];
	char name_l_[PADL_(int)]; int name; char name_r_[PADR_(int)];
};
struct ibcs2_rename_args {
	char from_l_[PADL_(char *)]; char * from; char from_r_[PADR_(char *)];
	char to_l_[PADL_(char *)]; char * to; char to_r_[PADR_(char *)];
};
struct xenix_utsname_args {
	char addr_l_[PADL_(long)]; long addr; char addr_r_[PADR_(long)];
};
int	xenix_rdchk(struct thread *, struct xenix_rdchk_args *);
int	xenix_chsize(struct thread *, struct xenix_chsize_args *);
int	xenix_ftime(struct thread *, struct xenix_ftime_args *);
int	xenix_nap(struct thread *, struct xenix_nap_args *);
int	xenix_scoinfo(struct thread *, struct xenix_scoinfo_args *);
int	xenix_eaccess(struct thread *, struct xenix_eaccess_args *);
int	ibcs2_sigaction(struct thread *, struct ibcs2_sigaction_args *);
int	ibcs2_sigprocmask(struct thread *, struct ibcs2_sigprocmask_args *);
int	ibcs2_sigpending(struct thread *, struct ibcs2_sigpending_args *);
int	ibcs2_sigsuspend(struct thread *, struct ibcs2_sigsuspend_args *);
int	ibcs2_getgroups(struct thread *, struct ibcs2_getgroups_args *);
int	ibcs2_setgroups(struct thread *, struct ibcs2_setgroups_args *);
int	ibcs2_sysconf(struct thread *, struct ibcs2_sysconf_args *);
int	ibcs2_pathconf(struct thread *, struct ibcs2_pathconf_args *);
int	ibcs2_fpathconf(struct thread *, struct ibcs2_fpathconf_args *);
int	ibcs2_rename(struct thread *, struct ibcs2_rename_args *);
int	xenix_utsname(struct thread *, struct xenix_utsname_args *);

#ifdef COMPAT_43


#endif /* COMPAT_43 */


#ifdef COMPAT_FREEBSD4


#endif /* COMPAT_FREEBSD4 */


#ifdef COMPAT_FREEBSD6


#endif /* COMPAT_FREEBSD6 */


#ifdef COMPAT_FREEBSD7


#endif /* COMPAT_FREEBSD7 */


#ifdef COMPAT_FREEBSD10


#endif /* COMPAT_FREEBSD10 */

#define	IBCS2_XENIX_AUE_xenix_rdchk	AUE_NULL
#define	IBCS2_XENIX_AUE_xenix_chsize	AUE_FTRUNCATE
#define	IBCS2_XENIX_AUE_xenix_ftime	AUE_NULL
#define	IBCS2_XENIX_AUE_xenix_nap	AUE_NULL
#define	IBCS2_XENIX_AUE_xenix_scoinfo	AUE_NULL
#define	IBCS2_XENIX_AUE_xenix_eaccess	AUE_EACCESS
#define	IBCS2_XENIX_AUE_ibcs2_sigaction	AUE_NULL
#define	IBCS2_XENIX_AUE_ibcs2_sigprocmask	AUE_NULL
#define	IBCS2_XENIX_AUE_ibcs2_sigpending	AUE_NULL
#define	IBCS2_XENIX_AUE_ibcs2_sigsuspend	AUE_NULL
#define	IBCS2_XENIX_AUE_ibcs2_getgroups	AUE_GETGROUPS
#define	IBCS2_XENIX_AUE_ibcs2_setgroups	AUE_SETGROUPS
#define	IBCS2_XENIX_AUE_ibcs2_sysconf	AUE_NULL
#define	IBCS2_XENIX_AUE_ibcs2_pathconf	AUE_PATHCONF
#define	IBCS2_XENIX_AUE_ibcs2_fpathconf	AUE_FPATHCONF
#define	IBCS2_XENIX_AUE_ibcs2_rename	AUE_RENAME
#define	IBCS2_XENIX_AUE_xenix_utsname	AUE_NULL

#undef PAD_
#undef PADL_
#undef PADR_

#endif /* !_IBCS2_XENIX_H_ */
