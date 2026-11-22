// SPDX-License-Identifier: Apache-2.0
void main(tensor unused,tensor flags){
 const int lines=get_dim_size(flags,1);
 const int5 begin=get_index_space_offset(),end=begin+get_index_space_size();
 for(int block=begin[0];block<end[0];++block)for(int line=0;line<lines;++line)
  v_i32_st_tnsr_partial((int5){0,line},flags,(int64)0,31,0);
}
