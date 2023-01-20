#ifndef __TIMER_H
#define __TIMER_H

void timeout     (void (*func)(void*), void *arg, int time);
void untimeout   (void (*func)(void*), void *arg);
void calltimeout (void);

#endif
