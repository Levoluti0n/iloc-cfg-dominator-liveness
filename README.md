# ILOC CFG · Dominators · Liveness

A small compiler middle-end laboratory implementing core program analysis passes on a linear ILOC-like intermediate representation.

This project parses a linear ILOC-style IR, constructs a Control-Flow Graph (CFG), computes dominator sets, and performs liveness data-flow analysis — the same foundational analyses used in real compilers such as LLVM, GCC, and FPGA HLS toolchains.

---

## Features

- **Linear IR Parser**
  - Supports labels, conditional branches, unconditional jumps, and returns
  - Accepts ILOC-style syntax with optional inline comments

- **Control-Flow Graph (CFG) Construction**
  - Leader detection
  - Basic block formation
  - Predecessor / successor edge construction

- **Dominator Analysis**
  - Iterative data-flow algorithm
  - Computes dominator sets for each basic block

- **Liveness Analysis**
  - USE / DEF computation per block
  - Backward data-flow solving IN / OUT sets
  - Register-level liveness tracking

---

## Example Input (ILOC)

```txt
L0:
  loadI 0      => r1
  loadI 10     => r2
  cmp_LT r1,r2 => r3
  cbr r3       -> L1, L2

L1:
  addI r1,1    => r1
  jumpI        -> L0

L2:
  store r1     => @out
  return
