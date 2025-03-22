# Edusat-BVA

## About

<div id="top" align="center">

This project was developed by **Thomas Hashem** and **Basel Khouri** as part of the course **"Algorithms in Logic - 00960265"** taught by **Prof. Ofer Strichman**.

</div>

The project implements Simple Bounded Variable Addition (BVA) into the EDUSAT SAT solver. It is designed to be modular and can be integrated into other SAT solvers. The algorithm is based on the paper [Automated Reencoding of Boolean Formulas](https://research.ibm.com/haifa/conferences/hvc2012/papers/paper16.pdf).

Additionally, as part of this project, we extended EDUSAT to produce **DRAT UNSAT proofs** in cases where the CNF is unsatisfiable.

## Cloning the Repository

Ensure that you clone the repository with its submodules. Use the following command:

```bash
git clone --recurse-submodules <repository-url>
```

If you have already cloned the repository without submodules, you can initialize and update them using:

```bash
git submodule update --init --recursive
```

## Building the Project

To build the project, follow these steps:

1. **Release Build**: Run `make release` to create a release build.
2. **Debug Build**: Run `make debug` to create a debug build.

For the first-time setup, it is recommended to run the `./build.sh` script. This script not only builds the project but also initializes and builds the required submodules: `cadical` and `drat-trim`.

## Scripts Overview

This project includes several utility scripts to streamline the workflow:

### `/scripts/set_env.sh`
This script initializes all the required aliases and environment variables. It should be run before executing any other script to ensure the environment is correctly configured.

### `/scripts/process_cnf.sh`
This script processes all CNF files in a predefined folder (hardcoded in the script). It runs `cadical` on each CNF file and renames the files based on the result:
- `_yes` is appended if the result is SAT.
- `_no` is appended if the result is UNSAT.
- `_unknown` is appended if the result is inconclusive.

### `/tests/sat_verifier.sh`
This script verifies the correctness of SAT assignments produced by the `edusat -bva` command:
1. It takes a folder containing CNF instances as input.
2. Filters out all `_yes` instances.
3. Runs `edusat -bva YES_CNF_INPUT` on each filtered instance to produce an assignment.
4. Concatenates the assignment to the original CNF and runs `cadical` to verify the result:
   - If `cadical` confirms SAT, the result is verified.
   - Otherwise, the result is not verified, indicating a potential bug.

Note: This script must be run from the `tests` folder only after running `set_env.sh` script.

### `/tests/unsat-verifier.sh`
This script verifies all UNSAT instances produced by the `edusat -bva` command:
1. It takes a folder containing CNF instances as input.
2. Filters out all `_no` instances.
3. Runs the `edusat -bva -proof PROOF_FILE NO_CNF_INPUT` command to generate a DRAT proof.
4. Uses `drat-trim` to validate the proof with the command: `drat-trim NO_CNF_INPUT PROOF_FILE`.
   - drat-trim either outputs: **verified** or **not verified**.

Note: This script must be run from the `tests` folder only after running `set_env.sh` script.