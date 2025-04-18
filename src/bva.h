#include <queue>
#include <set>
#include <cstdint>
#include <limits.h>
#include <initializer_list>
#include <unordered_map>
#include <unordered_set>
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
    class Clause
    {
    public:
        Clause() : active(true) {}
        Clause(initializer_list<int> init)
            : literals(init), active(true) {}
        Clause(const vector<int> &init)
            : literals(init), active(true) {}
        int size() const { return literals.size(); }
        void push_back(int lit) { literals.push_back(lit); }
        std::vector<int> literals;
        bool active;
        std::vector<int>::iterator begin() { return literals.begin(); }
        std::vector<int>::iterator end() { return literals.end(); }
        std::vector<int>::const_iterator begin() const { return literals.cbegin(); }
        std::vector<int>::const_iterator end() const { return literals.cend(); }
        int& operator[](std::size_t i) { return literals[i]; }
        const int& operator[](std::size_t i) const { return literals[i]; }
    };

    typedef vector<Clause *> Occs;
    typedef unordered_map<int, vector<Clause*>> LitMap;

    // Overload the << operator for Clause
    std::ostream &operator<<(std::ostream &os, const Clause &c);

    struct ClauseHasher {
        size_t operator()(const Clause *c) const {
            size_t h = 0;
            assert(c);
            for (int lit : *c)
                h ^= std::hash<int>{}(lit);
            return h;
        }
    };

    class AutomatedReencoder
    {
    private:
        ProofTracer *proof;
        vector<int> imported_clause;
        unsigned size_vars;
        vector<signed char> marks;
        vector<char> seen;
        vector<Occs> otab;
        int max_var, max_iterations;
        struct
        {
            int64_t added, deleted, aux_vars;
        } stats;

        unordered_set<Clause *, ClauseHasher> cnf;

    private:
        int vidx(int lit) const;
        unsigned vlit(int lit) const;
        Occs &occs(int lit);
        const Occs &occs(int lit) const;
        void enlarge_marks(int64_t idx);
        signed char marked(int lit) const;
        void mark(int lit);
        void unmark(int lit);
        bool tautological(const Clause &c);
        Clause* find(Clause &c);
        bool existsInLitMap(const LitMap &, int, Clause &);
        int getLeastOccurring(Clause *, int);
        int getSingleLiteralDifference(Clause *, Clause *);
        bool reductionIncreases(const LitMap &, const set<int> &, const vector<Clause *> &, int) const;
        int introduceNewVariable();
        bool clausesAreIdentical(const Clause &, const Clause &);
        bool unary(const Clause *) const;
        Clause *newClause(priority_queue<pair<size_t, int>> &, Clause *);
        void removeClause(priority_queue<pair<size_t, int>> &, Clause &, vector<Clause *> &);
        void popExpiredElementsFromHeap(priority_queue<pair<size_t, int>> &);
        void dumpCNF() const;
        void dumpOccurrences() const;
        void dumpReplacebleMatching(const set<int> &M_lit, const vector<Clause *> &M_cls) const;
        void dumpLitMap(const LitMap &P_cls) const;

    public:
        AutomatedReencoder(ProofTracer *);
        ~AutomatedReencoder();
        int num_occs(int) const;
        void applySimpleBVA();
        void readCNF(std::ifstream &);
        const unordered_set<Clause *, ClauseHasher> &getCNF() const { return cnf; }
        int maxVar() const;
        void setIterations(int);
        void writeDimacsCNF(const char *) const;
    };
};