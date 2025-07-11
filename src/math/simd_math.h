#pragma once

#include <immintrin.h>

namespace op
{
    inline void add(const float* l, const float* r, float* dst)
    {
        __m128 sl = _mm_load_ps(l);
        __m128 sr = _mm_load_ps(r);
        auto a = _mm_add_ps(sl, sr);
        _mm_store_ps(dst, a);
    }

    inline void sub(const float* l, const float* r, float* dst)
    {
        __m128 sl = _mm_load_ps(l);
        __m128 sr = _mm_load_ps(r);
        auto a = _mm_sub_ps(sl, sr);
        _mm_store_ps(dst, a);
    }

    inline void mul(const float* l, const float* r, float* dst)
    {
        __m128 sl = _mm_load_ps(l);
        __m128 sr = _mm_load_ps(r);
        auto a = _mm_mul_ps(sl, sr);
        _mm_store_ps(dst, a);
    }

    inline void div(const float* l, const float* r, float* dst)
    {
        __m128 sl = _mm_load_ps(l);
        __m128 sr = _mm_load_ps(r);
        auto a = _mm_div_ps(sl, sr);
        _mm_store_ps(dst, a);
    }

    inline void sqrt(const float* f4, float* dst)
    {
        __m128 f = _mm_load_ps(f4);
        auto a = _mm_sqrt_ps(f);
        _mm_store_ps(dst, a);
    }

    inline void length(const float* f4, float* dst)
    {
        __m128 f = _mm_load_ps(f4);
        auto a = _mm_mul_ps(f, f);
        a = _mm_hadd_ps(a, a);
        a = _mm_hadd_ps(a, a);
        a = _mm_sqrt_ss(a);
        
        *dst = _mm_cvtss_f32(a);
    }

    inline void normalize(const float* f4, float* dst)
    {
        __m128 f = _mm_load_ps(f4);
        
        auto a = _mm_mul_ps(f, f);
        a = _mm_hadd_ps(a, a);
        a = _mm_hadd_ps(a, a);
        a = _mm_sqrt_ps(a);
        
        a = _mm_div_ps(f, a);
        _mm_store_ps(dst, a);
    }

    inline void normalize_safe(const float* f4, float* dst)
    {
        __m128 f = _mm_load_ps(f4);
        auto a = _mm_cmpneq_ps(f, _mm_setzero_ps());
        if (_mm_movemask_ps(a) == 0)
        {
            _mm_store_ps(dst, f);
            return;
        }
        
        a = _mm_mul_ps(f, f);
        a = _mm_hadd_ps(a, a);
        a = _mm_hadd_ps(a, a);
        a = _mm_sqrt_ps(a);
        
        a = _mm_div_ps(f, a);
        _mm_store_ps(dst, a);
    }

    inline void cross(const float* l, const float* r, float* dst)
    {
        dst[0] = l[1] * r[2] - l[2] * r[1];
        dst[1] = - l[0] * r[2] + l[2] * r[0];
        dst[2] = l[0] * r[1] - l[1] * r[0];
    }

    inline void dot(const float* l, const float* r, float* dst)
    {
        __m128 sl = _mm_load_ps(l);
        __m128 sr = _mm_load_ps(r);
        
        auto a = _mm_mul_ps(sl, sr);
        a = _mm_hadd_ps(a, a);
        a = _mm_hadd_ps(a, a);

        *dst = _mm_cvtss_f32(a);
    }

    inline bool near_equal(const float* l, const float* r)
    {
        __m128 sl = _mm_load_ps(l);
        __m128 sr = _mm_load_ps(r);

        auto diff = _mm_sub_ps(sl, sr);
        auto abs_diff = _mm_andnot_ps(_mm_set1_ps(-0.0f), diff);
        auto cmp = _mm_cmplt_ps(abs_diff, _mm_set1_ps(0.00001f));
        return _mm_movemask_ps(cmp) == 0xF;
    }

    inline void gram_schmidt_orth(const float* forward, const float* up, float* right, float* new_up)
    {
        cross(forward, up, right);
        normalize(right, right);
        cross(forward, right, new_up);
    }
}
