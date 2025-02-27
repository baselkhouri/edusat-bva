#include "solver.h"
#include "options.h"

// ================== Global Variables ==================

int verbose = 0;
int preprocess = 0;
string proof_path = "";
double begin_time;
double read_cnf_time;
double preprocess_time;
double timeout = 0.0;

VAR_DEC_HEURISTIC VarDecHeuristic = VAR_DEC_HEURISTIC::MINISAT;
VAL_DEC_HEURISTIC ValDecHeuristic = VAL_DEC_HEURISTIC::PHASESAVING;

// Solver instance
Solver S;

// Options
unordered_map<string, option*> options = {
	{"v",           new intoption(&verbose, 0, 2, "Verbosity level")},
	{"p", 		 	new intoption(&preprocess, 0, 1, "{0: Don't preprocess cnf, 1: preprocess cnf (BVA)}")},
	{"timeout",     new doubleoption(&timeout, 0.0, 36000.0, "Timeout in seconds")},
	{"valdh",       new intoption((int*)&ValDecHeuristic, 0, 1, "{0: phase-saving, 1: literal-score}")},
	{"proof", 	 	new stringoption(&proof_path, "Path to proof file")}
};

/******************  main ******************************/

int main(int argc, char** argv){
	begin_time = cpuTime();
	cout << "======================================================" << endl;
	cout << "This is hacked edusat by Basel and Thomas :)" << endl;
	cout << "======================================================" << endl << endl;
	parse_options(argc, argv);
	ifstream in (argv[argc - 1]);
	if (!in.good())
		Abort("cannot read input file", 1);
	if (options["proof"]->val() != "") {
		std::string out(options["proof"]->val());
		S.set_proof_file(out);
		cout << "Dumping proof to " << out << endl << endl;
	}
	cout << "Reading CNF from " << argv[argc - 1] << endl;
	S.read_cnf(in);
	in.close();
	if (preprocess)
		S.preprocessBVA();
	S.solve();	

	return 0;
}
