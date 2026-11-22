// VLM forwarding (TPCLocalMemoryForwarding): a vector stored to VLM is read
// back at the same address, after a store that overlaps it, after a store
// under a run-time predicate, at an address known only at run time, across a
// tensor store, and in the next row (kept in VLM between iterations).
//
// DIFFTEST: arch gaudi gaudi2 gaudi3
// DIFFTEST: in  f32 256 8 3
// DIFFTEST: out f32 256 8 3
// DIFFTEST: scalar i32 0
// DIFFTEST: index-space 4 1 3

void main(tensor a, tensor out, int slot) {
  const int5 start = get_index_space_offset();
  const int5 end = get_index_space_size() + start;
  int5 i = {0, 0, 0, 0, 0};

  for (int z = start[2]; z < end[2]; z++) {
    i[2] = z;
    for (int x = start[0] * 64; x < end[0] * 64; x += 64) {
      i[0] = x;
      i[1] = 0;
      // The previous row.
      v_f32_st_l_v(1024, v_f32_ld_tnsr_b(i, a));
      for (int y = 0; y < 8; y++) {
        i[1] = y;
        float64 v = v_f32_ld_tnsr_b(i, a);
        v_f32_st_l_v(0, v);
        v_f32_st_l_v(256, v * 2.0f);
        float64 same = v_f32_ld_l_v_b(0);
        // Overlaps the vectors at 0 and 256.
        v_f32_st_l_v(128, v + 1.0f);
        float64 over = v_f32_ld_l_v_b(0);
        // Stores on odd rows only.
        v_f32_st_l_v(256, v - 3.0f, 0, (y & 1) != 0, 0);
        float64 cond = v_f32_ld_l_v_b(256);
        // slot is 0: reads the vector at 0, stored at 128 on top.
        float64 any = v_f32_ld_l_v_b(slot * 256);
        float64 prev = v_f32_ld_l_v_b(1024);
        v_f32_st_tnsr(i, out, same + over * 0.5f + cond * 0.25f + any - prev);
        v_f32_st_l_v(1024, v);
      }
    }
  }
}
