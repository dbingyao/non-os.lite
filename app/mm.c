/* matrix multiply tests -- C language, version 1.0, May 1993
 *
 * compile with -DN=<size>
 *
 * I usually run a script file
 *   time.script 500 >500.times
 * where the script file contains
 *   cc -O -DN=$1 mm.c
 *   a.out -n             (I suggest at least two runs per method to
 *   a.out -n             alert you to variations.  Five or ten runs
 *   a.out -t             each, giving avg. and std dev. of times is
 *   a.out -t             best.)
 *     ...
 *
 * Contact Mark Smotherman (mark@cs.clemson.edu) for questions, comments,
 * and to report results showing wide variations.  E.g., a wide variation
 * appeared on an IBM RS/6000 Model 320 with "cc -O -DN=500 mm.c" (xlc
 * compiler):
 *  500x500 mm - normal algorithm                     utime     230.81 secs
 *  500x500 mm - normal algorithm                     utime     230.72 secs
 *  500x500 mm - temporary variable in loop           utime     231.00 secs
 *  500x500 mm - temporary variable in loop           utime     230.79 secs
 *  500x500 mm - unrolled inner loop, factor of  8    utime     232.09 secs
 *  500x500 mm - unrolled inner loop, factor of  8    utime     231.84 secs
 *  500x500 mm - pointers used to access matrices     utime     230.74 secs
 *  500x500 mm - pointers used to access matrices     utime     230.45 secs
 *  500x500 mm - blocking, factor of  32              utime      60.40 secs
 *  500x500 mm - blocking, factor of  32              utime      60.57 secs
 *  500x500 mm - interchanged inner loops             utime      27.36 secs
 *  500x500 mm - interchanged inner loops             utime      27.40 secs
 *  500x500 mm - 20x20 subarray (from D. Warner)      utime       9.49 secs
 *  500x500 mm - 20x20 subarray (from D. Warner)      utime       9.50 secs
 *  500x500 mm - 20x20 subarray (from T. Maeno)       utime       9.10 secs
 *  500x500 mm - 20x20 subarray (from T. Maeno)       utime       9.05 secs
 *
 * The algorithms can also be sensitive to TLB thrashing.  On a 600x600
 * test an IBM RS/6000 Model 30 showed variations depending on relative
 * location of the matrices.  (The model 30 has 64 TLB entries organized
 * as 2-way set associative.)
 *
 * 600x600 mm - 20x20 subarray (from T. Maeno)       utime      19.12 secs
 * 600x600 mm - 20x20 subarray (from T. Maeno)       utime      19.23 secs
 * 600x600 mm - 20x20 subarray (from D. Warner)      utime      18.87 secs
 * 600x600 mm - 20x20 subarray (from D. Warner)      utime      18.64 secs
 * 600x600 mm - 20x20 btranspose (Warner/Smotherman) utime      17.70 secs
 * 600x600 mm - 20x20 btranspose (Warner/Smotherman) utime      17.76 secs
 * 
 * Changing the declaration to include 10000 dummy entries between the
 * b and c matrices (suggested by T. Maeno), i.e.,
 *
 * double a[N][N],b[N][N],dummy[10000],c[N][N],d[N][N],bt[N][N];
 *
 * 600x600 mm - 20x20 subarray (from T. Maeno)       utime      16.41 secs
 * 600x600 mm - 20x20 subarray (from T. Maeno)       utime      16.40 secs
 * 600x600 mm - 20x20 subarray (from D. Warner)      utime      16.68 secs
 * 600x600 mm - 20x20 subarray (from D. Warner)      utime      16.67 secs
 * 600x600 mm - 20x20 btranspose (Warner/Smotherman) utime      16.97 secs
 * 600x600 mm - 20x20 btranspose (Warner/Smotherman) utime      16.98 secs
 *
 * I hope to add other algorithms (e.g., Strassen-Winograd) in the near
 * future.
 */

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

