#include "bva.h"
#include "profiler.h"
#include <utility> // for pair

// #define DEBMSG

#ifdef DEBMSG
#define DEBUG_MSG(cmd) \
    do                 \
    {                  \
        cmd;           \
    } while (0)
#else
#define DEBUG_MSG(cmd) \
    do                 \
    {                  \
    } while (0)
#endif

namespace BVA
{

    // Overload the << operator for Clause
    ostream &operator<<(ostream &os, const Clause &c)
    {
        os << "[";
        for (size_t i = 0; i < c.size(); ++i)
        {
            if (i > 0)
                os << " ";
            os << c[i];
        }
        os << "]";
        return os;
    }

    int AutomatedReencoder::vidx(int lit) const
    {
        int idx;
        assert(lit);
        assert(lit != INT_MIN);
        idx = abs(lit);
        assert(idx <= max_var);
        return idx;
    }

    unsigned AutomatedReencoder::vlit(int lit) const { return (lit < 0) + 2u * (unsigned)vidx(lit); }

    Occs &AutomatedReencoder::occs(int lit)
    {
        assert(vlit(lit) < otab.size());
        return otab[vlit(lit)];
    }

    const Occs &AutomatedReencoder::occs(int lit) const
    {
        assert(vlit(lit) < otab.size());
        return otab[vlit(lit)];
    }

    void AutomatedReencoder::enlarge_marks(int64_t idx)
    {
        assert(0 < idx), assert(idx <= INT_MAX);
        assert(idx > max_var);
        int64_t new_size_vars = size_vars ? 2 * size_vars : 2;
        while (idx >= new_size_vars)
            new_size_vars *= 2;

        marks.resize(2 * new_size_vars);
        seen.resize(2 * new_size_vars);

        assert(idx < new_size_vars);
        size_vars = new_size_vars;
    }

    signed char AutomatedReencoder::marked(int lit) const
    {
        assert(abs(lit) < marks.size());
        signed char res = marks[abs(lit)];
        if (lit < 0)
            res = -res;
        return res;
    }

    void AutomatedReencoder::mark(int lit)
    {
        assert(!marked(lit));
        marks[abs(lit)] = (lit > 0) - (lit < 0);
        assert(marked(lit) > 0);
        assert(marked(-lit) < 0);
    }

    void AutomatedReencoder::unmark(int lit)
    {
        assert(abs(lit) < marks.size());
        marks[abs(lit)] = 0;
        assert(!marked(lit));
    }

    bool AutomatedReencoder::tautological(const vector<int> &c)
    {
        imported_clause.clear();
        unsigned idx;
        for (int lit : c)
        {
            idx = abs(lit);
            if (idx >= size_vars)
                enlarge_marks(idx);
        }
        bool imported_tautological = false;
        int tmp;
        for (int lit : c)
        {
            tmp = marked(lit);
            if (tmp < 0)
                imported_tautological = true;
            else if (!tmp)
            {
                imported_clause.push_back(lit);
                mark(lit);
            }
        }
        for (const auto &lit : c)
            unmark(lit);
        return imported_tautological;
    }

    bool AutomatedReencoder::duplicate(vector<int> &lits)
    {
        Clause c(lits);
        // Find the specific bucket in the hash table
        size_t bucketIndex = cnf.bucket(&c);

        // Iterate over all elements in the specific bucket to find the clause
        // Do not use .find() as it relies on pointer equality
        for (auto it = cnf.begin(bucketIndex); it != cnf.end(bucketIndex); it++)
            if (clausesAreIdentical(**it, c))
                return true;
        return false;
    }

    // Returns the last least occuring literal in c that is not 'other'.
    // In case the clause is unary, returns 0.
    int AutomatedReencoder::getLeastOccurring(Clause *c, int other)
    {
        assert(c && c->size() && other);
        if (c->size() == 1)
            return 0;
        int res_lit, res_occCount = INT_MAX;
        for (int lit : *c)
        {
            if (other == lit)
                continue;
            int occCount = occs(lit).size();
            assert(occCount >= 1);
            if (occCount > res_occCount)
                continue;
            res_lit = lit;
            res_occCount = occCount;
        }
        assert(res_lit);
        return res_lit;
    }

