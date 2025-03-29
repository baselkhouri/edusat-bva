#include "solver.h"
#include "options.h"

// ================== Global Variables ==================

int verbose = 0;
string proof_path = "";
double solving_begin_time;
double timeout = 0.0;

VAR_DEC_HEURISTIC VarDecHeuristic = VAR_DEC_HEURISTIC::MINISAT;
VAL_DEC_HEURISTIC ValDecHeuristic = VAL_DEC_HEURISTIC::PHASESAVING;

// Solver instance
Solver S;

// Options
unordered_map<string, option*> options = {
	{"v",           new intoption(&verbose, 0, 2, "Verbosity level")},
	{"timeout",     new doubleoption(&timeout, 0.0, 36000.0, "Timeout in seconds")},
	{"valdh",       new booloption((int*)&ValDecHeuristic, "{0: phase-saving, 1: literal-score}")},
	{"proof", 	 	new stringoption(&proof_path, "Path to proof file")}
};

/******************  main ******************************/

int main(int argc, char** argv){
	parse_options(argc, argv);
	ifstream in (argv[argc - 1]);
	if (!in.good())
		Abort("cannot read input file", 1);
	if (options["proof"]->val() != "") {
		std::string out(options["proof"]->val());
		S.set_proof_file(out);
	}
	TIME_BLOCK("[   EDUSAT   ] Solve");
	{
		TIME_BLOCK("[   EDUSAT   ] Reading CNF");
		S.read_cnf(in);
	}
	in.close();
	solving_begin_time = cpuTime();
	{
		TIME_BLOCK("[   EDUSAT   ] Search");
		S.solve();
	}
	return 0;
}
