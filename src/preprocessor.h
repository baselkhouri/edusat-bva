#pragma once

#include <vector>
#include <iostream>

#include "clause.h"

class Preprocessor {
public:
    virtual ~Preprocessor() = default;

    /// @brief Reencodes the CNF using the preprocessor's algorithm.
    virtual void reencode(vector<Clause> &cnf) = 0;
};

class BVA : public Preprocessor {
private:
public:
    BVA() = default;
    virtual ~BVA() = default;

    virtual void reencode(vector<Clause> &cnf) override;
};
