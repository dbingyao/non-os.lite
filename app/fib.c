/**********************/
/* fib.c, 02 Mar 1997 */
/* Al Aburto Jr.      */
/**********************/

/***************************************************************/
/* Timer options. You MUST uncomment one of the options below  */
/* or compile, for example, with the '-DUNIX' option.          */
/***************************************************************/
/* #define Amiga       */
/* #define UNIX        */
/* #define UNIX_Old    */
/* #define VMS         */
/* #define BORLAND_C   */
/* #define MSC         */
/* #define MAC         */
/* #define IPSC        */
/* #define FORTRAN_SEC */
/* #define GTODay      */
/* #define CTimer      */
/* #define UXPM        */
/* #define MAC_TMgr    */
/* #define PARIX       */
/* #define POSIX       */
/* #define WIN32       */
/* #define POSIX1      */
/***********************/

#include <common.h>
#include <platform.h>

unsigned long fib(long);

unsigned long fib(x)
long x;
{
 if (x > 2)
  return(fib(x-1)+fib(x-2));
 else
  return(1);
}


void fibonanci(void)
{
 register unsigned long IMax,value;
 double starttime, benchtime;

 IMax = 20;

 prints("\n");
 prints("Fibonacci Benchmark\n");

 starttime = (double)get_timer(0)/CONFIG_SYS_HZ;
 value = fib(IMax);
 benchtime = ((double)get_timer(0) / CONFIG_SYS_HZ) - starttime;
  
 prints("\n");
 prints("The %02d'th Fibonacci Number is: %d\n",IMax,value);
 prints("Run Time (sec) =  %10.3lf\n\n",benchtime);
}

