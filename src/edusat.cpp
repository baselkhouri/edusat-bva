#include "solver.h"
#include "options.h"
#include "bva.h"

// ================== Global Variables ==================

int verbose = 0;
int preprocess = 0;
string proof_path = "";
string bva_export_path = "";
double solving_begin_time;
double begin_time;
double read_cnf_time;
double preprocess_time;
double timeout = 0.0;
int bva_length = 100;

VAR_DEC_HEURISTIC VarDecHeuristic = VAR_DEC_HEURISTIC::MINISAT;
VAL_DEC_HEURISTIC ValDecHeuristic = VAL_DEC_HEURISTIC::PHASESAVING;

// Solver instance
Solver S;

// Options
unordered_map<string, option*> options = {
	{"v",           new intoption(&verbose, 0, 2, "Verbosity level")},
	{"bva", 		 	new booloption(&preprocess, "{Apply Bounded Variable Addition (BVA) preprocessing technique}")},
	{"timeout",     new doubleoption(&timeout, 0.0, 36000.0, "Timeout in seconds")},
	{"valdh",       new booloption((int*)&ValDecHeuristic, "{0: phase-saving, 1: literal-score}")},
	{"proof", 	 	new stringoption(&proof_path, "Path to proof file")},
	{"bva-limit",   new intoption(&bva_length, 1, 10000000, "BVA Iterations")},
	{"bva-export",  new stringoption(&bva_export_path, "Export cnf to the specified file after BVA")},
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
	if (preprocess) {
		BVA::AutomatedReencoder processor(S.proof_tracer);
		processor.setIterations(bva_length);
		processor.readCNF(in);
		processor.applySimpleBVA();
		if (!bva_export_path.empty())
			processor.writeDimacsCNF(bva_export_path.c_str());
		// processor.writeDimacsCNF(0);
		preprocess_time = cpuTime() - begin_time;
		solving_begin_time = cpuTime();
		S.read_cnf(processor.getCNF(), processor.maxVar());
	} else {
		solving_begin_time = cpuTime();
		S.read_cnf(in);
	}
	in.close();
	S.solve();
	return 0;
}
