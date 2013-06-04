/**
 * (C) Copyright 2013 Faraday Technology
 * BingYao Luo <bjluo@faraday-tech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <common.h>
#include <div64.h>
#include <platform.h>

static unsigned long long tick;

extern void heapsort(void);
extern void strswap(void);
extern void fibonanci(void);
extern void fourier_transform(void);
extern void mm(int argc, char *argv[]);

void pemutation(void);
void matrix_multiply(void);
void pointer(void);

struct _my_handler {
	void (*m_func)(void);
};

static struct _my_handler handler_test[] = {
					    {pemutation},
					    {pointer},
					    {strswap},
					    {matrix_multiply},
					    {heapsort},
					    {fibonanci},
					    {fourier_transform},
					    {0}
					   };

void timer2_tick_plus(void)
{
	tick++;
}

char str[] = {'1', '2', '3','4','5','\0'};

void swap (char *x, char *y)
{
        char tmp;

        tmp = *x;
        *x = *y;
        *y = tmp;
}

/*
 * 231, 321, 312, 132, 213,123
 */
void permute (char *start, char *end)
{
	char *cp;

	if (start == end) {
		prints("%s ", start);
	} else {
		for (cp = start; cp <= end; cp ++) {
			swap(cp, end);
			//prints("!%s! ", start);
			permute(start, end - 1);
			swap(cp, end);
			//prints("*%s* ", start);
		}
	}
}

void pemutation(void)
{
	prints(" %s\n", __func__);

	permute(str, str + strlen(str) - 1);
                
	prints("\n");
}

void matrix_multiply(void)
{
	char alg[] = {'n','v','p','t','i','u','b','m','w','r', 0}; 
	char *argv[3];
	int i, argc;

	prints(" %s\n", __func__);

	argv[0]  = (char *)malloc(4);
	argv[0][0] = 'm';
	argv[0][1] = 'm';
	argv[0][2] = '\0';
	argv[1]  = (char *)malloc(4);
	argv[1][0] = '-';
	argv[1][2] = '\0';
	argv[2]  = (char *)malloc(4);
	argv[2][1] = '\0';

	i = 0;
	while (alg[i]) {
		argv[1][1] = alg[i];
		argc = 2;

		if (alg[i] == 'u') {
			argv[2][0] = '8';
			argc = 3;
		} else if (alg[i] == 'b') {
			argv[2][0] = '6';
			argc = 3;
		} else if (alg[i] == 'm') {
			argv[2][0] = '4';
			argc = 3;
		} else if (alg[i] == 'w') {
			argv[2][0] = '4';
			argc = 3;
		}

		mm(argc, argv);
		i++;
	}
	prints("\n");
}

void pointer(void)
{
        int ary[3] = {100, 200, 300};
        int *p1;
        int *p2;
        int **pp1;
	char  *buffer = (char *) 0x10000000;  
	float **posArr = (float **) 0x10000000;
	int i;

	for (i=1; i<=8; i++)
		buffer[i-1] = i;

	prints(" %s\n", __func__);
	prints("&buffer: 0x%08x, buffer: 0x%08x\n", (int) &buffer, (int) ((int *)buffer));
	prints("&posArr: 0x%08x, posArr[0]: 0x%08x\n", (int) &posArr, (int) ((int *)posArr));
	prints("posArr[0]: 0x%08x, posArr[1]: 0x%08x\n", (int) ((int *)&posArr[0]), (int) ((int *)&posArr[1]));
	prints(" ary = 0x%08x, &p1 = 0x%08x, &p2=0x%08x, &pp1= 0x%08x\n",
		(int) ary, (int) &p1, (int) &p2, (int)&pp1);

	p1 = ary;
	p2 = &ary[1];
	pp1 = &p1;
	prints(" p1 = 0x%08x[%d], p2 = 0x%08x[%d], pp1 = 0x%08x[0x%08x [%d]]\n",
		(int)p1, (int)*p1, (int)p2, (int)*p2, (int)pp1, (int)*pp1, (int)**pp1);

	*pp1 = p2;
	prints(" p1 = 0x%08x[%d], p2 = 0x%08x[%d], pp1 = 0x%08x[0x%08x [%d]]\n",
		(int)p1, (int)*p1, (int)p2, (int)*p2, (int)pp1, (int)*pp1, (int)**pp1);

	*p1 -= 1;
	prints(" p1 = 0x%08x[%d], p2 = 0x%08x[%d], pp1 = 0x%08x[0x%08x [%d]]\n",
		(int)p1, (int)*p1, (int)p2, (int)*p2, (int)pp1, (int)*pp1, (int)**pp1);
	p1 = &ary[2];
	prints(" p1 = 0x%08x[%d], p2 = 0x%08x[%d], pp1 = 0x%08x[0x%08x [%d]]\n",
		(int)p1, (int)*p1, (int)p2, (int)*p2, (int)pp1, (int)*pp1, (int)**pp1);
	**pp1 += 1;
	prints(" p1 = 0x%08x[%d], p2 = 0x%08x[%d], pp1 = 0x%08x[0x%08x [%d]]\n",
		(int)p1, (int)*p1, (int)p2, (int)*p2, (int)pp1, (int)*pp1, (int)**pp1);
}

int do_spi_bootmode_test(int argc, char * const argv[])
{
	int i;

	/* Enable timer and count down */
	tick = 0;

	init_timer(2, TIMER_CLOCK, TIMER_USE_EXTCLK, timer2_tick_plus);

	i = 0;
	do {
		/* 200 ms */
		udelay(200000);

		prints(" Loop %d: stick %llu\n", i, tick);
		handler_test[(i % 6)].m_func();
		prints(" Loop %d: etick %llu\n", i, tick);
		
		prints(" ------------------------ \n");

		// Press 'q' to leave the burnin
		if ('q' == kbhit()) {
			disable_timer(2);
			break;
		}

		i++;
	} while (1);

	return 0;
}
