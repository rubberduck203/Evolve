#pragma once

class NewUniverseOptions
{
public:
	NewUniverseOptions();

	int num_strains;

	int seed;
	int width;
	int height;
	int want_barrier;

	bool	has_strain[8];
	int	energy[8];
	CString	filename[8];
	bool	sexonly[8];
	int	population[8];

	KFORTH_MUTATE_OPTIONS kfmo;
};
