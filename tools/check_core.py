"""Build the limited public core and check it against an independent row DP."""
from pathlib import Path
import ctypes as C
import hashlib
import json
import os
import random
import subprocess

ROOT = Path(__file__).resolve().parents[1]
OPS = (2, 4, 6, 8, 14)


def reference(n, target, operators, zeros, ones, domains, budget, charged):
    plain = [0, 0]
    supported = [0, 0]
    for index, op in enumerate(OPS):
        if not operators & (1 << index):
            continue
        states = {(0, 0)}
        for row in range(n):
            choices = []
            for x in (0, 1):
                for y in (0, 1):
                    if any(((zeros[i] >> row) & 1 and v) or
                           ((ones[i] >> row) & 1 and not v)
                           for i, v in enumerate((x, y))):
                        continue
                    if ((op >> (x + 2 * y)) & 1) == ((target >> row) & 1):
                        choices.append((x, y))
            states = {(a + x, b + y) for a, b in states for x, y in choices}
        for a_raw, b_raw in states:
            if not (0 < a_raw < n and 0 < b_raw < n):
                continue
            a, b = min(a_raw, n - a_raw), min(b_raw, n - b_raw)
            if not (domains[0] & (1 << a) and domains[1] & (1 << b)):
                continue
            plain[0] |= 1 << a
            plain[1] |= 1 << b
            cost = (2 * a * (n - a) if charged & 1 else 0)
            cost += 2 * b * (n - b) if charged & 2 else 0
            if cost <= budget:
                supported[0] |= 1 << a
                supported[1] |= 1 << b
    return supported, plain


def main():
    build = ROOT / 'build'
    build.mkdir(exist_ok=True)
    library = build / 'libproject9_core.so'
    command = [os.environ.get('CC', 'cc'), '-std=c99', '-O2', '-Wall', '-Wextra',
               '-Werror', '-fPIC', '-shared', str(ROOT / 'src/joint_relation.c'),
               '-o', str(library)]
    subprocess.run(command, check=True)
    lib = C.CDLL(str(library))

    class Result(C.Structure):
        _fields_ = [('supported', C.c_uint * 2), ('without_budget', C.c_uint * 2)]

    ptr = C.POINTER(C.c_uint)
    args = [C.c_uint, C.c_uint, C.c_uint, ptr, ptr, ptr, C.c_int, C.c_uint]
    lib.p9_joint_support.argtypes = args + [C.POINTER(Result)]
    lib.p9_joint_support.restype = C.c_int
    lib.p9_connection_feasible.argtypes = args
    lib.p9_connection_feasible.restype = C.c_int
    checked = 0

    def check(n, target, operators, zeros, ones, domains, budget, charged):
        nonlocal checked
        a = (n, target, operators, (C.c_uint * 2)(*zeros),
             (C.c_uint * 2)(*ones), (C.c_uint * 2)(*domains), budget, charged)
        result = Result()
        assert lib.p9_joint_support(*a, C.byref(result)) == 0
        expected = reference(n, target, operators, zeros, ones, domains, budget, charged)
        actual = (list(result.supported), list(result.without_budget))
        assert actual == expected, (n, target, operators, zeros, ones, domains, budget, charged, actual, expected)
        assert lib.p9_connection_feasible(*a) == int(bool(expected[0][0]))
        checked += 1

    # Exhaust all normalized two-input targets and operation subsets with broad domains.
    for target in range(0, 16, 2):
        for operators in range(32):
            for budget in (-1, 0, 6, 8, 12, 16):
                check(4, target, operators, [1, 1], [0, 0], [6, 6], budget, 3)
    rng = random.Random(20261025)
    for n in (4, 8, 16):
        for index in range(400):
            zeros, ones = [1, 1], [0, 0]
            for pin in range(2):
                for row in range(1, n):
                    value = rng.randrange(5)
                    if value == 0:
                        zeros[pin] |= 1 << row
                    elif value == 1:
                        ones[pin] |= 1 << row
            mask = ((1 << (n // 2 + 1)) - 1) & ~1
            domains = [rng.randrange(mask + 1) & mask for _ in range(2)]
            check(n, rng.randrange(1 << n) & ~1, rng.randrange(32), zeros, ones,
                  domains, rng.randrange(-1, n * n + 1), rng.randrange(4))
    example = json.loads((ROOT / 'examples/joint_budget.json').read_text())
    for budget in (100, 126):
        check(16, int(example['target_hex'], 16), 4, [1, 1], [0, 0], [18, 18], budget, 3)
    assert reference(16, 0x3e, 4, [1, 1], [0, 0], [18, 18], 100, 3) == ([0, 0], [18, 18])
    assert reference(16, 0x3e, 4, [1, 1], [0, 0], [18, 18], 126, 3) == ([18, 18], [18, 18])
    invalid = Result()
    vals = (C.c_uint * 2)(1, 1)
    assert lib.p9_joint_support(32, 0, 31, vals, vals, vals, 0, 3, C.byref(invalid)) == -1
    assert lib.p9_joint_support(16, 1, 31, vals, vals, vals, 0, 3, C.byref(invalid)) == -1
    result = dict(passed=True, valid_cases=checked, invalid_cases=2,
                  reference='Independent row-wise integer-pair DP; no closed-form helper reused.',
                  source_sha256=hashlib.sha256((ROOT / 'src/joint_relation.c').read_bytes()).hexdigest(),
                  scope='Public local core correctness only; no complete SAT solver performance claim.')
    (build / 'validation.json').write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result))


if __name__ == '__main__':
    main()
