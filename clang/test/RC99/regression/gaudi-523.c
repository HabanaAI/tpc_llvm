// RUN: %tpc_clang -S -O1 -o - %s

void main(int int_par, tensor input_tnsr, tensor out_tnsr)
{
    int5 out_index = {0,0,0,0,0};

    int int_var = 2;
    int total_count_for_level_2 = 0;
    for (int i = 0; i < int_par; i++)
    //for (int i = 0; i < int_var; i++) // with tis line - no crash
    //for (int i = 0; i < 2; i++) // with tis line - no crash
    {
        for (int j = 0; j < i; j++)
        {
            total_count_for_level_2++; // comment out this line - no crash
        }
    }

    i32_st_tnsr_i_v(out_index, out_tnsr, total_count_for_level_2); // comment out this line - no crash
}
