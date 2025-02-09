#pragma once
#include <string>
using namespace std;

enum OptionType {
	BOOL_OPTION,
	INT_OPTION,
	DOUBLE_OPTION,
	STRING_OPTION
};

class option {
public:
	option(string _msg) : msg(_msg) {};	
	string msg;
	virtual bool parse(string) = 0; 
	virtual string val() = 0;
	virtual OptionType type() = 0;
	virtual string type_error(string val) = 0;
};

class intoption : public option {
public:	intoption(int* p, int _lb, int _ub, string _msg) : option(_msg),
	p_to_var(p), lb(_lb), ub(_ub) {};
	  int* p_to_var; // pointer to the variable holding the option value. 
	  int lb; // lower-bound
	  int ub; // upper-bound
	  bool parse(string st); 
	  string val() { return to_string(*p_to_var); }
	  OptionType type() { return INT_OPTION; }
	  string type_error(string val) { return "value " + val + " not numeric"; }
};

class booloption : public intoption {
public:	booloption(int* p, string _msg) : intoption((int*)p, 0, 1, _msg) {};
	  bool parse(string st) override;
	  string val() override { return *(p_to_var) ? "true" : "false"; }
	  OptionType type() override { return BOOL_OPTION; }
	  string type_error(string val) override { return "value " + val + " not boolean"; }
};

class doubleoption : public option {
public:	doubleoption(double* p, double _lb, double _ub, string _msg) : option(_msg),
	p_to_var(p), lb(_lb), ub(_ub) {}
	  double* p_to_var; // pointer to the variable holding the option value. 
	  double lb; // lower-bound
	  double ub; // upper-bound
	  bool parse(string st);
	  string val() { return to_string(*p_to_var); }
	  OptionType type() { return DOUBLE_OPTION; }
	  string type_error(string val) { return "value " + val + " not double"; }
};

class stringoption : public option {
public:	stringoption(string* p, string _msg) : option(_msg), p_to_var(p) {}
	  string* p_to_var; // pointer to the variable holding the option value. 
	  bool parse(string st);
	  string val() { return *p_to_var; }
	  OptionType type() { return STRING_OPTION; }
	  string type_error(string val) { return "value " + val + " not string"; }
};
void Abort(string s, int i);
void parse_options(int argc, char** argv);