#include <stdio.h>
#include <math.h>

#include <common.h>
#include <platform.h>

void msg();
void normal();
void tnsq();
void unroll4();
void unroll8();
void unroll16();
void pnsq();
void transpose();
void reg_loops();
void tiling();
void maeno();
void warner();
void robert();

#define N	20

double a[N][N],b[N][N],c[N][N],bt[N][N],d[N][N];

mm(argc,argv)
  int argc;
  char *argv[];
{
  char alg_choice;
  int parm;
  int i,j;
  double sum,row_sum;
  double t_start=0.0,t_end=0.0;

/* defaults */
#if 0 //ted
  alg_choice = 'x';
  parm = 4;
#else
//  argc = 0;
//  alg_choice = 'n';
//  parm = 4;
#endif

  i = 1;
  while(argc>1){
    switch(argv[i][0]){
       case '-':
       alg_choice = argv[i][1];
       break;
       default:
       sscanf(argv[i],"%d",&parm);
    }
    argc--;
    i++;
  }

  switch(alg_choice){
    case 'n':
       prints("%3dx%3d mm - normal algorithm                    ",N,N);
       break;
    case 'v':
       prints("%3dx%3d mm - temporary variable in loop          ",N,N);
       break;
    case 'p':
       prints("%3dx%3d mm - pointers used to access matrices    ",N,N);
       break;
    case 't':
       prints("%3dx%3d mm - transposed b matrix                 ",N,N);
       break;
    case 'i':
       prints("%3dx%3d mm - interchanged inner loops            ",N,N);
       break;
    case 'u':
       if((parm!=4)&&(parm!=8)&&(parm!=16)){
       prints("%s: unrolling factor of %d not implemented\n",argv[0],parm);
       prints("  usage: %s -<option> [<n>]\n",argv[0]);
       prints("  option u - innermost loop unrolled by factor of n=4,8,16\n");
       return;
       }
       prints("%3dx%3d mm - unrolled inner loop, factor of %2d   ",N,N,parm);
       break;
    case 'b':
       if((parm<4)||(parm>N)){
       prints("%s: blocking step size of %d is unreasonable\n",argv[0],parm);
       return;
       }
       prints("%3dx%3d mm - blocking, factor of %3d             ",N,N,parm);
       break;
    case 'm':
       if((N%parm)||(N%4)){
       prints("%s: matrix size for Maeno method must be\n",argv[0]);
       prints(" evenly divisible both by 4 and by block size %d\n",parm);
       return;
       }
       if(parm%4){
       prints("%s: block size for Maeno method must be",argv[0]);
       prints(" evenly divisible by 4\n");
       return;
       }
       prints("%3dx%3d mm - %3dx%3d subarray (from T. Maeno)    ",N,N,
       parm,parm);
       break;
    case 'w':
       if ((N%parm)||(N%2))
	 {
	  prints("%s: matrix size for Warner method must be\n",argv[0]);
	  prints(" evenly divisible both by 2 and by block size %d\n",parm);
	  return;
	 }
       if (parm%2)
	 {
	  prints("%s: block size for Warner method must be",argv[0]);
	  prints(" evenly divisible by 2\n");
	  return;
	 }
       prints("%3dx%3d mm - %3dx%3d subarray (from D. Warner)   ",N,N,parm,parm);
       break;
    case 'r':
       prints("%3dx%3d mm - Robert's algorithm                  ",N,N);
       break;
    case 'x':
       prints("%s: unspecified algorithm\n",argv[0]);
       msg(argv[0]);
       return;
    default:
       prints("%s: unknown algorithm choice %c\n",argv[0],alg_choice);
       msg(argv[0]);
       return;
  }

/* set coefficients so that result matrix should have row entries
 * equal to (1/2)*n*(n-1)*i in row i
 */
  for (i=0;i<N;i++){
    for (j=0;j<N;j++){
       a[i][j] = b[i][j] = (double) i;
    }
  }

/* try to flush cache */
  for(i=0;i<N;i++){
    for (j=0;j<N;j++){
       d[i][j] = 0.0;
    }
  }

  switch(alg_choice){
    case 'n':
       t_start = (double)get_timer(0)/CONFIG_SYS_HZ;
       normal();
       t_end = (double)get_timer(0)/CONFIG_SYS_HZ;
       prints(" utime %10.2f secs\n",t_end-t_start);
       break;
    case 'v':
       t_start = (double)get_timer(0)/CONFIG_SYS_HZ;
       tnsq();
       t_end = (double)get_timer(0)/CONFIG_SYS_HZ;
       prints(" utime %10.2f secs\n",t_end-t_start);
       break;
    case 'u':
       switch(parm){
       case 4:
	 t_start = (double)get_timer(0)/CONFIG_SYS_HZ;
	 unroll4();
	 t_end = (double)get_timer(0)/CONFIG_SYS_HZ;
	 break;   
       case 8:
	 t_start = (double)get_timer(0)/CONFIG_SYS_HZ;
	 unroll8();
	 t_end = (double)get_timer(0)/CONFIG_SYS_HZ;
	 break;   
       case 16:
	 t_start = (double)get_timer(0)/CONFIG_SYS_HZ;
	 unroll16();
	 t_end = (double)get_timer(0)/CONFIG_SYS_HZ;
	 break;   
       default:
	 prints("program logic bug in call to unrolled versions\n");
	 return;
       }
       prints(" utime %10.2f secs\n",t_end-t_start);
       break;
    case 'p':
       t_start = (double)get_timer(0)/CONFIG_SYS_HZ;
       pnsq();
       t_end = (double)get_timer(0)/CONFIG_SYS_HZ;
       prints(" utime %10.2f secs\n",t_end-t_start);
       break;
    case 't':
       t_start = (double)get_timer(0)/CONFIG_SYS_HZ;
       transpose();
       t_end = (double)get_timer(0)/CONFIG_SYS_HZ;
       prints(" utime %10.2f secs\n",t_end-t_start);
       break;
    case 'i':
       t_start = (double)get_timer(0)/CONFIG_SYS_HZ;
       reg_loops();
       t_end = (double)get_timer(0)/CONFIG_SYS_HZ;
       prints(" utime %10.2f secs\n",t_end-t_start);
       break;
    case 'b':
       t_start = (double)get_timer(0)/CONFIG_SYS_HZ;
       tiling(parm);
       t_end = (double)get_timer(0)/CONFIG_SYS_HZ;
       prints(" utime %10.2f secs\n",t_end-t_start);
       break;
    case 'm':
       t_start = (double)get_timer(0)/CONFIG_SYS_HZ;
       maeno(parm);
       t_end = (double)get_timer(0)/CONFIG_SYS_HZ;
       prints(" utime %10.2f secs\n",t_end-t_start);
       break;
    case 'w':
       t_start = (double)get_timer(0)/CONFIG_SYS_HZ;
       warner(parm);
       t_end = (double)get_timer(0)/CONFIG_SYS_HZ;
       prints(" utime %10.2f secs\n",t_end-t_start);
       break;
    case 'r':
       t_start = (double)get_timer(0)/CONFIG_SYS_HZ;
       robert();
       t_end = (double)get_timer(0)/CONFIG_SYS_HZ;
       prints(" utime %10.2f secs\n",t_end-t_start);
       break;
    default:
       prints("unknown algorithm!\n");
       return;
  }

/* check result */
  sum = 0.5*((double)(N*(N-1)));
  for (i=0;i<N;i++){
    row_sum = ((double)i)*sum;
    for (j=0;j<N;j++){
       if (c[i][j]!=row_sum){
       prints("error in result entry c[%d][%d]: %e != %e\n",
		i,j,c[i][j],row_sum);
       return;
       }
       if (a[i][j]!=((double)i)){
       prints("error in entry a[%d][%d]: %e != %d\n",
		i,j,a[i][j],i);
       return;
       }
       if (b[i][j]!=((double)i)){
       prints("error in entry b[%d][%d]: %e != %d\n",
		i,j,b[i][j],i);
       return;
       }
    }
  }
}

