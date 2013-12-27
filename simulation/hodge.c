//first trimmed down


/* HODGE-C -- A C implementation of Gerhard & Schuster's hodge-podge machine */

/* hodge.c -- main program

   Copyright (C) 1993 Joerg Heitkoetter

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */

#include <string.h>
#include "hodge.h"

/* flags */
bool moore = FALSE;			/* von-Neumann default */
bool torus = FALSE;			/* bounded by default */

int n = 100;				/* number of states from SANE to ILL */
int ncells = 128;			/* number of each side of grid */
int nticks = 1000;			/* timer ticks to computer */

int hotval;				/* initial value at hot spot */

int random_seed = 12345678;		/* seed for random number generator */

int nsane;				/* number of sane cells */
int ninfected;				/* number of infected cells */
int nill;				/* number of ill cells */

int k1 = 2;				/* G&S's default */
int k2 = 3;				/* G&S's default */
int g = 25;				/* some nice waves */

grid_t cell, ncell;			/* the CA storage */

void hodge(void);
cell_s get_state (int *val);

int
map (i)
    int i;
{
    if (!torus) {
	if (i < 0)
	    return (0);
	else if (i == ncells)
	    return (ncells - 1);
	else
	    return (i);
    } else {
	if (i < 0)
	    return (ncells - 1);
	else if (i == ncells)
	    return (0);
	else
	    return (i);
    }
}


void main (void)
{
    int c, index;
    hodge();
}

void hodge (void)
{
    int A, B, S;
    int i, j;
    int t = 0;
    int nval;				/* range: [0..8*n + n] */

    //    initialize (random_seed);

    while (t++ < nticks) {
	nsane = ninfected = nill = 0;

	for (i = 0; i < ncells; i++)
	    for (j = 0; j < ncells; j++) {
		A = scount (i, j, ILL);
		B = scount (i, j, INFECTED);
		S = vsum (i, j) + cell[i][j].value;

		switch (cell[i][j].state) {
		 case SANE:
		     nsane++;
		     nval = (cell_v) (A / k1 + B / k2);
		     break;

		 case INFECTED:
		     ninfected++;
		     B++;		/* add self to INFECTED */
		     nval = (cell_v) (S / B + g);
		     break;

		 case ILL:
		     nill++;
		     nval = V_SANE;
		     break;
		}

	       /* set state & value */
		ncell[i][j].state = get_state (&nval);
		ncell[i][j].value = (cell_v) nval;
	    }


       /* copy new cells to old */
	memcpy (cell, ncell, sizeof (grid_t));
    }
}

/*
 *	initialize -- init the global data structures
 */
void
initialize (seed)
    int seed;
{
    int i, j, center, thres = 0;

    srand (seed);

	for (i = thres; i < ncells-thres; i++)
	    for (j = thres; j < ncells-thres; j++) {
		cell[i][j].state = (cell_s) (random () % MAXSTATES);

		if (cell[i][j].state == SANE)
		    cell[i][j].value = V_SANE;
		else if (cell[i][j].state == INFECTED)
		    cell[i][j].value = (cell_v) (rand () % (n - 1) + 1);
		else
		    cell[i][j].value = V_ILL;

		ncell[i][j].state = SANE;
		ncell[i][j].value = V_SANE;
	    }
}

/*
 *	get_state -- compute the state from a cell's value
 */
cell_s get_state (int *val){
    if (*val <= V_SANE) {
	*val = V_SANE;
	return (SANE);
    } else {
	if (*val >= V_ILL) {
	    *val = V_ILL;
	    return (ILL);
	} else {
	    return (INFECTED);
	}
    }
}

/*
 *	scount -- count cells in specified state
 */
int
scount (i, j, state)
    int i, j;
    cell_s state;
{
    int count = 0;

   /* always count von-Neumann cells */
    if (cell[map (i - 1)][j].state == state)
	++count;
    if (cell[i][map (j - 1)].state == state)
	++count;
    if (cell[i][map (j + 1)].state == state)
	++count;
    if (cell[map (i + 1)][j].state == state)
	++count;

   /* sometimes count Moore cells */
    if (moore) {
	if (cell[map (i - 1)][map (j - 1)].state == state)
	    ++count;
	if (cell[map (i + 1)][map (j - 1)].state == state)
	    ++count;
	if (cell[map (i - 1)][map (j + 1)].state == state)
	    ++count;
	if (cell[map (i + 1)][map (j + 1)].state == state)
	    ++count;
    }
    return (count);
}

/*
 *	vsum -- sum up all neighbour cells' values
 */
int
vsum (i, j)
    int i, j;
{
    int sum = 0;

   /* always sum von-Neumann cells */
    sum = cell[map (i - 1)][j].value
	+ cell[i][map (j - 1)].value
	+ cell[i][map (j + 1)].value
	+ cell[map (i + 1)][j].value;

   /* sometimes sum Moore cells */
    if (moore) {
	sum += cell[map (i - 1)][map (j - 1)].value
	    + cell[map (i + 1)][map (j - 1)].value
	    + cell[map (i - 1)][map (j + 1)].value
	    + cell[map (i + 1)][map (j + 1)].value;
    }
    return (sum);
}
