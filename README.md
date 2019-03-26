# STY1-Thread-KernelLab-part2
Project 05 Thread &amp; Kernel Lab - SC-T-215-STY1 - Operating systems course in Reykjavik University - Spring 2019

1. The goal is to make things thread-safe AND FAST. 
- We put a limit on how many consumers you can start from cmd (2x the number of producers). There are other options however (HINT HINT, see what are to do in the KTM). 
- You will need to put back in what you did on part1 first and then focus on making the producer and consumer functions thread-safe and fast but you MUST NOT CHANGE THE FUNDAMENTAL BEHAVIOUR of the functions. I.e. you can not change the calls to the rand_sleep() nor can you skip steps like what is printed and/or otherwise done. 
2. Bonus IDEAS: 
- Use signal handlers to your advantage. 
- Implement a led per slot in the array.
- There is one bonus also mentioned in the KTM description below. 
- If you have a cool idea for a bonus, ask me in private msg. on Piazza. 
3. Following is the description for what you must add to the kernel module (kmt.c): 
```
/***********************************************************
 * Part 2: DESCRIPTION OF YOUR TASK                        *
 ***********************************************************
 * Your task is to add functionality such that a process   *
 * can request a signal sent to it when one of the lights  *
 * are turned ON or OFF. You will need to:                 *
 * 1) Add a second sysfs file to the light object called   *
 * "reqsig".                                               *
 *                                                         *
 * 2) Implement the reqsig_store() functions such that can *
 * pars from the input (i.e. from buf) the 4 parameters    *
 * that are needed:                                        *
 *      I)   the pid of the process                        *
 *      II)  the number of the signal to send to           *
 *      III) a led (pin) to monitor (R/G/B)                *
 *      IV)  the action to look for (i.e. turning led on   *
 *           or off)                                       *
 * You must create some data structure to store this info  *
 * You should not register the same information multiple   *
 * times. I.e. if an identical registration comes in it    *
 * should be ignored.                                      *
 *                                                         *
 * 3) You must implement the reqsig_show() function that   *
 * prints out all currently registrations in order of pid. *
 *                                                         *
 * 4) You must add functionality to the light_store()      *
 * function such that signals are sent to those processes  *
 * that have registered to be notified about the given     *
 * change. For example if process with pid 9999 has asked  *
 * to be sent signal 10 (SIGUSR1) when the red light is    *
 * turned on then, when light_store changes the red light  *
 * from 0 to 1 a signal should be sent (but not if it was  *
 * already 1, only if it was chanted from 0 to 1).         *
 *                                                         *
 * BONUS) There is a potential security flaw here, explain *
 * what it is and given a detailed example of how it could *
 * be exploited.                                           *
 ************************************************************/
 ```
