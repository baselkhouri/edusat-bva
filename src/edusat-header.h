#pragma once
#include <iostream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <set>
#include <string>
#include <sstream>  
#include <fstream>
#include <cassert>
#include <ctime>

using namespace std;

// ================== CpuTime ==================
#ifdef _MSC_VER
#include <ctime>

inline double cpuTime(void) {
    return (double)clock() / CLOCKS_PER_SEC; }
#else

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

inline double cpuTime(void) {
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    return (double)ru.ru_utime.tv_sec + (double)ru.ru_utime.tv_usec / 1000000; }
#endif

// ================== Typedefs ==================

typedef int Var;
typedef int Lit;
typedef vector<Lit> clause_t;
typedef clause_t::iterator clause_it;
typedef clause_t::const_iterator clause_cit;
typedef vector<Lit> trail_t;

// ================== Defines ==================

#define Assert(exp) AssertCheck(exp, __func__, __LINE__)
#define Neg(l) (l & 1)
#define Restart_multiplier 1.1f
#define Restart_lower 100
#define Restart_upper 1000
#define Max_bring_forward 10
#define var_decay 0.99
#define Rescale_threshold 1e100
#define Assignment_file "assignment.txt"

// ================== Enums ==================

enum class VAR_DEC_HEURISTIC {
	MINISAT
	// add other decision heuristics here. Add an option to choose between them.
 };

 enum class VAL_DEC_HEURISTIC {
	/* Same as last value. Initially false*/
	PHASESAVING, 
	/* Choose literal with highest frequency */
	LITSCORE 
};

enum class LitState {
	L_UNSAT,
	L_SAT,
	L_UNASSIGNED
};

enum class VarState {
	V_FALSE,
	V_TRUE,
	V_UNASSIGNED
};

enum class ClauseState {
	C_UNSAT,
	C_SAT,
	C_UNIT,
	C_UNDEF
};

enum class SolverState{
	UNSAT,
	SAT,
	CONFLICT, 
	UNDEF,
	TIMEOUT
};

// ================== Global Variables ==================
extern int verbose;
extern int preprocess;
extern string proof_path;
extern double timeout;
extern double begin_time;
extern double read_cnf_time;
extern double preprocess_time;
extern VAR_DEC_HEURISTIC VarDecHeuristic;
extern VAL_DEC_HEURISTIC ValDecHeuristic;

// ================== Functions ==================

inline unsigned int Abs(int x) { // because the result is compared to an unsigned int. unsigned int are introduced by size() functions, that return size_t, which is defined to be unsigned. 
	if (x < 0) return (unsigned int)-x;
	else return (unsigned int)x;
}

inline unsigned int v2l(int i) { // maps a literal as it appears in the cnf to literal
	if (i < 0) return ((-i) << 1) - 1; 
	else return i << 1;
} 

inline Var l2v(Lit l) {
	return (l+1) / 2;	
} 

inline Lit negate(Lit l) {
	if (Neg(l)) return l + 1;  // odd
	return l - 1;		
}

inline int l2rl(int l) {
	return Neg(l)? -((l + 1) / 2) : l / 2;
}

inline LitState lit_state(Lit l, VarState var_state) {
    return var_state == VarState::V_UNASSIGNED ? LitState::L_UNASSIGNED : ((Neg(l) && var_state == VarState::V_FALSE) || (!Neg(l) && var_state == VarState::V_TRUE)) ? LitState::L_SAT : LitState::L_UNSAT;
}

inline bool verbose_now() {
	return verbose > 1;
}

// ================== Errors and Assertions ==================
inline void Abort(string s, int i) {
	cout << endl << "Abort: ";
	switch (i) {
	case 1: cout << "(input error)" << endl; break;
	case 2: cout << "command line arguments error" << endl; break;
	case 3: break;
	default: cout << "(exit code " << i << ")" << endl; break;
	}
	cout << s << endl;
	exit(i);
}

// For production wrap with #ifdef _DEBUG
inline void AssertCheck(bool cond, string func_name, int line, string msg = "") {
	if (cond) return;
	cout << "Assertion fail" << endl;
	cout << msg << endl;
	cout << func_name << " line " << line << endl;
	exit(1);
}