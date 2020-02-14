
#define EXPDISTSCHED 1
#define LINUXSCHED 2

extern int currSchedClass; 

void setschedclass (int sched_class);

int getschedclass();

void next_epoch();