void msg(cmd)
char *cmd;
{
  prints("  usage: %s -<option> [<n>]\n",cmd);
  prints("  option n - normal matrix multiply\n");
  prints("  option v - normal matrix multiply using temp variable\n");
  prints("  option u - innermost loop unrolled by factor of n=4,8,16\n");
  prints("  option p - matrix multiply using pointers\n");
  prints("  option t - matrix multiply using transpose of b matrix\n");
  prints("  option i - matrix multiply with interchanged loops\n");
  prints("  option b - matrix multiply using blocking with step=n\n");
  prints("  option m - matrix multiply using Maeno method of blocking,\n");
  prints("             n specifies size of subarray (divisible by 4)\n");
  prints("  option w - matrix multiply using Warner method of blocking,\n");
  prints("             n specifies size of subarray (divisible by 2)\n");
  prints("  option r - matrix multiply using the Robert method with:\n");
  prints("             b transpose, pointers, and temporary variables\n");
}

void normal(){
    int i,j,k;
    for (i=0;i<N;i++){
       for (j=0;j<N;j++){
	   c[i][j] = 0.0;
	   for (k=0;k<N;k++){
	       c[i][j] += a[i][k]*b[k][j];
	   }
       }
    }
}

void tnsq(){
    int i,j,k;
    double temp;
    for (i=0;i<N;i++){
       for (j=0;j<N;j++){
	   temp = a[i][0]*b[0][j];
	   for (k=1;k<N;k++){
	       temp += a[i][k]*b[k][j];
	   }
	   c[i][j] = temp;
       }
    }
}

