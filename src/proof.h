#pragma once
#include "file.h"

class ProofTracer {
public:
    virtual ~ProofTracer() = default;
    virtual void notify_added_clause(const std::vector<int>&, bool original) = 0;
    virtual void notify_deleted_clause(const std::vector<int>&) = 0;
    virtual void notify_comment(const std::string &) = 0;
};

class ProofDumper : public ProofTracer {
private:
    File m_file;
public:
    ProofDumper(const std::string& filename) {
        if (!m_file.open(filename))
            std::cerr << "Could not open file " << filename << " for writing.\n";
    }

    virtual ~ProofDumper() { m_file.close(); }

    virtual void notify_added_clause(const std::vector<int>& c, bool original) override {
        if (original)
            return;
        std::string line;
        for (int lit : c)
            line += std::to_string(lit) + " ";
        line += "0";
        m_file.line(line);
    }

    virtual void notify_deleted_clause(const std::vector<int>& c) override {
        std::string line = "d";
        for (int lit : c)
            line += " " + std::to_string(lit);
        line += " 0";
        m_file.line(line);
    }

    virtual void notify_comment(const std::string &line) override {
        m_file.line("c " + line);
    }
};
