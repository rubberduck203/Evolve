
#include "stdafx.h"
#include "evolve.h"
#include "NewUniverseOptions.h"

/*
 * Setup the defaults
 *
 */
NewUniverseOptions::NewUniverseOptions()
{
	int i;
	
	seed = (long) GetTickCount();
	width = 700;
	height = 600;
	want_barrier = 1;

	num_strains = 0;
	for(i=0; i<8; i++) {
		has_strain[i] = FALSE;
		energy[i] = 0;
		filename[i] = "";
		sexonly[i] = false;
		population[i] = 1;
	}

	energy[0] = 10000;

	kforth_mutate_options_defaults(&kfmo);

}




