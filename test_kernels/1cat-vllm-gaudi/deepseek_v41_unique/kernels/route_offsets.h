// SPDX-License-Identifier: Apache-2.0
#ifndef DSV41_GROUPED_RANKED_OFFSET_H
#define DSV41_GROUPED_RANKED_OFFSET_H
// Prefix of exact M template row capacities for count-sorted owners. Each
// owner g is bounded by floor(36/(g+1)); these intervals are the constant
// quotient runs. Avoid a scalar load/division loop in every TPC partition.
static inline int dsv41_ranked_offset(int group) {
    if (group >= 18) return 245 + group - 18;
    if (group >= 12) return 227 + 3 * (group - 12);
    if (group >= 9) return 209 + 6 * (group - 9);
    if (group >= 7) return 189 + 10 * (group - 7);
    if (group == 6) return 174;
    if (group >= 3) return 111 + 21 * (group - 3);
    if (group == 2) return 84;
    if (group == 1) return 51;
    return 0;
}
static inline int dsv41_ranked_full_capacity(int group) {
    if (group >= 6) return 0;
    if (group >= 3) return 6;
    if (group == 2) return 12;
    if (group == 1) return 18;
    return 36;
}
static inline int dsv41_compact_offset(int group) {
    // Count-sorted capacity quotient runs, verified against the host formula.
    if (group >= 18) return 148 + group - 18;
    if (group >= 12) return 136 + 2 * (group - 12);
    if (group >= 9) return 127 + 3 * (group - 9);
    if (group >= 7) return 119 + 4 * (group - 7);
    if (group == 6) return 114;
    if (group >= 3) return 81 + 11 * (group - 3);
    if (group == 2) return 64;
    if (group == 1) return 41;
    return 0;
}
#endif
