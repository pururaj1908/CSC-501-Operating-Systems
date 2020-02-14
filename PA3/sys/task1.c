#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>
#include <lock.h>



void swriter (char *msg, int sem)
{
	kprintf ("  %s: to acquire sem\n", msg);
        wait(sem);
        kprintf ("  %s: acquired sem, sleep 10s\n", msg);
	sleep (10);
        kprintf ("  %s: to release sem\n", msg);
        signal(sem);	
}

void sreader (char *msg, int sem)
{
	kprintf ("  %s: to acquire sem\n", msg);
	wait(sem);
	kprintf ("  %s: acquired sem\n", msg);
        kprintf ("  %s: to release sem\n", msg);
	signal(sem);
}	 

void testsem()
{
	int sem;
	int rd1, rd2;
	int wr1;
	
	sem = screate(1); /* count = 1 passing for semaphore*/
	
	rd1 = create(sreader, 2000, 40, "readerA", 2, "reader A", sem);
        rd2 = create(sreader, 2000, 30, "readerB", 2, "reader B", sem);
        wr1 = create(swriter, 2000, 10, "writer", 2, "writer", sem);
	
	kprintf("-start writer, then sleep 1s. sem granted to write (prio 10)\n");
        resume(wr1);
        sleep (1);
	kprintf("Priority of writer:%d\n", getprio(wr1));

        kprintf("-start reader A, then sleep 1s. reader A(prio 40) blocked on the sem\n");
        resume(rd1);
        sleep (1);
	kprintf("Priority of writer:%d\n", getprio(wr1));

        kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the sem\n");
        resume (rd2);
	sleep (1);
	kprintf("Priority of writer:%d\n", getprio(wr1));
				
}		


void lreader (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, 20);
        kprintf ("  %s: acquired lock\n", msg);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void lwriter (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, 20);
        kprintf ("  %s: acquired lock, sleep 10s\n", msg);
        sleep (10);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void testlck()
{
	int ld;
	int rd1, rd2;
	int wr1;
	
	ld = lcreate(); 
	
	rd1 = create(lreader, 2000, 40, "readerA", 2, "reader A", ld);
        rd2 = create(lreader, 2000, 30, "readerB", 2, "reader B", ld);
        wr1 = create(lwriter, 2000, 10, "writer", 2, "writer", ld);
	
	kprintf("-start writer, then sleep 1s. lock granted to write (prio 10)\n");
        resume(wr1);
        sleep (1);
	kprintf("Priority of writer:%d\n", getprio(wr1));

        kprintf("-start reader A, then sleep 1s. reader A(prio 40) blocked on the lock\n");
        resume(rd1);
        sleep (1);
	kprintf("Priority of writer:%d\n", getprio(wr1));

        kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the lock\n");
        resume (rd2);
	sleep (1);
	kprintf("Priority of writer:%d\n", getprio(wr1));
				
}

int task1()
{
	kprintf("\nTest Results from Semaphore approach :-\n");
	testsem();
	kprintf("\nEnd of Test Result from Semaphore approach\n");
	kprintf("\nTest Results from Priority Inheritance approach :-\n");
	testlck();
	kprintf("\nEnd of Test Result Priority Inheritance approach\n");
	
}	
