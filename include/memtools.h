#pragma once

#include "glm/glm.hpp"
#include <immintrin.h>

// OPTIMIZE THIS PLEASE
inline void generate_ramp(float* buf, size_t buf_size){
	for(int i=0; i<buf_size; i++){
		buf[i] = 2.f*i/buf_size-1.f;
	}
}

// OPTIMIZE THIS PLEASE
inline void generate_duplicate_ramp(float* buf, size_t buf_size){
	for(int i=0; i<buf_size; i++){
		buf[2*i] = 2.f*i/buf_size-1.f;
		buf[2*i+1] = buf[2*i];
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

// p = [P3,P2,P1,P0]  (SSE2)
inline float bezier_value(__m128 p, const float t){
	const __m128 tv = _mm_set_ps1(t);
	
	__m128 h = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(p),4)); // [0,P3,P2,P1]
	h = _mm_sub_ps(h,p); // [-P3,P3-P2,P2-P1,P1-P0]
	h = _mm_mul_ps(h,tv); // [-P3,P3-P2,P2-P1,P1-P0] t
	h = _mm_add_ps(h,p); // [-P3 t,P2+(P3-P2)t,P1+(P2-P1)t,P0+(P1-P0)t] = [?,H2,H1,H0]
	
	__m128 l = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(h),4)); // [0,?,H2,H1]
	l = _mm_sub_ps(l,h); // [?,?,H2-H1,H1-H0]
	l = _mm_mul_ps(l,tv); // [?,?,H2-H1,H1-H0] t
	l = _mm_add_ps(l,h); // [?,?,H1+(H2-H1)t,H0+(H1-H0)t] = [?,?,L1,L0]
	
	h = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(l),4)); // [0,?,?,L1]
	h = _mm_sub_ps(h,l); // [?,?,?,L1-L0]
	h = _mm_mul_ps(h,tv); // [?,?,?,(L1-L0)t]
	h = _mm_add_ps(h,l); // [?,?,?,L0+(L1-L0)t] = [?,?,?,b(t)]
	
	return _mm_cvtss_f32(h); // b(t)
}

//vv Neville's algorithm (SSE2)
inline float cubic_interpolate(__m128 x, __m128 y, float in){
	__m128 inv = _mm_set_ps1(in); // [in,in,in,in]
	inv = _mm_sub_ps(inv,x); // [in-X3,in-X2,in-X1,in-X0]
	
	__m128 v = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(inv),4)); // [0,in-X3,in-X2,in-X1]
	__m128 d = _mm_sub_ps(v,inv); // [X3-in,X2-X3,X1-X2,X0-X1]
	v = _mm_mul_ps(v,y); // [0,(in-X3)Y2,(in-X2)Y1,(in-X1)Y0]
	__m128 res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(y),4)); // [0,Y3,Y2,Y1]
	res = _mm_mul_ps(inv,res); // [0,(in-X2)Y3,(in-X1)Y2,(in-X0)Y1]
	res = _mm_sub_ps(v,res); // [0,(in-X3)Y2-(in-X2)Y3,(in-X2)Y1-(in-X1)Y2,(in-X1)Y0-(in-X0)Y1]
	res = _mm_div_ps(res,d); // [0,p23,p12,p01]
	
	v = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(inv),8)); // [0,0,in-X3,in-X2]
	d = _mm_sub_ps(v,inv); // [X3-in,X2-in,X1-X3,X0-X2]
	v = _mm_mul_ps(v,res); // [0,0,(in-X3)p12,(in-X2)p01]
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res),4)); // [0,0,p23,p12]
	res = _mm_mul_ps(inv,res); // [0,0,(in-X1)p23,(in-X0)p12]
	res = _mm_sub_ps(v,res); // [0,0,(in-X3)p12-(in-X1)p23,(in-X2)p01-(in-X0)p12]
	res = _mm_div_ps(res,d); // [0,0,p13,p02]
	
	v = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(inv),12)); // [0,0,0,in-X3]
	d = _mm_sub_ps(v,inv); // [X3-in,X2-in,X1-in,X0-X3]
	v = _mm_mul_ps(v,res); // [0,0,0,(in-X3)p02]
	res = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(res),4)); // [0,0,0,p13]
	res = _mm_mul_ps(inv,res); // [0,0,0,(in-X0)p13]
	res = _mm_sub_ps(v,res); // [0,0,0,(in-X3)p02-(in-X0)p13]
	res = _mm_div_ps(res,d); // [0,0,0,p03]
	
	return _mm_cvtss_f32(res); // p03
}

inline float linear_interpolate(float v1, float v2, float amt){
	return (1-amt)*v1+amt*v2;
}

inline void float_copy(float* dest, float* src, size_t buf_size){
	for(int i=0;i<buf_size;i++){
		dest[i] = src[i];
	}
}

inline void float_clamp(float* dest, float* src, size_t buf_size, float lower, float higher){
	for(int i=0;i<buf_size;i++){
		if(src[i] > higher) dest[i] = higher;
		else if(src[i] < lower) dest[i] = lower;
		else dest[i] = src[i];
	}
}