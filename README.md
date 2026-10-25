# TJCAP: target-conditioned joint feasibility core

A **limited public source release** of the local joint-count relation used in
our extension of the DATE2026 exact-switching-activity synthesis algorithm.
The repository provides the core relation, budget support filtering, a local
connection-feasibility interface, an independent reference checker, one
explanatory example and aggregate experimental results.

**This is not the full SAT-integrated solver or a full reproducibility package.**
Complete integration, SAT explanations, full benchmark runners/raw logs,
proof documents and development history are retained in the private handoff.
The aggregate end-to-end results were measured using that complete solver;
they cannot be reproduced by timing this standalone core.

## Build and verify

Requirements: Linux/WSL, Python3.10+ and a C compiler (GCC or Clang).
No Python packages or ABC checkout are required for this public subset.

```sh
python3 tools/check_core.py
```

This compiles `src/joint_relation.c` into an ignored `build/` directory and
compares it with an independent row-by-row integer-pair dynamic program.
It also checks the budget example and the connection-feasibility API.
The test is a correctness check, not an integrated solver benchmark.

## Interface and scope

See `src/joint_relation.h`. Truth tables contain 4, 8 or 16 rows, corresponding to
2, 3 or 4 inputs. Row 0 must be normalized to zero. Operation masks select
the two-input operations with truth encodings 2, 4, 6, 8, 14 using index `x+2*y`.
Domain bit `a` denotes folded population `min(A,N-A)=a`; input signals are
nonconstant. Input 0/1 may be charged separately. The budget argument is the
residual upper bound after subtracting a sound lower bound for other nodes.

The output gives supports with and without budget filtering. Empty joint
support permits excluding a candidate connection under these premises.
Nonempty support only establishes local feasibility, not existence of a
complete circuit. Calls are serial: the implementation caches small tables
in static storage. Five-input support is not implemented.

## Reported data

`data/` contains aggregate results with scope and adverse cases retained.
The208-class main comparison and 832-call factorial are distinct cohorts.
The 48 long calls also provide 30-second prefixes; these are not independent
repeats. On seven real circuits with identical completed work sets, the full
workflow median-time geometric speedups are 2.194x versus the author
engineering control and 1.519x versus the strong control. The eighth circuit,
cavlc, completes 85/93 rather than 82/93 with lower total time in all three runs.
Full results do not imply a standalone kernel advantage over generic GAC.

See `THIRD_PARTY_NOTICES.md` for the original paper, author source and licenses.
This project is research code. No paper title, authorship list, publication
acceptance or physical-power benefit is claimed by this repository.