void unroll4(){
    int i,j,k;
    double temp;
    for (i=0;i<N;i++){
       for (j=0;j<N;j++){
	   temp = 0.0;
	   for (k=0;k<(N-3);k+=4){
	       temp += a[i][k]*b[k][j];
	       temp += a[i][k+1]*b[k+1][j];
	       temp += a[i][k+2]*b[k+2][j];
	       temp += a[i][k+3]*b[k+3][j];
	   }
	   for (;k<N;k++){
	       temp += a[i][k]*b[k][j];
	   }
	   c[i][j] = temp;
       }
    }
}

void unroll8(){
    int i,j,k;
    double temp;
    for (i=0;i<N;i++){
       for (j=0;j<N;j++){
	   temp = 0.0;
	   for (k=0;k<(N-7);k+=8){
	       temp += a[i][k]*b[k][j];
	       temp += a[i][k+1]*b[k+1][j];
	       temp += a[i][k+2]*b[k+2][j];
	       temp += a[i][k+3]*b[k+3][j];
	       temp += a[i][k+4]*b[k+4][j];
	       temp += a[i][k+5]*b[k+5][j];
	       temp += a[i][k+6]*b[k+6][j];
	       temp += a[i][k+7]*b[k+7][j];
	   }
	   for (;k<N;k++){
	       temp += a[i][k]*b[k][j];
	   }
	   c[i][j] = temp;
       }
    }
}

