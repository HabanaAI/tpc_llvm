// SPDX-License-Identifier: Apache-2.0
// One eight-vector load/decode stage; intentionally no include guard.
                    const uchar256 next0 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64;
                    const uchar256 next1 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64;
                    const uchar256 next2 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64;
                    const uchar256 next3 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64;
                    const uchar256 next4 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64;
                    const uchar256 next5 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64;
                    const uchar256 next6 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64;
                    const uchar256 next7 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64;
#if DSV41_N256_PREFETCH == 16
                    const uchar256 next8 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64;
                    const uchar256 next9 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64;
                    const uchar256 next10 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64;
                    const uchar256 next11 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64;
                    const uchar256 next12 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64;
                    const uchar256 next13 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64;
                    const uchar256 next14 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64;
                    const uchar256 next15 = v_u8_ld_tnsr_b(source, q16, SW_UNPACK | SW_UNPCK_4_TO_8);
                    source[0] += 64;
#endif
                    DSV41_N256_STORE(pending0);
                    DSV41_N256_STORE(pending1);
                    DSV41_N256_STORE(pending2);
                    DSV41_N256_STORE(pending3);
                    DSV41_N256_STORE(pending4);
                    DSV41_N256_STORE(pending5);
                    DSV41_N256_STORE(pending6);
                    DSV41_N256_STORE(pending7);
#if DSV41_N256_PREFETCH == 16
                    DSV41_N256_STORE(pending8);
                    DSV41_N256_STORE(pending9);
                    DSV41_N256_STORE(pending10);
                    DSV41_N256_STORE(pending11);
                    DSV41_N256_STORE(pending12);
                    DSV41_N256_STORE(pending13);
                    DSV41_N256_STORE(pending14);
                    DSV41_N256_STORE(pending15);
#endif
                    pending0 = next0;
                    pending1 = next1;
                    pending2 = next2;
                    pending3 = next3;
                    pending4 = next4;
                    pending5 = next5;
                    pending6 = next6;
                    pending7 = next7;
#if DSV41_N256_PREFETCH == 16
                    pending8 = next8;
                    pending9 = next9;
                    pending10 = next10;
                    pending11 = next11;
                    pending12 = next12;
                    pending13 = next13;
                    pending14 = next14;
                    pending15 = next15;
#endif
