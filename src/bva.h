#include <cstdint>
#include <limits.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <cassert>
#include <fstream>
#include <string>
#include <sstream>
#include "utils.h"
#include "proof.h"

using namespace std;

namespace BVA
{

typedef vector<int> Clause;
typedef vector<Clause *> Occs;

// Overload the << operator for Clause
std::ostream &operator<<(std::ostream &os, const Clause &c);

class AutomatedReencoder
{
private:
    ProofTracer *proof;
    vector<Clause *> cnf;
    Clause imported_clause;
    unsigned size_vars;
    vector<signed char> marks;
    vector<char> seen;
    vector<Occs> otab;
    int max_var;

private:
    int vidx(int lit) const;
    unsigned vlit(int lit) const;
    Occs &occs(int lit);
    const Occs &occs(int lit) const;
    void enlarge_marks(int64_t idx);
    signed char marked(int lit) const;
    void mark(int lit);
    void unmark(int lit);
    bool tautological(const vector<int> &c);
    int getLeastOccurring(Clause *, int);
    int getSingleLiteralDifference(Clause *, Clause *);
    int reduction(const vector<int> &, const vector<Clause *> &) const;
    bool reductionIncreases(const vector<int> &, const vector<Clause *> &, int) const;
    bool isReplaceableMatch(const vector<int> &, const vector<Clause *> &) const;
    int introduceNewVariable();
    bool clausesAreIdentical(const Clause &, const Clause &);
    bool unary(const Clause *) const;

public:
    AutomatedReencoder(ProofTracer *);
    int num_occs(int) const;
    void applySimpleBVA();
    void readCNF(std::ifstream &);
    const vector<Clause *> &getCNF() const;
    int maxVar() const;
    void dump() const;
};
};