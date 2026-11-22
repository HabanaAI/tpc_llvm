// SPDX-License-Identifier: Apache-2.0
// Q and KV have independent work points after the common input projection.
// Retain every BF16/FP8/cache byte boundary in their shared physical launch.
#define DSV41_QNORM_FUNCTION 1
#define DSV41_QNORM_PUBLISH 1
#include "deepseek_v41_qnorm_quant_gaudi2.c"
#define DSV41_KV_PUBLISH_FUNCTION 1
#include "deepseek_v41_kv_norm_rope_publish_gaudi2.c"
void main(tensor q,tensor qnorm,tensor kv,tensor kvnorm,tensor positions,tensor phase,
          tensor cache,tensor decoded,tensor quantized,tensor scales,tensor normalized,
          tensor rotated,tensor completion,float epsilon,int offset) {
    const int5 begin=get_index_space_offset();
    const int5 end=begin+get_index_space_size();
    for(int row=begin[1];row<(end[1] ? end[1] : 1);++row) {
    for(int point=begin[0];point<end[0];++point) {
        if(point==0) {
            qnorm_quant_row(q,qnorm,quantized,scales,normalized,epsilon,1.0f/1280.0f,(int5){row},(int5){row+1});
        } else {
            kv_norm_publish_groups(kv,kvnorm,positions,phase,cache,decoded,rotated,completion,
                                   epsilon,1.0f/512.0f,offset,(int5){point-1,row},(int5){point,row+1});
        }
    }
    }
}
