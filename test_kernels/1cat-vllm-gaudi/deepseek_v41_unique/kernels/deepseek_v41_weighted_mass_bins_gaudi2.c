// SPDX-License-Identifier: Apache-2.0
// One score/probability load per tile iteration; all sixteen bins stay in VRF.
#pragma clang fp contract(off)
void main(tensor scores,tensor probability,tensor prefixes,
#if defined(DSV41_WEIGHTED_SPARSE_BOUNDS) && DSV41_WEIGHTED_SPARSE_BOUNDS
          tensor active_blocks,
#endif
          tensor bins,int shift) {
    const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
    const int columns=get_dim_size(scores,0);
    const int64 lanes=(int64)V_LANE_ID_32;
    const unsigned int mask=shift==28?0u:0xffffffffu<<(shift+4);
    for(int row=begin[1];row<end[1];++row) {
        const unsigned int prefix=(unsigned int)s_i32_ld_g(gen_addr((int5){0,row},prefixes));
        for(int part=begin[0];part<end[0];++part) {
            const int first=part*2048,last=first+2048<columns?first+2048:columns;
#if defined(DSV41_WEIGHTED_SPARSE_BOUNDS) && DSV41_WEIGHTED_SPARSE_BOUNDS
            const unsigned active_mask=s_u32_ld_g(gen_addr((int5){part,row},active_blocks));
            if(!active_mask) {
                v_f32_st_tnsr_partial((int5){0,part,row},bins,(float64)0,15,0);
                continue;
            }
#endif
            float64 mass0=0;
            float64 mass1=0;
            float64 mass2=0;
            float64 mass3=0;
            float64 mass4=0;
            float64 mass5=0;
            float64 mass6=0;
            float64 mass7=0;
            float64 mass8=0;
            float64 mass9=0;
            float64 mass10=0;
            float64 mass11=0;
            float64 mass12=0;
            float64 mass13=0;
            float64 mass14=0;
            float64 mass15=0;
            for(int col=first;col<last;col+=64) {
#if defined(DSV41_WEIGHTED_SPARSE_BOUNDS) && DSV41_WEIGHTED_SPARSE_BOUNDS
                if(!(active_mask & (1u<<((col-first)/64))))continue;
#endif
                const float64 value=v_f32_ld_tnsr_b((int5){col,row},scores);
                const uint64 bits=*((uint64*)&value);
                const uint64 key=v_u32_sel_grt_u32_b(bits,0x7fffffffu,~bits,bits^0x80000000u);
                const uint64 digit=(key>>shift)&15;
                const bool64 eligible=v_u32_cmp_eq_b(key&mask,prefix)&v_i32_cmp_less_b(lanes+col,columns);
                const float64 weight=v_f32_ld_tnsr_b((int5){col,row},probability);
                mass0+=v_f32_mov_vb(weight,0,(float64)0,eligible&v_u32_cmp_eq_b(digit,0),0);
                mass1+=v_f32_mov_vb(weight,0,(float64)0,eligible&v_u32_cmp_eq_b(digit,1),0);
                mass2+=v_f32_mov_vb(weight,0,(float64)0,eligible&v_u32_cmp_eq_b(digit,2),0);
                mass3+=v_f32_mov_vb(weight,0,(float64)0,eligible&v_u32_cmp_eq_b(digit,3),0);
                mass4+=v_f32_mov_vb(weight,0,(float64)0,eligible&v_u32_cmp_eq_b(digit,4),0);
                mass5+=v_f32_mov_vb(weight,0,(float64)0,eligible&v_u32_cmp_eq_b(digit,5),0);
                mass6+=v_f32_mov_vb(weight,0,(float64)0,eligible&v_u32_cmp_eq_b(digit,6),0);
                mass7+=v_f32_mov_vb(weight,0,(float64)0,eligible&v_u32_cmp_eq_b(digit,7),0);
                mass8+=v_f32_mov_vb(weight,0,(float64)0,eligible&v_u32_cmp_eq_b(digit,8),0);
                mass9+=v_f32_mov_vb(weight,0,(float64)0,eligible&v_u32_cmp_eq_b(digit,9),0);
                mass10+=v_f32_mov_vb(weight,0,(float64)0,eligible&v_u32_cmp_eq_b(digit,10),0);
                mass11+=v_f32_mov_vb(weight,0,(float64)0,eligible&v_u32_cmp_eq_b(digit,11),0);
                mass12+=v_f32_mov_vb(weight,0,(float64)0,eligible&v_u32_cmp_eq_b(digit,12),0);
                mass13+=v_f32_mov_vb(weight,0,(float64)0,eligible&v_u32_cmp_eq_b(digit,13),0);
                mass14+=v_f32_mov_vb(weight,0,(float64)0,eligible&v_u32_cmp_eq_b(digit,14),0);
                mass15+=v_f32_mov_vb(weight,0,(float64)0,eligible&v_u32_cmp_eq_b(digit,15),0);
            }
            mass0=v_f32_reduce_add(mass0);
            v_f32_st_tnsr_partial((int5){0,part,row},bins,mass0,0,0);
            mass1=v_f32_reduce_add(mass1);
            v_f32_st_tnsr_partial((int5){1,part,row},bins,mass1,0,0);
            mass2=v_f32_reduce_add(mass2);
            v_f32_st_tnsr_partial((int5){2,part,row},bins,mass2,0,0);
            mass3=v_f32_reduce_add(mass3);
            v_f32_st_tnsr_partial((int5){3,part,row},bins,mass3,0,0);
            mass4=v_f32_reduce_add(mass4);
            v_f32_st_tnsr_partial((int5){4,part,row},bins,mass4,0,0);
            mass5=v_f32_reduce_add(mass5);
            v_f32_st_tnsr_partial((int5){5,part,row},bins,mass5,0,0);
            mass6=v_f32_reduce_add(mass6);
            v_f32_st_tnsr_partial((int5){6,part,row},bins,mass6,0,0);
            mass7=v_f32_reduce_add(mass7);
            v_f32_st_tnsr_partial((int5){7,part,row},bins,mass7,0,0);
            mass8=v_f32_reduce_add(mass8);
            v_f32_st_tnsr_partial((int5){8,part,row},bins,mass8,0,0);
            mass9=v_f32_reduce_add(mass9);
            v_f32_st_tnsr_partial((int5){9,part,row},bins,mass9,0,0);
            mass10=v_f32_reduce_add(mass10);
            v_f32_st_tnsr_partial((int5){10,part,row},bins,mass10,0,0);
            mass11=v_f32_reduce_add(mass11);
            v_f32_st_tnsr_partial((int5){11,part,row},bins,mass11,0,0);
            mass12=v_f32_reduce_add(mass12);
            v_f32_st_tnsr_partial((int5){12,part,row},bins,mass12,0,0);
            mass13=v_f32_reduce_add(mass13);
            v_f32_st_tnsr_partial((int5){13,part,row},bins,mass13,0,0);
            mass14=v_f32_reduce_add(mass14);
            v_f32_st_tnsr_partial((int5){14,part,row},bins,mass14,0,0);
            mass15=v_f32_reduce_add(mass15);
            v_f32_st_tnsr_partial((int5){15,part,row},bins,mass15,0,0);
        }
    }
}
