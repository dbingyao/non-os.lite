/*-------------------Start heapsort.c program-------------------*/

/****************************************************************/
/*                         HEAPSORT                             */
/*                     C Program Source                         */
/*          Heapsort program for variable sized arrays          */
/*                 Version 1.0, 04 Oct 1992                     */
/*                Al Aburto, aburto@nosc.mil                    */
/*                                                              */
/* Based on the Heap Sort code in 'Numerical Recipes in C' by   */
/* William H. Press, Brian P. Flannery, Saul A. Teukolsky, and  */
/* William T. Vetterling, Cambridge University Press, 1990,     */
/* ISBN 0-521-35465-X.                                          */
/*                                                              */
/* The MIPS rating is based upon the program run time (runtime) */
/* for one iteration and a gcc 2.1 unoptimized (gcc -DUNIX)     */
/* assembly dump count of instructions per iteration for a i486 */
/* machine (assuming 80386 code).  This is the reference used.  */
/*                                                              */
/* The maximum amount of memory allocated is based on the 'imax'*/
/* variable in main(). Memory size = (2000*sizeof(long))*2^imax.*/
/* imax is currently set to 8, but this value may be increased  */
/* or decreased depending upon your system memory limits. For   */
/* standard Intel PC CPU machines a value of imax = 3 must be   */
/* used else your system may crash or hang up despite code in   */
/* the program to prevent this.                                 */
/****************************************************************/

/****************************************************/
/* Example Compilation:                             */
/* (1) UNIX Systems:                                */
/*     cc -DUNIX -O heapsort.c -o heapsort          */
/*     cc -DUNIX heapsort.c -o heapsort             */
/****************************************************/

/***************************************************************/
/* Timer options. You MUST uncomment one of the options below  */
/* or compile, for example, with the '-DUNIX' option.          */
/***************************************************************/
/* #define Amiga       */
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

#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <common.h>
#include <platform.h>

double nulltime,runtime,sta,stb;
double emips,hmips,lmips,smips[21];

long bplong,ErrorFlag;

long NLoops[21];

int HSORT(long m, long n, long p);

heapsort(void)
{

long  i,j,k,p,imax;

bplong = sizeof(long);

prints("\n   Heap Sort C Program\n");
prints("   Version 1.0, 04 Oct 1992\n\n");

prints("   Size of long (bytes): %d\n\n",bplong);

                                  /* NLoops[] holds number of loops  */
                                  /* (iterations) to conduct. Preset */
                                  /* to 1 iteration.                 */
for( i=0 ; i<= 20 ; i++)
{
 NLoops[i] = 1;
}
                                  /* Predetermine runtime (sec) for  */
                                  /* memory size 400 * sizeof(long),*/
                                  /* and 64 iterations. p = 0 means */
                                  /* don't print the result.         */
j = 400;
k = 64;
p = 0;
HSORT(j,k,p);
                                  /* Set number of iterations (loops)*/
                                  /* based on runtime above --- so   */
                                  /* program won't take forever on   */
                                  /* the slower machines.            */

i = 4;
if ( runtime > 0.0125 ) i = 1;

prints(" Predetermine runtime with msize %d: %10.4lf secs\n", j  * bplong, runtime);

prints(" Set number of iterations to %d\n", i);

NLoops[0] =  32 * i; 
NLoops[1] =  16 * i; 
NLoops[2] =   8 * i;
NLoops[3] =   4 * i;
NLoops[4] =   2 * i;
NLoops[5] =       i;
NLoops[6] =   i / 2;
NLoops[7] =   i / 4;

if ( i == 1 )
{
NLoops[6]  = 1;
NLoops[7]  = 1;
}

prints(" Redo the first run and print the results\n");
prints("   Array Size    RunTime      Scale    MIPS\n");       
prints("    (bytes)       (sec)\n");
j = 400;
k = NLoops[0];
p = 1;
HSORT(j,k,p);
                                  /* Save estimated mips result      */
smips[0] = emips;

j = 400;
ErrorFlag = 0;
                                  /* Now do it for memory sizes up to */ 
                                  /* (400*sizeof(long)) * (2 ** imax)*/
                                  /* where imax determines maximum    */
                                  /* amount of memory allocated.      */
                                  /* Currently I set imax = 4, so if  */
                                  /* sizeof(long) = 4 program will run*/
                                  /* from 4000, 8000, ..., and up to */
                                  /* 64000 byte memory size. You can*/
                                  /* increase imax, but imax = 8 is   */
                                  /* limit for this test program.     */

imax = 4;
prints(" Now do it for memory sizes up to %d * (2 ** imax), imax=%d\n", (j * bplong), imax );
prints("   Array Size    RunTime      Scale    MIPS\n");       
prints("    (bytes)       (sec)\n");
for( i=1 ; i<= imax ; i++)
{
   j = 2 * j;

   k = NLoops[i];

   HSORT(j,k,p);
   smips[i] = emips;

   if( ErrorFlag > 0L ) break;

}

if( ErrorFlag == 2L )
{
prints("\n   Could Not Allocate Memory for Array Size: %ld\n",j*bplong);
}

hmips = 0.0;
lmips = 1.0e+06;
for( k = 0; k < i; k++)
{
if( smips[k] > hmips ) hmips = smips[k];
if( smips[k] < lmips ) lmips = smips[k];
}

prints("\n   Runtime is the average for 1 iteration.\n");
prints("   High MIPS = %8.2lf\n",hmips);
prints("   Low  MIPS = %8.2lf\n\n",lmips);

}                                  /* End of main */