    // Returns a literal l in c that is not in d in case both clauses differ only
    // in one lit. Otherwise, returns 0.
    int AutomatedReencoder::getSingleLiteralDifference(Clause *c, Clause *d)
    {
        assert(c && d);
        if (c->size() != d->size())
            return 0;
        for (int lit : *d)
        {
            assert(!marked(lit));
            mark(lit);
        }
        int diffs = 0, diff_lit = 0;
        for (int lit : *c)
        {
            if (marked(lit) > 0)
                continue;
            diffs++;
            diff_lit = lit;
        }
        for (int lit : *d)
            unmark(lit);
        return diffs > 1 ? 0 : diff_lit;
    }

    bool AutomatedReencoder::reductionIncreases(const LitMap &P_cls, const set<int> &M_lit, const vector<Clause *> &M_cls, int lit) const
    {
        const int old_red = M_lit.size() * M_cls.size() - M_lit.size() - M_cls.size();

        auto M_litt(M_lit);
        M_litt.insert(lit);
        // Count only relevant clauses in P_cls
        int P_relevant = P_cls.at(lit).size();
        assert(P_relevant);
        const int new_red = (M_litt.size()) * P_relevant - M_litt.size() - P_relevant;
        DEBUG_MSG(cout << "new_red = " << new_red << " old_red " << old_red << endl);
        return new_red > old_red && new_red > 0;
    }

    int AutomatedReencoder::introduceNewVariable()
    {
        int x = ++max_var;
        if (x > size_vars)
            enlarge_marks(x);
        for (int i = otab.size() - 1; i < 2 * size_vars + 2; i++)
            otab.push_back(Occs());
        stats.aux_vars++;
        return x;
    }

    bool AutomatedReencoder::clausesAreIdentical(const Clause &c, const Clause &d)
    {
        bool identical = true;
        for (int lit : c)
            mark(lit);
        for (int lit : d)
            identical &= (marked(lit) > 0);
        for (int lit : c)
            unmark(lit);
        return identical;
    }

    bool AutomatedReencoder::unary(const Clause *c) const
    {
        assert(c);
        return c->size() == 1;
    }

    Clause *AutomatedReencoder::newClause(priority_queue<pair<size_t, int>> &Q, const vector<int> &lits)
    {
        PROFILER_START(new_clause);
        Clause *c = new Clause(lits);
        assert(c);
        if (proof)
            proof->notify_added_clause(c->literals, false /*learnt*/);
        for (int lit : *c)
        {
            occs(lit).push_back(c);
            Q.push({occs(lit).size(), lit});
        }
        auto [it, inserted] = cnf.insert(c);
        if (!inserted)
        {
            // Clause already exists
            delete c;
            return *it;
        }
        stats.added += 1;
        PROFILER_STOP(new_clause);
        return c;
    }

    void AutomatedReencoder::removeClause(priority_queue<pair<size_t, int>> &Q, Clause &c, vector<Clause *> &to_deallocate)
    {
        PROFILER_START(remove_clause);
        int deleted = 0;
        // Find the specific bucket in the hash table
        size_t bucketIndex = cnf.bucket(&c);

        // Iterate over all elements in the specific bucket to find the clause
        // Do not use .find() as it relies on pointer equality
        for (auto it = cnf.begin(bucketIndex); it != cnf.end(bucketIndex); it++)
        {
            if (clausesAreIdentical(**it, c))
            {
                Clause *clause_to_delete = *it;
                assert(clause_to_delete->active);
                to_deallocate.push_back(clause_to_delete);
                deleted++;
                clause_to_delete->active = false;
                assert(clause_to_delete);
            }
        }

        // Remove from occs
        for (int lit : c)
        {
            auto &os = occs(lit);
            const auto end = os.end();
            auto i = os.begin();
            for (auto j = i; j != end; j++)
            {
                const Clause *d = *i++ = *j;
                if (!d->active)
                    i--;
            }
            assert(i + 1 == end);
            os.resize(i - os.begin());
            Q.push({occs(lit).size(), lit});
        }

        stats.deleted += deleted;
        PROFILER_STOP(remove_clause);
    }

