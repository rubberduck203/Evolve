#pragma once

//
// This class implements the Find feature.
//
// It will evaluate a find expression and set the radioactive tracer
// flag for organisms that match the expression.
//
// The find expression is given in KFORTH notation with special
// find instructions.
//
// To use:
//	1. Create instance of this class, give the find_expression
//	2. Check if error is TRUE or FALSE.
//	3. if error is FALSE, then error_message will explain the problem
//	4. Otherwise call execute() with the simulation to use.
//	5. Now all organisms that match the expression will have a radioactive tracer.
//
// 'reset_tracers' will first clear any previous tracers.
//

class OrganismFinder
{
public:
	OrganismFinder(CString find_expression, bool reset_tracers);
	~OrganismFinder();

	bool error;
	CString error_message;

	void execute(UNIVERSE *u);

	ORGANISM	*organism;

	int		min_energy;
	int		max_energy;
	int		avg_energy;

	int		min_generation;
	int		max_generation;
	int		avg_generation;

	int		min_age;
	int		max_age;
	int		avg_age;

	int		max_num_cells;

private:
	bool evalute(KFORTH_MACHINE *kfm, ORGANISM *o);

	bool		m_reset_tracers;

	KFORTH_PROGRAM	*m_kfp;

};
