#pragma once
#include "edusat-header.h"
#include "clause.h"
#include "bva.h"
#include "proof.h"

class Solver {
	vector<Clause> cnf; // clause DB. 
	vector<int> unaries; 
	trail_t trail;  // assignment stack	
	vector<int> separators; // indices into trail showing increase in dl 	
	vector<int> LitScore; // literal => frequency of this literal (# appearances in all clauses). 
	vector<vector<int> > watches;  // Lit => vector of clause indices into CNF
	vector<VarState> state;  // current assignment
	vector<VarState> prev_state; // for phase-saving: same as state, only that it is not reset to 0 upon backtracking. 
	vector<int> antecedent; // var => clause index in the cnf vector. For variables that their value was assigned in BCP, this is the clause that gave this variable its value. 
	vector<bool> marked;	// var => seen during analyze()
	vector<int> dlevel; // var => decision level in which this variable was assigned its value. 
	vector<int> conflicts_at_dl; // decision level => # of conflicts under it. Used for local restarts.

public:
	ProofTracer *proof_tracer;
	inline void set_proof_file(std::string f) {
		proof_tracer = new ProofDumper(f);
	}

private:
	// Used by VAR_DH_MINISAT:	
	map<double, unordered_set<Var>, greater<double>> m_Score2Vars; // 'greater' forces an order from large to small of the keys
	map<double, unordered_set<Var>, greater<double>>::iterator m_Score2Vars_it;
	unordered_set<Var>::iterator m_VarsSameScore_it;
	vector<double>	m_activity; // Var => activity
	double			m_var_inc;	// current increment of var score (it increases over time)
	double			m_curr_activity;
	bool			m_should_reset_iterators;

	unsigned int 
		nvars,			// # vars
		nclauses, 		// # clauses
		nlits,			// # literals = 2*nvars				
		qhead;			// index into trail. Used in BCP() to follow the propagation process.
	int					
		num_learned, 	
		num_decisions,
		num_assignments,
		num_restarts,
		dl,				// decision level
		max_dl,			// max dl seen so far since the last restart
		conflicting_clause_idx, // holds the index of the current conflicting clause in cnf[]. -1 if none.				
		restart_threshold,
		restart_lower,
		restart_upper;

	Lit 		asserted_lit;

	float restart_multiplier;
	
	// access	
	int get_learned() { return num_learned; }
	void set_nvars(int x) { nvars = x; }
	int get_nvars() { return nvars; }
	void set_nclauses(int x) { nclauses = x; }
	size_t cnf_size() { return cnf.size(); }
	VarState get_state(int x) { return state[x]; }

	// misc.
	void add_to_trail(int x) { trail.push_back(x); }

	void reset(); // initialization that is invoked initially + every restart
	void initialize();
	void reset_iterators(double activity_key = 0.0);	

	// solving	
	SolverState decide();
	void test();
	SolverState BCP();
	int  analyze(const Clause);
	inline int  getVal(Var v);
	inline void add_clause(Clause& c, int l, int r, bool original = false);
	inline void add_unary_clause(Lit l, bool original = false);
	inline void assert_lit(Lit l);	
	void m_rescaleScores(double& new_score);
	inline void backtrack(int k);
	void restart();
	
	// scores	
	inline void bumpVarScore(int idx);
	inline void bumpLitScore(int lit_idx);

public:
	Solver():
		proof_tracer(0),
		nvars(0), nclauses(0), num_learned(0), num_decisions(0), num_assignments(0), 
		num_restarts(0), m_var_inc(1.0), qhead(0), 
		restart_threshold(Restart_lower), restart_lower(Restart_lower), 
		restart_upper(Restart_upper), restart_multiplier(Restart_multiplier) {};
	~Solver() { delete proof_tracer; }
	void read_cnf(ifstream& in);
	void read_cnf(const unordered_set<BVA::Clause *, BVA::ClauseHasher> &cnf, const int max_var);
	VarState get_lit_state(int l) { return state[l2v(l)]; }
	SolverState _solve();
	void solve();

	ClauseState next_not_false(Clause &c, bool is_left_watch, Lit other_watch, bool binary, int& loc);
	
	// debugging
	void print_cnf(){
		for(vector<Clause>::iterator i = cnf.begin(); i != cnf.end(); ++i) {
			i -> print_with_watches(); 
			cout << endl;
		}
	} 

	void print_real_cnf() {
		for(vector<Clause>::iterator i = cnf.begin(); i != cnf.end(); ++i) {
			i -> print_real_lits(); 
			cout << endl;
		}
	} 

	void print_state(const char *file_name) {
		ofstream out;
		out.open(file_name);		
		for (vector<VarState>::iterator it = state.begin() + 1; it != state.end(); ++it) {
			char sign = (*it) == VarState::V_FALSE ? -1 : (*it) == VarState::V_TRUE ? 1 : 0;
			out << sign * (it - state.begin()) << " "; out << endl;
		}
	}	

	void print_state() {
		for (vector<VarState>::iterator it = state.begin() + 1; it != state.end(); ++it) {
			char sign = (*it) == VarState::V_FALSE ? -1 : (*it) == VarState::V_TRUE ? 1 : 0;
			cout << sign * (it - state.begin()) << " "; cout << endl;
		}
	}	
	
	void print_watches() {
		for (vector<vector<int> >::iterator it = watches.begin() + 1; it != watches.end(); ++it) {
			cout << distance(watches.begin(), it) << ": ";
			for (vector<int>::iterator it_c = (*it).begin(); it_c != (*it).end(); ++it_c) {
				cnf[*it_c].print();
				cout << "; ";
			}
			cout << endl;
		}
	};


	void print_stats() {cout << endl << "Statistics: " << endl << "===================" << endl << 
		"### Restarts:\t\t" << num_restarts << endl <<
		"### Learned-clauses:\t" << num_learned << endl <<
		"### Decisions:\t\t" << num_decisions << endl <<
		"### Implications:\t" << num_assignments - num_decisions << endl <<
		"### Preprocess Time:\t" << preprocess_time << endl <<
		"### Search Time:\t" << cpuTime() - solving_begin_time << endl <<
		"### Solve Time:\t\t" << cpuTime() - begin_time << endl;
	}
	
	void validate_assignment();
};


