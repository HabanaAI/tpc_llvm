// RUN: %clang_cc1 -S -triple tpc-none-none -std=rc99 -mllvm -tpc-unbranch=1 -mllvm -tpc-unbranch-cnf=1 -O1 %s -o - | FileCheck %s

/*
With new changes unbranch already do not hide issie.

 ADRF registers are not spillable on this processor come back!
 
commit 20de96da46d3b3bb2463eff289799252fbfcc9ce 
Author: Md Asghar Ahmad Shahid <aashahid@habana.ai>
Date:   Wed Jun 16 09:07:44 2021 +0300

    [LLVM-1775]: Fix to handle overlapping paths involving fptrunc/fpext and fptrunc/store-pack.

    Change-Id: Ib07ae3b97cd07341090a71917bb63acf595d91a7



*/

int to_prepare_data = 0;
int to_run_test = 1;
void test_runner(int to_do, tensor out_tnsr)
{
    int5 out_index = {-1,0,0,0,0};
    if (to_do == to_run_test)
    {
        out_index[0] += 1;
        s_i32_st_g (gen_addr(out_index, out_tnsr, 0, 0, 1, 0), 1, 0, 1, 0);

        out_index[0] += 1;
        s_i32_st_g (gen_addr(out_index, out_tnsr, 0, 0, 1, 0), 2, 0, 1, 0);

        out_index[0] += 1;
        s_i32_st_g (gen_addr(out_index, out_tnsr, 0, 0, 1, 0), 3, 0, 1, 0);

        out_index[0] += 1;
        s_i32_st_g (gen_addr(out_index, out_tnsr, 0, 0, 1, 0), 4, 0, 1, 0);

        out_index[0] += 1;
        s_i32_st_g (gen_addr(out_index, out_tnsr, 0, 0, 1, 0), 5, 0, 1, 0);

        out_index[0] += 1;
        s_i32_st_g (gen_addr(out_index, out_tnsr, 0, 0, 1, 0), 6, 0, 1, 0);
#if 1
        out_index[0] += 1;
        s_i32_st_g (gen_addr(out_index, out_tnsr, 0, 0, 1, 0), 7, 0, 1, 0);

        out_index[0] += 1;
        s_i32_st_g (gen_addr(out_index, out_tnsr, 0, 0, 1, 0), 8, 0, 1, 0);

        out_index[0] += 1;
        s_i32_st_g (gen_addr(out_index, out_tnsr, 0, 0, 1, 0), 9, 0, 1, 0);
#endif
    }
}

void main(tensor out_tnsr)
{
    test_runner(to_prepare_data, out_tnsr);  // dummy call; remove -> correct result 
    test_runner(to_run_test, out_tnsr);
}

//CHECK-NOT: jmpr