    void AutomatedReencoder::popExpiredElementsFromHeap(priority_queue<pair<size_t, int>> &Q)
    {
        PROFILER_START(pop_expired_elements);
        // Now, since occs have updated for some literals without
        // updating their place in the heap, we ensure the next
        // variable to pop is stale!
        while (!Q.empty())
        {
            auto [storedOcc, storedLit] = Q.top();
            // Check if it's stale
            if (storedOcc == occs(storedLit).size())
            {
                // It's valid
                break;
            }
            else
            {
                // It's stale -- pop and ignore
                Q.pop();
            }
        }
        PROFILER_STOP(pop_expired_elements);
    }

    AutomatedReencoder::AutomatedReencoder(ProofTracer *t) : proof(t),
                                                             size_vars(0),
                                                             max_var(0),
                                                             max_iterations(10000000),
                                                             cnf(0, ClauseHasher())
    {
        ADD_PROFILER_OPERATION(preprocessing);
        ADD_PROFILER_OPERATION(build_occs_list);
        ADD_PROFILER_OPERATION(build_priority_queue);
        ADD_PROFILER_OPERATION(pop_expired_elements);
        ADD_PROFILER_OPERATION(remove_clause);
        ADD_PROFILER_OPERATION(new_clause);
        memset(&stats, 0, sizeof(stats));
    }

    AutomatedReencoder::~AutomatedReencoder()
    {
        for (Clause *c : cnf)
            delete c;
        cnf.clear();
        Profiler::getInstance().printAllStatistics();
    }

    int AutomatedReencoder::num_occs(int a) const { return occs(a).size(); }