void unroll16(){
    int i,j,k;
    double temp;
    for (i=0;i<N;i++){
       for (j=0;j<N;j++){
	   temp = 0.0;
	   for (k=0;k<(N-15);k+=16){
	       temp += a[i][k]*b[k][j];
	       temp += a[i][k+1]*b[k+1][j];
	       temp += a[i][k+2]*b[k+2][j];
	       temp += a[i][k+3]*b[k+3][j];
	       temp += a[i][k+4]*b[k+4][j];
	       temp += a[i][k+5]*b[k+5][j];
	       temp += a[i][k+6]*b[k+6][j];
	       temp += a[i][k+7]*b[k+7][j];
	       temp += a[i][k+8]*b[k+8][j];
	       temp += a[i][k+9]*b[k+9][j];
	       temp += a[i][k+10]*b[k+10][j];
	       temp += a[i][k+11]*b[k+11][j];
	       temp += a[i][k+12]*b[k+12][j];
	       temp += a[i][k+13]*b[k+13][j];
	       temp += a[i][k+14]*b[k+14][j];
	       temp += a[i][k+15]*b[k+15][j];
	   }
	   for (;k<N;k++){
	       temp += a[i][k]*b[k][j];
	   }
	   c[i][j] = temp;
       }
    }
}

void pnsq(){
    int i,j,k;
    double temp,*pa,*pb;
    for (i=0;i<N;i++){
       for (j=0;j<N;j++){
	   pa = a[i];
	   pb = &b[0][j];
	   temp = (*pa++)*(*pb);
	   for (k=1;k<N;k++){
	       pb = pb + N;
	       temp += (*pa++)*(*pb);
	   }
	   c[i][j] = temp;
       }
    }
}

void transpose(){
    int i,j,k;
    double temp;
    for (i=0;i<N;i++){
       for (j=0;j<N;j++){
	   bt[j][i] = b[i][j];
       }
    }
    for (i=0;i<N;i++){
       for (j=0;j<N;j++){
	   temp = a[i][0]*bt[j][0];
	   for (k=1;k<N;k++){
	       temp += a[i][k]*bt[j][k];
	   }
	   c[i][j] = temp;
       }
    }
}

/* from Monica Lam ASPLOS-IV paper */
void reg_loops(){
    int i,j,k;
    register double a_entry;
    for (i=0;i<N;i++){
       for (j=0;j<N;j++){
	   c[i][j] = 0.0;
       }
    }
    for (i=0;i<N;i++){
       for (k=0;k<N;k++){
	   a_entry = a[i][k];
	   for (j=0;j<N;j++){
	       c[i][j] += a_entry*b[k][j];
	   }
       }
    }
}

/* from Monica Lam ASPLOS-IV paper */
#define min(a,b) ((a)<=(b)?(a):(b))
void tiling(step)
int step;
{
    int i,j,k,jj,kk;
    register double a_entry;
    for (i=0;i<N;i++){
       for (j=0;j<N;j++){
	   c[i][j] = 0.0;
       }
    }
    for(kk=0;kk<N;kk+=step){
       for(jj=0;jj<N;jj+=step){
	   for (i=0;i<N;i++){
	       for(k=kk;k<min(kk+step,N);k++){
		   a_entry = a[i][k];
		   for(j=jj;j<min(jj+step,N);j++){
		       c[i][j] += a_entry*b[k][j];
		   }
		}
	   }
       }
    }
}


/* Matrix Multiply tuned for SS-10/30;
 *                      Maeno Toshinori
 *                      Tokyo Institute of Technology
 *
 * Using gcc-2.4.1 (-O2), this program ends in 12 seconds on SS-10/30. 
 *
 * in original algorithm - sub-area for cache tiling
 * #define      L       20
 * #define      L2      20
 * three 20x20 matrices reside in cache; two may be enough
 */

