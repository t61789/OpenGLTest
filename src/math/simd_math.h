#pragma once

#include <immintrin.h>

#include "const.h"

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

    inline void max(const float* l, const float* r, float* dst)
    {
        __m128 sl = _mm_load_ps(l);
        __m128 sr = _mm_load_ps(r);

        auto a = _mm_max_ps(sl, sr);
        _mm_store_ps(dst, a);
    }

    inline void min(const float* l, const float* r, float* dst)
    {
        __m128 sl = _mm_load_ps(l);
        __m128 sr = _mm_load_ps(r);

        auto a = _mm_min_ps(sl, sr);
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

    inline void gram_schmidt_ortho(const float* forward, const float* up, float* right, float* new_up)
    {
        cross(up, forward, right);
        normalize(right, right);
        cross(forward, right, new_up);
    }

    inline __m128 sign(const __m128 v)
    {
        __m128 zero = _mm_setzero_ps();
        __m128 negative_mask = _mm_cmplt_ps(v, zero);  // 负数掩码
        __m128 positive_mask = _mm_cmpgt_ps(v, zero);  // 正数掩码
    
        __m128 negative = _mm_and_ps(negative_mask, _mm_set1_ps(-1.0f));
        __m128 positive = _mm_and_ps(positive_mask, _mm_set1_ps(1.0f));
    
        return _mm_or_ps(negative, positive);
    }

    inline void sign(const float* v, float* dst)
    {
        auto sv = _mm_load_ps(v);
        
        _mm_store_ps(dst, sign(sv));
    }
    
    struct SimdVec3
    {
        __m128 x;
        __m128 y;
        __m128 z;

        SimdVec3() = default;
        
        explicit SimdVec3(cr<__m128> v)
        {
            x = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0));
            y = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
            z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
        }
        
        SimdVec3(cr<__m128> x, cr<__m128> y, cr<__m128> z)
        {
            this->x = x;
            this->y = y;
            this->z = z;
        }
    };

    struct SimdVec4
    {
        __m128 x;
        __m128 y;
        __m128 z;
        __m128 w;

        SimdVec4() = default;
        
        explicit SimdVec4(cr<__m128> v)
        {
            x = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0));
            y = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
            z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
            w = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));
        }
        
        SimdVec4(cr<__m128> x, cr<__m128> y, cr<__m128> z, cr<__m128> w)
        {
            this->x = x;
            this->y = y;
            this->z = z;
            this->w = w;
        }
    };
    
    inline SimdVec3 add(cr<SimdVec3> l, cr<SimdVec3>r)
    {
        return {
            _mm_add_ps(l.x, r.x),
            _mm_add_ps(l.y, r.y),
            _mm_add_ps(l.z, r.z)
        };
    }

    inline SimdVec3 sub(cr<SimdVec3> l, cr<SimdVec3>r)
    {
        return {
            _mm_sub_ps(l.x, r.x),
            _mm_sub_ps(l.y, r.y),
            _mm_sub_ps(l.z, r.z)
        };
    }

    inline SimdVec3 mul(cr<SimdVec3> l, cr<SimdVec3>r)
    {
        return {
            _mm_mul_ps(l.x, r.x),
            _mm_mul_ps(l.y, r.y),
            _mm_mul_ps(l.z, r.z)
        };
    }

    inline SimdVec4 add(cr<SimdVec4> l, cr<SimdVec4>r)
    {
        return {
            _mm_add_ps(l.x, r.x),
            _mm_add_ps(l.y, r.y),
            _mm_add_ps(l.z, r.z),
            _mm_add_ps(l.w, r.w)
        };
    }

    inline SimdVec4 sub(cr<SimdVec4> l, cr<SimdVec4>r)
    {
        return {
            _mm_sub_ps(l.x, r.x),
            _mm_sub_ps(l.y, r.y),
            _mm_sub_ps(l.z, r.z),
            _mm_sub_ps(l.w, r.w)
        };
    }

    inline SimdVec4 mul(cr<SimdVec4> l, cr<SimdVec4>r)
    {
        return {
            _mm_mul_ps(l.x, r.x),
            _mm_mul_ps(l.y, r.y),
            _mm_mul_ps(l.z, r.z),
            _mm_mul_ps(l.w, r.w)
        };
    }

    inline __m128 dot(cr<SimdVec4> l, cr<SimdVec4>r)
    {
        auto x = _mm_mul_ps(l.x, r.x);
        auto y = _mm_mul_ps(l.y, r.y);
        auto z = _mm_mul_ps(l.z, r.z);
        auto w = _mm_mul_ps(l.w, r.w);

        return _mm_add_ps(x, _mm_add_ps(y, _mm_add_ps(z, w)));
    }
}
