#include "bva.h"
#include <utility> // for pair

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

    bool AutomatedReencoder::reductionIncreases(const vector<pair<int, Clause *>> &P_cls, const set<int> &M_lit, const vector<Clause *> &M_cls, int lit) const
    {
        auto M_litt(M_lit);
        M_litt.insert(lit);
        const int new_red = (M_litt.size()) * P_cls.size() - M_litt.size() - P_cls.size();
        const int old_red = M_lit.size() * M_cls.size() - M_lit.size() - M_cls.size();
        // cout << "new_red = " << new_red << " old_red " << old_red << endl;
        return new_red > old_red && new_red > 0;
    }

    bool AutomatedReencoder::isReplaceableMatch(const vector<int> &M_lit, const vector<Clause *> &M_cls) const
    {
        assert(0 && "Needs to be implemented");
    }

    int AutomatedReencoder::introduceNewVariable()
    {
        int x = ++max_var;
        if (x > size_vars)
            enlarge_marks(x);
        for (int i = otab.size() - 1; i < 2 * size_vars + 2; i++)
            otab.push_back(Occs());
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
        Clause *c = new Clause(lits);
        assert(c);
        if (proof)
            proof->notify_added_clause(*c, false /*learnt*/);
        for (int lit : *c)
        {
            occs(lit).push_back(c);
            Q.push({occs(lit).size(), lit});
        }
        cnf.push_back(c);
        return c;
    }

    Clause *AutomatedReencoder::removeClause(priority_queue<pair<size_t, int>> &Q, const vector<int> &lits)
    {
        vector<Clause *> to_delete;

        // Remove from cnf
        const auto end = cnf.end();
        auto i = cnf.begin();
        for (auto j = i; j != end; j++)
        {
            Clause *d = *i++ = *j;
            if (clausesAreIdentical(*d, lits))
            {
                i--;
                if (proof)
                    proof->notify_deleted_clause(*d);
                to_delete.push_back(d);
            }
        }
        assert(i + 1 == end); // can be relaxed?
        cnf.resize(i - cnf.begin());

        // Remove from occs
        for (Clause *c : to_delete)
        {
            assert(c);
            for (int lit : *c)
            {
                auto &os = occs(lit);
                const auto end = os.end();
                auto i = os.begin();
                for (auto j = i; j != end; j++)
                {
                    const Clause *d = *i++ = *j;
                    if (c == d)
                        i--;
                }
                assert(i + 1 == end);
                os.resize(i - os.begin());
                Q.push({occs(lit).size(), lit});
            }
        }

        assert(to_delete.size() == 1);
        return to_delete[0];
    }

    void AutomatedReencoder::popExpiredElementsFromHeap(priority_queue<pair<size_t, int>> &Q)
    {
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
    }

    AutomatedReencoder::AutomatedReencoder(ProofTracer *t) : proof(t), size_vars(0), max_var(0) {}

    int AutomatedReencoder::num_occs(int a) const { return occs(a).size(); }

    // TODO:
    // 1) Consider converting P intro a priority queue
    // 2) Consider changing the clauses databse to achieve efficient removal of clauses
    // 3) Should we propagate all unit clauses and assume no units?
    // 4) Consider directly updating elements in the queue
    // 5) Deallocating the clauses in the destructor is missing!
    // 6) Consider fixing the method ::reductionIncreases
    void AutomatedReencoder::applySimpleBVA()
    {
        TIME_BLOCK("[BVA] Simple Bounded Variable Addition");

        {
            TIME_BLOCK("[BVA] Building occurrences list");
            assert(max_var <= size_vars);
            otab.resize(2 * size_vars + 2, Occs());
            for (Clause *c : cnf)
            {
                assert(c);
                for (int lit : *c)
                    occs(lit).push_back(c);
            }
        }

        priority_queue<pair<size_t, int>> Q;

        {
            TIME_BLOCK("[BVA] Building prority queue");
            for (int lit = -max_var; lit <= max_var; ++lit)
            {
                if (lit == 0)
                    continue;
                size_t occCount = occs(lit).size();
                if (occCount)
                    Q.push({occCount, lit});
            }
        }

        // Main algorithm loop
        while (!Q.empty())
        {
            popExpiredElementsFromHeap(Q);
            if (Q.empty())
                break;
            auto [occCount, l] = Q.top();
            Q.pop();
            const auto &F_l = occs(l);
            assert(occCount == F_l.size());

            // cout << "l = " << l << endl;

            vector<Clause *> M_cls(F_l);
            set<int> M_lit = {l};

        label1:
            vector<pair<int, Clause *>> P = {};
            assert(P.empty());

            // cout << "M_lit = ";
            // for (int lit : M_lit)
            //     cout << lit << " ";
            // cout << endl;
            // cout << "M_cls = {";
            // for (const auto &c : M_cls)
            //     cout << "   " << *c << endl;
            // cout << "}" << endl;

            // Lines 5-10
            for (const auto &c : M_cls)
            {
                if (unary(c))
                    continue;
                int l_min = getLeastOccurring(c, l);
                // cout << "l_min = " << l_min << endl;
                assert(l_min && l_min != l);
                const auto &F_l_min = occs(l_min);
                // cout << "F_l_min = {" << endl;
                // for (const auto &c : F_l_min)
                //     cout << "   " << *c << endl;
                // cout << "}" << endl;
                // Lines 7-10
                for (const auto &d : F_l_min)
                    if (l == getSingleLiteralDifference(c, d))
                    {
                        int l_ = getSingleLiteralDifference(d, c);
                        assert(l_);
                        P.push_back({l_, c});
                    }
                // cout << "P = {" << endl;
                // for (const auto &p : P)
                //     cout << "   <" << p.first << ", " << *p.second << ">" << endl;
                // cout << "}" << endl;
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

                // cout << "l_max = " << l_max << endl;

                // Lines 12-16
                if (reductionIncreases(P, M_lit, M_cls, l_max))
                {
                    // cout << "REDUCTION INCREASES!" << endl;
                    M_lit.insert(l_max);
                    M_cls.clear();
                    for (const auto &p : P)
                        if (p.first == l_max)
                            M_cls.push_back(p.second);
                    goto label1;
                }
            }
            else
            {
                // cout << "P is empty" << endl;
            }

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
                    to_deallocate.push_back(removeClause(Q, d));
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
                delete to_deallocate[i];

            Q.push({occs(l).size(), l});
            Q.push({occs(-x).size(), -x});
            Q.push({occs(x).size(), x});
        }
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
            Clause clause;

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
            if (!tautological(clause))
                cnf.push_back(new Clause(imported_clause));
            else
                num_tautologies++;
        }

        assert(pLineFound);
        assert(max_var == numVariables);                      // Can be relaxed
        assert((cnf.size() + num_tautologies) == numClauses); // Can be relaxed

        cout << num_tautologies << " tautological clauses has been found" << endl;
    }

    const vector<Clause *> &AutomatedReencoder::getCNF() const { return cnf; }

    int AutomatedReencoder::maxVar() const { return max_var; }

    void AutomatedReencoder::dump() const
    {
        TIME_BLOCK("[BVA] dumping db");
        cout << "clauses: " << endl;
        for (Clause *c : cnf)
            cout << *c << endl;
        cout << "occs: " << endl;
        for (int v = 1; v <= max_var; v++)
        {
            const auto &os = occs(v);
            cout << "   " << v << ':' << endl;
            for (Clause *c : os)
                cout << "       " << *c << endl;
        }
    }

};