#ifndef __TIMER_H
#define __TIMER_H

/* Timer sources -- we get four on the SH4 */
#define TMU0	0	/* Available */
#define TMU1	1	/* Available */
#define TMU2	2	/* Available */
#define WDT	3	/* Not supported yet */

/* Pre-initialize a timer; set values but don't start it */
int timer_prime(int which, uint32 speed);

/* Start a timer -- starts it running (and interrupts if applicable) */
int timer_start(int which);

/* Stop a timer -- and disables its interrupt */
int timer_stop(int which);

/* Returns the count value of a timer */
uint32 timer_count(int which);

/* Clears the timer underflow bit and returns what its value was */
int timer_clear(int which);

/* Millisecond accurate spin-lock sleep function */
void timer_sleep(int ms);

/* Init function */
int timer_init();

/* Shutdown */
void timer_shutdown();

#endif	/* __TIMER_H */