    // TODO:
    // 1) <<MINOR>> Consider converting P intro a priority queue
    // 2) <<MAJOR>> Consider changing the clauses databse to a hash table instead of vector to achieve efficient removal of clauses
    // 3) <<INFO>> Should we propagate all unit clauses and assume no units? The answer seems to be NO...
    // 4) <<MINOR>> Consider directly updating elements in the queue
    // 5) <<MINOR>> Consider fixing the method ::reductionIncreases
    void AutomatedReencoder::applySimpleBVA()
    {
        TIME_BLOCK("[BVA] Simple Bounded Variable Addition");
        PROFILER_START(preprocessing);

        // if (proof)
        //     proof->notify_comment("Applying Simple Bounded Addition Algorithm:");

        {
            TIME_BLOCK("[BVA] Building occurrences list");
            PROFILER_START(build_occs_list);
            assert(max_var <= size_vars);
            otab.resize(2 * size_vars + 2, Occs());
            for (Clause *c : cnf)
            {
                assert(c);
                for (int lit : *c)
                    occs(lit).push_back(c);
            }
            PROFILER_STOP(build_occs_list);
        }

        priority_queue<pair<size_t, int>> Q;

        {
            TIME_BLOCK("[BVA] Building prority queue");
            PROFILER_START(build_priority_queue);
            for (int lit = -max_var; lit <= max_var; ++lit)
            {
                if (lit == 0)
                    continue;
                size_t occCount = occs(lit).size();
                if (occCount)
                    Q.push({occCount, lit});
            }
            PROFILER_STOP(build_priority_queue);
        }

        int iteration = 0;
        // Main algorithm loop
        while (!Q.empty())
        {
            // Check if maximum iteration limit has been reached
            if (++iteration > max_iterations)
                break;
            UPDATE_PROGRESS("[BVA] Iteration " + to_string(iteration));

            // Ensure the next element is not stale
            popExpiredElementsFromHeap(Q);
            if (Q.empty())
                break;

            auto [occCount, l] = Q.top();
            Q.pop();
            const auto &F_l = occs(l);
            assert(occCount == F_l.size());

            DEBUG_MSG(cout << "l = " << l << endl);

            vector<Clause *> M_cls(F_l);
            set<int> M_lit = {l};

        label1:
            LitMap P = {};
            assert(P.empty());

            DEBUG_MSG(cout << "M_lit = ";
                      for (int lit : M_lit)
                          cout
                      << lit << " ";
                      cout << endl;
                      cout << "M_cls = {" << endl;
                      for (const auto &c : M_cls)
                          cout
                      << "   " << *c << endl;
                      cout << "}" << endl;);

            // Lines 5-10
            for (const auto &c : M_cls)
            {
                if (unary(c))
                    continue;
                int l_min = getLeastOccurring(c, l);
                assert(l_min && l_min != l);
                const auto &F_l_min = occs(l_min);
                DEBUG_MSG(cout << "l_min = " << l_min << endl;
                          cout << "F_l_min = {" << endl;
                          for (const auto &c : F_l_min)
                              cout
                          << "   " << *c << endl;
                          cout << "}" << endl;);
                // Lines 7-10
                for (const auto &d : F_l_min)
                    if (l == getSingleLiteralDifference(c, d))
                    {
                        int l_ = getSingleLiteralDifference(d, c);
                        assert(l_);
                        P[l_].push_back(c);
                    }
                DEBUG_MSG(
                    cout << "P = {\n";
                    for (const auto &entry : P) {
                        cout << "   " << entry.first << " -> { ";
                        for (const Clause *clause : entry.second)
                        {
                            cout << *clause << " ";
                        }
                        cout << "}\n";
                    } cout
                    << "}\n";);
            }

            if (P.size())
            {
                // Line 11
                int l_max = 0, l_max_occs = 0;
                for (const auto &p : P)
                    if (occs(p.first).size() > l_max_occs)
                    {
                        l_max = p.first;
                        l_max_occs = occs(p.first).size();
                    }
                assert(l_max);

                DEBUG_MSG(cout << "l_max = " << l_max << endl;);

                // Lines 12-16
                if (reductionIncreases(P, M_lit, M_cls, l_max))
                {
                    DEBUG_MSG(cout << "REDUCTION INCREASES!" << endl;);
                    M_lit.insert(l_max);
                    M_cls.clear();
                    M_cls.insert(M_cls.end(), P[l_max].begin(), P[l_max].end());
                    goto label1;
                }
            }
            else
                DEBUG_MSG(cout << "P is empty" << endl;);

            // Line 17
            if (M_lit.size() == 1)
                continue;

            vector<Clause *> to_deallocate;
            // Lines 18-22
            int x = introduceNewVariable();

            for (int l_ : M_lit)
            {
                newClause(Q, {l_, x});
                for (Clause *c : M_cls)
                {
                    Clause d = {l_};
                    for (int ll : *c)
                        if (ll != l)
                            d.push_back(ll);
                    removeClause(Q, d, to_deallocate);
                }
            }

            // Lines 23-24
            for (const auto &c : M_cls)
            {
                vector<int> d = {-x};
                for (int lit : *c)
                    if (lit != l)
                        d.push_back(lit);
                newClause(Q, d);
            }

            for (int i = 0; i < to_deallocate.size(); i++)
            {
                Clause *d = to_deallocate[i];
                assert(!d->active);
                if (proof)
                    proof->notify_deleted_clause(d->literals);
                cnf.erase(d);
                delete d;
            }

            Q.push({occs(l).size(), l});
            Q.push({occs(-x).size(), -x});
            Q.push({occs(x).size(), x});
        }

        if (iteration > max_iterations)
            cout
                << " -> Reached max iterations limit" << endl;
        else
            cout
                << " -> Algorithm ended" << endl;
        cout << "[BVA] Statistics:" << endl;
        cout << "[BVA]    " << stats.added << " clauses added" << endl;
        cout << "[BVA]    " << stats.deleted << " clauses deleted" << endl;
        cout << "[BVA]    " << stats.deleted - stats.added << " clauses reduced in total" << endl;
        cout << "[BVA]    " << stats.aux_vars << " auxiliary variables used" << endl;
        if (proof)
        {
            proof->notify_comment("    " + to_string(stats.added) + " clauses added");
            proof->notify_comment("    " + to_string(stats.deleted) + " clauses deleted");
            proof->notify_comment("    " + to_string(stats.aux_vars) + " auxiliary variables used");
        }
        PROFILER_STOP(preprocessing);
    }