void maeno(lparm)
int lparm;
{
int i,j,k,kk,i2,r,kt,it;
double t0,t1,t2,t3,s,t4,t5,t6,t7;
       for(i=0;i<N;i++)
	 for(j=0;j<N;j++) {
	       c[i][j]=0.0;
	       }

       for (i2=0; i2<N; i2+=lparm)
	 for (kk=0; kk<N; kk+=lparm) {
	   it=i2+lparm;
	   kt=kk+lparm;
	       for (j=0; j<N; j+=4) 
	       for(i=i2;i<it;i+=2) {
		       t0=t1=t2=t3=0.0;
		       t4=t5=t6=t7=0.0;
		       for(k=kk; k<kt; k++) {
			       s=a[i][k];
			       t0 += s*b[k][j];
			       t1 += s*b[k][j+1];
			       t2 += s*b[k][j+2];
			       t3 += s*b[k][j+3];
			       s=a[i+1][k];
			       t4 += s*b[k][j];
			       t5 += s*b[k][j+1];
			       t6 += s*b[k][j+2];
			       t7 += s*b[k][j+3];
		       }
		       c[i][j]+=t0;
		       c[i][j+1]+=t1;
		       c[i][j+2]+=t2;
		       c[i][j+3]+=t3;
		       c[i+1][j]+=t4;
		       c[i+1][j+1]+=t5;
		       c[i+1][j+2]+=t6;
		       c[i+1][j+3]+=t7;
	       }
       }
}       


void warner(nb)
int nb;
{
       int i,j,k;
       int ii,jj,kk;
       double s00,s01,s10,s11;
/*
 * Matrix Multiply by Dan Warner, Dept. of Mathematics, Clemson University
 *
 *    mmbu2.f multiplies matrices a and b
 *    a and b are n by n matrices
 *    nb is the blocking parameter.
 *    the tuning guide indicates nb = 50 is reasonable for the
 *    ibm model 530 hence 25 should be reasonable for the 320 
 *    since the 320 has 32k rather than 64k of cache.      
 *    Inner loops unrolled to depth of 2
 *    The loop functions without clean up code at the end only
 *    if the unrolling occurs to a depth k which divides into n
 *    in this case n must be divisible by 2.
 *    The blocking parameter nb must divide into n if the
 *    multiply is to succeed without clean up code at the end. 
 *
 * converted to c by Mark Smotherman
 * note that nb must also be divisible by 2 => cannot use 25, so use 20
 */

       for( ii = 0; ii < N; ii += nb ){
	 for( jj = 0; jj < N; jj += nb ){
	   for( i = ii; i < ii + nb ; i++ ){
	       for( j = jj; j < jj + nb ; j++ ){
	       c[i][j] = 0.0;
	       }
	   }
	   for( kk = 0; kk < N; kk += nb ){
	       for( i = ii; i < ii + nb ; i += 2 ){
	       for( j = jj; j < jj + nb ; j += 2 ){
		 s00 = c[i  ][j  ];
		 s01 = c[i  ][j+1];
		 s10 = c[i+1][j  ];
		 s11 = c[i+1][j+1];
		 for( k = kk; k < kk + nb ; k++ ){
		   s00 = s00 + a[i  ][k]*b[k][j  ];
		   s01 = s01 + a[i  ][k]*b[k][j+1];
		   s10 = s10 + a[i+1][k]*b[k][j  ];
		   s11 = s11 + a[i+1][k]*b[k][j+1];
		 }
		 c[i  ][j  ] = s00;
		 c[i  ][j+1] = s01;
		 c[i+1][j  ] = s10;
		 c[i+1][j+1] = s11;
	       }
	       }
	   }
	 }
       }
}

/********************************************/
/* Contributed by Robert Debath 26 Nov 1995 */
/* rdebath@cix.compulink.co.uk              */
/********************************************/
void robert(void)
{
 int i,j,k;
 double temp, *pa, *pb;
 for (i = 0;i < N; i++)
    {
     for (j = 0;j < N; j++)
	 {
	  bt[j][i] = b[i][j];
	 }
    }

 for (i = 0; i < N; i++)
    {
     for (j = 0; j < N; j++)
	 {
	  pa = &a[i][0];
	  pb = &bt[j][0];
	  temp = (*pa++) * (*pb++);
	  
	  for (k = 1; k < N; k++)
	       {
		temp += (*pa++) * (*pb++);
	       }
	  c[i][j] = temp;
	 }
    }
}

/*-------- End of mm.c, say goodnight Becky! (Sep 1992) --------*/
