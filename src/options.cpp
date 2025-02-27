#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include "options.h"


void Abort(string s, int i) {
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

template<typename T> bool Tparse(string st, T val, T lb, T ub, T* var) {
	if (val < lb || val > ub)
		Abort(st + string(" value not in range"), 2);
	*var = val;	
	return true;
}

bool booloption::parse(string st){
	return intoption::parse(st);
}

bool intoption::parse(string st) {
	int val;
	try { val = stoi(st); }
	catch (...) {Abort(type_error(st), 1); }
	return Tparse<int>(st, val, lb, ub, p_to_var);
}

bool doubleoption::parse(string st) {
	double val;
	try { val = stod(st); }
	catch (...) { Abort(type_error(st), 1); }
	return Tparse<double>(st, val, lb, ub, p_to_var);
}

bool stringoption::parse(string st) {
	int int_val;
	double double_val;

	try {
		size_t pos;
		int_val = stoi(st, &pos);
		if (pos == st.size()) {
			// If it's an int, abort
			Abort(type_error(st), 1);
		}
	}
	catch (...) {
		// Do nothing
	}

	try {
		size_t pos;
		double_val = stod(st, &pos);
		if (pos == st.size()) {
			// If it's a double, abort
			Abort(type_error(st), 1);
		}
	}
	catch (...) {
		// Do nothing
	}

	*p_to_var = st;
	return true;
}

extern unordered_map<string, option*> options;

void help() {
	stringstream st;
	st << "\nUsage: edusat <options> <file name>\n \n"
		"Options:\n";
	for (auto h : options) {
		option* opt = options[h.first];
		st << left << setw(16) << "-" + h.first;
		st << opt->msg << ". Default: " << opt->val() << "." << endl;
	}
	Abort(st.str(), 3);
}

void parse_options(int argc, char** argv) {
	// Look for -h through all argv
	// Only -h is allowed to appear anywhere in the command line
	for (int i = 1; i < argc; ++i) {
		if (string(argv[i]).compare("-h") == 0) {
			help();
		}
	}
	for (int i = 1; i < argc - 1; ++i) {
		string st = argv[i] + 1;
		if (argv[i][0] != '-' || options.count(st) == 0) {
			cout << st << endl;
			Abort("Unknown flag ", 2);
		}
		if (i == argc - 2 || argv[i + 1][0] == '-') {
			if (options[st]->type() == BOOL_OPTION) {
				// If it's a boolean and no value is given, set it to true
				options[st]->parse("1");
				continue;
			}
			else
				Abort(string("missing value after ") + st, 2);
		}
		
		i++;
		options[st]->parse(argv[i]);
	}
}