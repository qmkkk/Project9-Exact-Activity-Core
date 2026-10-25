/* SPDX-License-Identifier: MIT */
#ifndef PROJECT9_JOINT_RELATION_H
#define PROJECT9_JOINT_RELATION_H
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    unsigned supported[2];
    unsigned without_budget[2];
} P9JointResult;
/* Serial API. rows=4,8,16. target and input-row restrictions normalized at row0.
 * operators bits0..4 select truth tables2,4,6,8,14 (index x+2*y).
 * domain bit a denotes folded population a, 1<=a<=rows/2.
 * budget is AFTER subtracting the safe lower bound for other charged nodes.
 * charged bits0/1 indicate which of the two distinct inputs are charged.
 * Returns0 on valid input, -1 on invalid input. Empty support means infeasible
 * under this local necessary condition, not global circuit unsatisfiability.
 */
int p9_joint_support(unsigned rows,unsigned target,unsigned operators,
    const unsigned zeros[2],const unsigned ones[2],const unsigned domains[2],
    int budget,unsigned charged,P9JointResult *result);
int p9_connection_feasible(unsigned rows,unsigned target,unsigned operators,
    const unsigned zeros[2],const unsigned ones[2],const unsigned domains[2],
    int budget,unsigned charged);
#ifdef __cplusplus
}
#endif
#endif