    void AutomatedReencoder::readCNF(ifstream &in)
    {
        TIME_BLOCK("[BVA] reading cnf");
        int64_t num_tautologies = 0;
        assert(cnf.empty());
        string line;
        bool pLineFound = false;
        int numVariables, numClauses;

        while (true)
        {
            // Attempt to read a line from the file
            if (!getline(in, line))
            {
                // No more lines to read; break from loop
                break;
            }

            // Trim whitespace from the line (optional)
            // (If you prefer, you can parse using stringstream directly.)

            // Skip empty lines
            if (line.empty())
                continue;

            // Skip comment lines that begin with 'c'
            if (line[0] == 'c')
                continue;

            // If it’s the problem line (e.g., "p cnf 10 25")
            if (line[0] == 'p')
            {
                assert(!pLineFound);
                stringstream ss(line);
                string tmp, format;
                ss >> tmp >> format >> numVariables >> numClauses;
                pLineFound = true;
                enlarge_marks(numVariables);
                max_var = numVariables;
                continue;
            }

            // Otherwise, we assume this line contains exactly one clause
            // terminated by 0.
            stringstream ss(line);
            vector<int> clause;

            int literal;
            while (ss >> literal)
            {
                if (literal == 0)
                {
                    // Reached end of this clause
                    break;
                }
                clause.push_back(literal);
                if (max_var < abs(literal))
                    max_var = abs(literal);
                if (size_vars < abs(literal))
                    enlarge_marks(abs(literal));
            }
            if (!tautological(clause) /* && !duplicate(clause) */)
                cnf.insert(new Clause(imported_clause));
            else
                num_tautologies++;
        }

        assert(pLineFound);
        assert(max_var == numVariables);                      // Can be relaxed
        assert((cnf.size() + num_tautologies) == numClauses); // Can be relaxed

        DEBUG_MSG(cout << num_tautologies << " tautological clauses has been found" << endl;);
    }

    int AutomatedReencoder::maxVar() const { return max_var; }

    void AutomatedReencoder::dumpCNF() const
    {
        TIME_BLOCK("[BVA] Dumping CNF");
        cout << "clauses: " << endl;
        for (Clause *c : cnf)
            cout << *c << endl;
        cout << "occs: " << endl;
    }
    void AutomatedReencoder::dumpOccurrences() const
    {
        TIME_BLOCK("[BVA] Dumping CNF");
        for (int v = -max_var; v <= max_var; v++)
        {
            if (v == 0)
                continue;
            const auto &os = occs(v);
            if (os.empty())
                continue;
            cout << "   " << v << ':' << endl;
            for (Clause *c : os)
                cout << "       " << *c << endl;
        }
    }

    void AutomatedReencoder::setIterations(int val)
    {
        assert(val >= 1);
        max_iterations = val;
    }

    void AutomatedReencoder::writeDimacsCNF(const char *fname) const
    {
        string msg = "[BVA] Processed CNF successfully written to " + string(fname);
        TIME_BLOCK(msg.c_str());
        // Decide whether to write to stdout or a file
        std::ostream *out = &std::cout;
        std::ofstream ofs;

        // If fname is not null and not an empty string,
        // then open the file for writing
        if (fname != nullptr && fname[0] != '\0')
        {
            ofs.open(fname);
            if (!ofs.is_open())
            {
                std::cerr << "Error: Could not open file '" << fname << "' for writing.\n";
                return;
            }
            out = &ofs; // redirect output to the file
        }

        // Compute the number of clauses
        const size_t numClauses = cnf.size();

        // Find the maximum variable index by scanning all literals
        int maxVar = 0;
        for (auto *clause : cnf)
        {
            if (!clause)
                continue;
            for (int lit : *clause)
            {
                int var = std::abs(lit);
                if (var > maxVar)
                {
                    maxVar = var;
                }
            }
        }

        // Print the "p cnf <maxVar> <numClauses>" line
        (*out) << "p cnf " << maxVar << " " << numClauses << "\n";

        // Print each clause in DIMACS format (literals followed by 0)
        for (auto *clause : cnf)
        {
            if (!clause)
                continue;
            for (int lit : *clause)
            {
                (*out) << lit << " ";
            }
            (*out) << "0\n";
        }

        // Close the file if we opened one
        if (ofs.is_open())
        {
            ofs.close();
        }
    }
};