// SPDX-License-Identifier: Apache-2.0
// The control and program operands retain the typed consumer dependencies.
// The qualified SAT decoder consumes an ordinary one-row int32 ID tensor.
void main(tensor metadata, tensor controls, tensor program, tensor ids) {
    const int64 experts = v_i32_ld_tnsr_b((int5){0,1,0,0,0}, metadata);
    v_i32_st_tnsr_partial((int5){0,0,0,0,0}, ids, experts, 35, 0);
}
