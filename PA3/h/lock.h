/* lock.h - isbadlock */

#ifndef _LOCK_H_
#define _LOCK_H_

#ifndef NLOCKS
#define	NLOCKS		50	/* number of maximum locks */
#endif

#define	LFREE	'\01'		/* this lock is free		*/
#define	LUSED	'\02'		/* this lock is used		*/

#ifndef DELETED
#define DELETED -6
#endif

#define READ 0
#define WRITE 1

struct	lentry	{		/* lock table entry			*/
	char	lstate;		/* the state LFREE or LUSED		*/
	int	lqhead;		/* q index of head of list		*/
	int	lqtail;		/* q index of tail of list		*/
	int	ltype;		/* lock type READ or WRITE		*/
	int	lprio;		/* maximum priority among lock's wait queue */
	int 	lproc_list[NPROC]; /* bit mask of process ids currently holding the lock  */
};
extern	struct	lentry	rw_locks[];
extern	int	nextlock;
extern unsigned long ctr1000;
#define	isbadlock(s)	(s<0 || s>=NLOCKS)

#endif