/*************************/
/*  Heap Sort Program    */
/*************************/

HSORT(long m, long n, long p)
{

register long *base;
register long i,j,k,l;
register long size;

long  iter,msize,iran,ia,ic,im,ih,ir;
long  count,ca,cb,cc,cd,ce,cf;

msize = m * bplong;
size  = m - 1;
base  = (long *)malloc((unsigned)msize);

prints("%s: base 0x%x msize %d iterations %d p %d\n", __func__, (int)base, msize, n, p);

ia = 106;
ic = 1283;
im = 6075;
ih = 1001;

   ErrorFlag = 0L;

   if( !base )
     {
     ErrorFlag = 2L;
     return 0;
     }

   sta = (double)get_timer(0) / CONFIG_SYS_HZ;
   stb = (double)get_timer(0) / CONFIG_SYS_HZ;
   nulltime = stb - sta;
   if ( nulltime < 0.0 ) nulltime = 0.0;

   count = 0;
   sta = (double)get_timer(0) / CONFIG_SYS_HZ;                       /* Start timing */
   for(iter=1 ; iter<=n ; iter++)       /* Do 'n' iterations */             

   {
       iran = 47;                        /* Fill with 'random' numbers */
       for(i=1 ; i<=size ; i++)                      
       {
       iran = (iran * ia + ic) % im;
       *(base+i) = 1 + (ih * iran) / im;
       }
       
       k = (size >> 1) + 1;              /* Heap sort the array */
       l = size;
       ca = 0; cb = 0; cc = 0;
       cd = 0; ce = 0; cf = 0;

       for (;;)
       {
       ca++;
       if (k > 1)
       {
          cb++;
          ir = *(base+(--k));
       }
       else
       {  
          cc++;
          ir = *(base+l);
          *(base+l) = *(base+1);
          if (--l == 1)
          {
               *(base+1) = ir;
               goto Done;
          }
       }

       i = k;
       j = k << 1;

       while (j <= l)
       {
          cd++;
          if ( (j < l) && (*(base+j) < *(base+j+1)) ) ++j;
          if (ir < *(base+j))
          {
               ce++;
               *(base+i) = *(base+j);   
               j += (i=j);
          }
          else 
          {
               cf++;
               j = l + 1;
          }
       }
       *(base+i) = ir;
       } 
Done:   
   count = count + ca;
   }
   stb =  (double)get_timer(0) / CONFIG_SYS_HZ;                       /* Stop timing */     
   runtime = stb - sta;
   if ( runtime < 0.0 ) runtime = 0.0;
                                       /* Scale runtime per iteration */
   runtime = (runtime - nulltime) / (double)n;
       
   ir = count / n;
   ir = (ir + ca) / 2;
                                       /* Estimate MIPS rating */
   emips = 24.0 * (double)size + 10.0 * (double)ir;
   emips = emips + 6.0 * (double)cb + 9.0 * (double)cc;
   emips = emips + 10.0 * (double)cd + 7.0 * (double)ce;
   emips = emips + 4.0 * (double)cf;
   sta   = 1.0e-06 * emips;
   emips = sta / runtime;

   if ( p != 0L )
   {
   prints("   %10ld %10.4lf %10.4lf %7.2lf\n",msize,runtime,sta,emips);
   }

   free(base);
return 0;
}

/*------ End of heapsort.c, say goodnight Vicki! (Sep 1992) ------*/
