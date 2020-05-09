#pragma once

// OPTIMIZE THIS PLEASE
inline void generate_ramp(float* buf, size_t buf_size){
	for(int i=0; i<buf_size; i++){
		buf[i] = 2.f*i/buf_size-1.f;
	}
}

// test vertex interleave (OPTIMIZE LATER PLEASE)
// sizeof(dest) = 2*sizeof(src1) = 2*sizeof(src2) = 2*sizeof(float)*src_len
inline void float_interleave(float* dest, float* src1, float* src2, size_t src_len){
	for(int i=0;i<src_len;i++){
		dest[2*i] = src1[i];
		dest[2*i+1] = src2[i];
	}
}

// sizeof(dest)/sizeof(float) = sizeof(src)/sizeof(double) = src_len
//inline void float_cast(float* dest, double* src, size_t src_len){
//	
//}