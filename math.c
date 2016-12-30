#include "math.h"
#include <linux/kernel.h>
#include <linux/module.h>

int __fpclassifyl(long double x)
{
	union ldshape u = { x };
	int e = u.bits.exp;
	if (!e) {
		if (u.bits.m >> 63) return FP_NAN;
		else if (u.bits.m) return FP_SUBNORMAL;
		else return FP_ZERO;
	}
	if (e == 0x7fff)
		return u.bits.m & (uint64_t)-1>>1 ? FP_NAN : FP_INFINITE;
	return u.bits.m & (uint64_t)1<<63 ? FP_NORMAL : FP_NAN;
}

void mscalbn(double x, int n, double* ret)
{
    double scale;

    if (n > 1023) {
	x *= 0x1p1023;
	n -= 1023;
	if (n > 1023) {
	    x *= 0x1p1023;
	    n -= 1023;
	    if (n > 1023) {
		STRICT_ASSIGN(double, x, x * 0x1p1023);
		*ret = x;
		goto done;
	    }
	}
    } else if (n < -1022) {
	x *= 0x1p-1022;
	n += 1022;
	if (n < -1022) {
	    x *= 0x1p-1022;
	    n += 1022;
	    if (n < -1022) {
		STRICT_ASSIGN(double, x, x * 0x1p-1022);
		*ret = x;
		goto done;
	    }
	}
    }
    INSERT_WORDS(scale, (uint32_t)(0x3ff+n)<<20, 0);
    STRICT_ASSIGN(double, x, x * scale);
    *ret = x;
done:
    return;
}

void mexp(double x, double* ret)
{
    double hi, lo, c, xx;
    int k, sign;
    uint32_t hx;

    GET_HIGH_WORD(hx, x);
    sign = hx>>31;
    hx &= 0x7fffffff;  /* high word of |x| */

    /* special cases */
    if (hx >= 0x40862e42) {  /* if |x| >= 709.78... */
    	if (isnan(x))
    	    *ret = x; goto done;
    	if (hx == 0x7ff00000 && sign) /* -inf */
    	    *ret = 0; goto done;
    	if (x > 709.782712893383973096) {
    	    /* overflow if x!=inf */
    	    STRICT_ASSIGN(double, x, 0x1p1023 * x);
    	    *ret = x; goto done;
    	}
    	if (x < -745.13321910194110842) {
    	    /* underflow */
    	    STRICT_ASSIGN(double, x, 0x1p-1000 * 0x1p-1000);
    	    *ret = x; goto done;
    	}
    }

    /* argument reduction */
    if (hx > 0x3fd62e42) {  /* if |x| > 0.5 ln2 */
    	if (hx >= 0x3ff0a2b2)  /* if |x| >= 1.5 ln2 */
    	    k = (int)(invln2*x + half[sign]);
    	else
    	    k = 1 - sign - sign;
    	hi = x - k*ln2hi;  /* k*ln2hi is exact here */
    	lo = k*ln2lo;
    	STRICT_ASSIGN(double, x, hi - lo);
    } else if (hx > 0x3e300000)  {  /* if |x| > 2**-28 */
    	k = 0;
    	hi = x;
    	lo = 0;
    } else {
    	/* inexact if x!=0 */
    	FORCE_EVAL(0x1p1023 + x);
	*ret = 1 + x; goto done;
    }

    /* x is now in primary range */
    xx = x*x;
    c = x - xx*(P1+xx*(P2+xx*(P3+xx*(P4+xx*P5))));
    x = 1 + (x*c/(2-c) - lo + hi);
    if (k == 0) {
	*ret = x;
        goto done;
    }
    
    mscalbn(x, k, ret);

done:
    return;
}

void mlog(double x, double* ret)
{
    double hfsq,f,s,z,R,w,t1,t2,dk;
    int32_t k,hx,i,j;
    uint32_t lx;

    EXTRACT_WORDS(hx, lx, x);

    k = 0;
    if (hx < 0x00100000) {  /* x < 2**-1022  */
	if (((hx&0x7fffffff)|lx) == 0) {
	    //return -two54/0.0;  /* log(+-0)=-inf */
	    *ret = -two54/0.0; 
	    goto done2;
	}
	
	if (hx < 0) 
	{
	    //return (x-x)/0.0;   /* log(-#) = NaN */
	    *ret = (x-x)/0.0; 
	    goto done2;
	}

	
	/* subnormal number, scale up x */
	k -= 54;
	x *= two54;
	GET_HIGH_WORD(hx,x);
    }
    
    if (hx >= 0x7ff00000)
    {
	//return x+x;
	*ret = x+x; 
	goto done2;
    }
    
    
    k += (hx>>20) - 1023;
    hx &= 0x000fffff;
    i = (hx+0x95f64)&0x100000;
    SET_HIGH_WORD(x, hx|(i^0x3ff00000));  /* normalize x or x/2 */
    k += i>>20;
    f = x - 1.0;
    
    if ((0x000fffff&(2+hx)) < 3) {  /* -2**-20 <= f < 2**-20 */
	if (f == 0.0) 
	{
	    if (k == 0) 
	    {
		*ret = 0.0; 
		goto done2;
		//return 0.0;
	    }
	    dk = (double)k;
	    *ret = dk*ln2hi + dk*ln2lo; 
	    goto done2;
	    //return dk*ln2_hi + dk*ln2_lo;
	}
	
	R = f*f*(0.5-0.33333333333333333*f);
	if (k == 0) 
	{
	    *ret = f - R; goto done2;
	}
	
	//return f - R;
	dk = (double)k;
	*ret = dk*ln2hi - ((R-dk*ln2lo)-f); 
	goto done2;
	//return dk*ln2_hi - ((R-dk*ln2_lo)-f);
    }
    
    s = f/(2.0+f);
    dk = (double)k;
    z = s*s;
    i = hx - 0x6147a;
    w = z*z;
    j = 0x6b851 - hx;
    t1 = w*(Lg2+w*(Lg4+w*Lg6));
    t2 = z*(Lg1+w*(Lg3+w*(Lg5+w*Lg7)));
    i |= j;
    R = t2 + t1;
    
    if (i > 0) 
    {
	hfsq = 0.5*f*f;

	if (k == 0)
	{
	    //return f - (hfsq-s*(hfsq+R));
	    *ret = f - (hfsq-s*(hfsq+R)); 
	    goto done2;
	}
	
	*ret = dk*ln2hi - ((hfsq-(s*(hfsq+R)+dk*ln2lo))-f); 
	goto done2;
	//return dk*ln2_hi - ((hfsq-(s*(hfsq+R)+dk*ln2_lo))-f);
	
    } 
    else 
    {
	if (k == 0)
	{
	    //return f - s*(f-R);
	    *ret = f - s*(f-R); 
	    goto done2;
	}
	
	*ret = dk*ln2hi - ((s*(f-R)-dk*ln2lo)-f); 
	goto done2;
	//return dk*ln2_hi - ((s*(f-R)-dk*ln2_lo)-f);
    }

done2:
    //printk(KERN_INFO "mlog = %i\n", (int) (*ret*1000000.0));
    return;
}

void mpow(double x, double y, double* ret)
{
	double z,ax,z_h,z_l,p_h,p_l;
	double y1,t1,t2,r,s,t,u,v,w;
	int32_t i,j,k,yisint,n;
	int32_t hx,hy,ix,iy;
	uint32_t lx,ly;

	EXTRACT_WORDS(hx, lx, x);
	EXTRACT_WORDS(hy, ly, y);
	ix = hx & 0x7fffffff;
	iy = hy & 0x7fffffff;

	/* x**0 = 1, even if x is NaN */
	if ((iy|ly) == 0) {
		*ret = 1.0;
		goto done;
	}
	/* 1**y = 1, even if y is NaN */
	if (hx == 0x3ff00000 && lx == 0)
	{
		*ret = 1.0;
		goto done;
	}
	/* NaN if either arg is NaN */
	if (ix > 0x7ff00000 || (ix == 0x7ff00000 && lx != 0) ||
	    iy > 0x7ff00000 || (iy == 0x7ff00000 && ly != 0))
	{
		*ret =  x + y;
		goto done;
	}

	/* determine if y is an odd int when x < 0
	 * yisint = 0       ... y is not an integer
	 * yisint = 1       ... y is an odd int
	 * yisint = 2       ... y is an even int
	 */
	yisint = 0;
	if (hx < 0) {
		if (iy >= 0x43400000)
			yisint = 2; /* even integer y */
		else if (iy >= 0x3ff00000) {
			k = (iy>>20) - 0x3ff;  /* exponent */
			if (k > 20) {
				j = ly>>(52-k);
				if ((j<<(52-k)) == ly)
					yisint = 2 - (j&1);
			} else if (ly == 0) {
				j = iy>>(20-k);
				if ((j<<(20-k)) == iy)
					yisint = 2 - (j&1);
			}
		}
	}

	/* special value of y */
	if (ly == 0) {
		if (iy == 0x7ff00000) {  /* y is +-inf */
			if (((ix-0x3ff00000)|lx) == 0)  /* (-1)**+-inf is 1 */
			{
			    *ret =  1.0;
			    goto done;
			}
			else if (ix >= 0x3ff00000) /* (|x|>1)**+-inf = inf,0 */
			{
				*ret =  hy >= 0 ? y : 0.0;
				goto done;
			}
			else                       /* (|x|<1)**+-inf = 0,inf */
			{
				*ret =  hy >= 0 ? 0.0 : -y;
				goto done;
			}
		}
		if (iy == 0x3ff00000)    /* y is +-1 */
		{
			*ret =  hy >= 0 ? x : 1.0/x;
			goto done;
		}
		if (hy == 0x40000000)    /* y is 2 */
		{
			*ret =  x*x;
			goto done;
		}
		if (hy == 0x3fe00000) {  /* y is 0.5 */
			if (hx >= 0)     /* x >= +0 */
			{
			    msqrt(x, ret);
			    goto done;
			}
		}
	}

        mfabs(x, &ax);
	
	/* special value of x */
	if (lx == 0) {
		if (ix == 0x7ff00000 || ix == 0 || ix == 0x3ff00000) { /* x is +-0,+-inf,+-1 */
			z = ax;
			if (hy < 0)   /* z = (1/|x|) */
				z = 1.0/z;
			if (hx < 0) {
				if (((ix-0x3ff00000)|yisint) == 0) {
					z = (z-z)/(z-z); /* (-1)**non-int is NaN */
				} else if (yisint == 1)
					z = -z;          /* (x<0)**odd = -(|x|**odd) */
			}
			*ret =  z;
			goto done;
		}
	}

	s = 1.0; /* sign of result */
	if (hx < 0) {
		if (yisint == 0) /* (x<0)**(non-int) is NaN */
		{
			*ret =  (x-x)/(x-x);
			goto done;
		}
		if (yisint == 1) /* (x<0)**(odd int) */
			s = -1.0;
	}

	/* |y| is huge */
	if (iy > 0x41e00000) { /* if |y| > 2**31 */
		if (iy > 0x43f00000) {  /* if |y| > 2**64, must o/uflow */
			if (ix <= 0x3fefffff)
			{
				*ret =  hy < 0 ? huge*huge : tiny*tiny;
				goto done;
			}
			if (ix >= 0x3ff00000)
			{
				*ret =  hy > 0 ? huge*huge : tiny*tiny;
				goto done;
			}
		}
		/* over/underflow if x is not close to one */
		if (ix < 0x3fefffff)
		{
			*ret =  hy < 0 ? s*huge*huge : s*tiny*tiny;
			goto done;
		}
		if (ix > 0x3ff00000)
		{
			*ret =  hy > 0 ? s*huge*huge : s*tiny*tiny;
			goto done;
		}
		/* now |1-x| is tiny <= 2**-20, suffice to compute
		   log(x) by x-x^2/2+x^3/3-x^4/4 */
		t = ax - 1.0;       /* t has 20 trailing zeros */
		w = (t*t)*(0.5 - t*(0.3333333333333333333333-t*0.25));
		u = ivln2_h*t;      /* ivln2_h has 21 sig. bits */
		v = t*ivln2_l - w*ivln2;
		t1 = u + v;
		SET_LOW_WORD(t1, 0);
		t2 = v - (t1-u);
	} else {
		double ss,s2,s_h,s_l,t_h,t_l;
		n = 0;
		/* take care subnormal number */
		if (ix < 0x00100000) {
			ax *= two53;
			n -= 53;
			GET_HIGH_WORD(ix,ax);
		}
		n += ((ix)>>20) - 0x3ff;
		j = ix & 0x000fffff;
		/* determine interval */
		ix = j | 0x3ff00000;   /* normalize ix */
		if (j <= 0x3988E)      /* |x|<sqrt(3/2) */
			k = 0;
		else if (j < 0xBB67A)  /* |x|<sqrt(3)   */
			k = 1;
		else {
			k = 0;
			n += 1;
			ix -= 0x00100000;
		}
		SET_HIGH_WORD(ax, ix);

		/* compute ss = s_h+s_l = (x-1)/(x+1) or (x-1.5)/(x+1.5) */
		u = ax - bp[k];        /* bp[0]=1.0, bp[1]=1.5 */
		v = 1.0/(ax+bp[k]);
		ss = u*v;
		s_h = ss;
		SET_LOW_WORD(s_h, 0);
		/* t_h=ax+bp[k] High */
		t_h = 0.0;
		SET_HIGH_WORD(t_h, ((ix>>1)|0x20000000) + 0x00080000 + (k<<18));
		t_l = ax - (t_h-bp[k]);
		s_l = v*((u-s_h*t_h)-s_h*t_l);
		/* compute log(ax) */
		s2 = ss*ss;
		r = s2*s2*(L1+s2*(L2+s2*(L3+s2*(L4+s2*(L5+s2*L6)))));
		r += s_l*(s_h+ss);
		s2 = s_h*s_h;
		t_h = 3.0 + s2 + r;
		SET_LOW_WORD(t_h, 0);
		t_l = r - ((t_h-3.0)-s2);
		/* u+v = ss*(1+...) */
		u = s_h*t_h;
		v = s_l*t_h + t_l*ss;
		/* 2/(3log2)*(ss+...) */
		p_h = u + v;
		SET_LOW_WORD(p_h, 0);
		p_l = v - (p_h-u);
		z_h = cp_h*p_h;        /* cp_h+cp_l = 2/(3*log2) */
		z_l = cp_l*p_h+p_l*cp + dp_l[k];
		/* log2(ax) = (ss+..)*2/(3*log2) = n + dp_h + z_h + z_l */
		t = (double)n;
		t1 = ((z_h + z_l) + dp_h[k]) + t;
		SET_LOW_WORD(t1, 0);
		t2 = z_l - (((t1 - t) - dp_h[k]) - z_h);
	}

	/* split up y into y1+y2 and compute (y1+y2)*(t1+t2) */
	y1 = y;
	SET_LOW_WORD(y1, 0);
	p_l = (y-y1)*t1 + y*t2;
	p_h = y1*t1;
	z = p_l + p_h;
	EXTRACT_WORDS(j, i, z);
	if (j >= 0x40900000) {                      /* z >= 1024 */
		if (((j-0x40900000)|i) != 0)        /* if z > 1024 */
		{
			*ret =  s*huge*huge;         /* overflow */
			goto done;
		}
		if (p_l + ovt > z - p_h)
		{
			*ret =  s*huge*huge;         /* overflow */
			goto done;
		}
	} else if ((j&0x7fffffff) >= 0x4090cc00) {  /* z <= -1075 */  // FIXME: instead of abs(j) use unsigned j
		if (((j-0xc090cc00)|i) != 0)        /* z < -1075 */
		{
			*ret =  s*tiny*tiny;         /* underflow */
			goto done;
		}
		if (p_l <= z - p_h)
		{
			*ret =  s*tiny*tiny;         /* underflow */
			goto done;
		}
	}
	/*
	 * compute 2**(p_h+p_l)
	 */
	i = j & 0x7fffffff;
	k = (i>>20) - 0x3ff;
	n = 0;
	if (i > 0x3fe00000) {  /* if |z| > 0.5, set n = [z+0.5] */
		n = j + (0x00100000>>(k+1));
		k = ((n&0x7fffffff)>>20) - 0x3ff;  /* new k for n */
		t = 0.0;
		SET_HIGH_WORD(t, n & ~(0x000fffff>>k));
		n = ((n&0x000fffff)|0x00100000)>>(20-k);
		if (j < 0)
			n = -n;
		p_h -= t;
	}
	t = p_l + p_h;
	SET_LOW_WORD(t, 0);
	u = t*lg2_h;
	v = (p_l-(t-p_h))*lg2 + t*lg2_l;
	z = u + v;
	w = v - (z-u);
	t = z*z;
	t1 = z - t*(P1+t*(P2+t*(P3+t*(P4+t*P5))));
	r = (z*t1)/(t1-2.0) - (w + z*w);
	z = 1.0 - (r-z);
	GET_HIGH_WORD(j, z);
	j += n<<20;
	if ((j>>20) <= 0)  /* subnormal output */
	    mscalbn(z,n, &z);
	else
		SET_HIGH_WORD(z, j);
	*ret =  s*z;
	goto done;

done:
	return;
}

void msqrt(double x, double* ret)
{
	double z;
	int32_t sign = (int)0x80000000;
	int32_t ix0,s0,q,m,t,i;
	uint32_t r,t1,s1,ix1,q1;

	EXTRACT_WORDS(ix0, ix1, x);

	/* take care of Inf and NaN */
	if ((ix0&0x7ff00000) == 0x7ff00000) {
		*ret = x*x + x;  /* sqrt(NaN)=NaN, sqrt(+inf)=+inf, sqrt(-inf)=sNaN */
		goto done;
	}
	/* take care of zero */
	if (ix0 <= 0) {
		if (((ix0&~sign)|ix1) == 0)
		{
			*ret = x;  /* sqrt(+-0) = +-0 */
			goto done;
		}
		if (ix0 < 0) 
		{
			*ret = (x-x)/(x-x);  /* sqrt(-ve) = sNaN */
			goto done;
		}
	}
	/* normalize x */
	m = ix0>>20;
	if (m == 0) {  /* subnormal x */
		while (ix0 == 0) {
			m -= 21;
			ix0 |= (ix1>>11);
			ix1 <<= 21;
		}
		for (i=0; (ix0&0x00100000) == 0; i++)
			ix0<<=1;
		m -= i - 1;
		ix0 |= ix1>>(32-i);
		ix1 <<= i;
	}
	m -= 1023;    /* unbias exponent */
	ix0 = (ix0&0x000fffff)|0x00100000;
	if (m & 1) {  /* odd m, double x to make it even */
		ix0 += ix0 + ((ix1&sign)>>31);
		ix1 += ix1;
	}
	m >>= 1;      /* m = [m/2] */

	/* generate sqrt(x) bit by bit */
	ix0 += ix0 + ((ix1&sign)>>31);
	ix1 += ix1;
	q = q1 = s0 = s1 = 0;  /* [q,q1] = sqrt(x) */
	r = 0x00200000;        /* r = moving bit from right to left */

	while (r != 0) {
		t = s0 + r;
		if (t <= ix0) {
			s0   = t + r;
			ix0 -= t;
			q   += r;
		}
		ix0 += ix0 + ((ix1&sign)>>31);
		ix1 += ix1;
		r >>= 1;
	}

	r = sign;
	while (r != 0) {
		t1 = s1 + r;
		t  = s0;
		if (t < ix0 || (t == ix0 && t1 <= ix1)) {
			s1 = t1 + r;
			if ((t1&sign) == sign && (s1&sign) == 0)
				s0++;
			ix0 -= t;
			if (ix1 < t1)
				ix0--;
			ix1 -= t1;
			q1 += r;
		}
		ix0 += ix0 + ((ix1&sign)>>31);
		ix1 += ix1;
		r >>= 1;
	}

	/* use floating add to find out rounding direction */
	if ((ix0|ix1) != 0) {
		z = 1.0 - tiny; /* raise inexact flag */
		if (z >= 1.0) {
			z = 1.0 + tiny;
			if (q1 == (uint32_t)0xffffffff) {
				q1 = 0;
				q++;
			} else if (z > 1.0) {
				if (q1 == (uint32_t)0xfffffffe)
					q++;
				q1 += 2;
			} else
				q1 += q1 & 1;
		}
	}
	ix0 = (q>>1) + 0x3fe00000;
	ix1 = q1>>1;
	if (q&1)
		ix1 |= sign;
	ix0 += m << 20;
	INSERT_WORDS(z, ix0, ix1);
	*ret = z;
	goto done;

done:
	return;
}

void mfabs(double x, double* ret)
{
	union dshape u;

	u.value = x;
	u.bits &= (uint64_t)-1 / 2;
        *ret = u.value;
	return;
}

/* caller must handle the case when reduction is not needed: |x| ~<= pi/4 */
int __mrem_pio2(double x, double *y)
{
	union {double f; uint64_t i;} u = {x};
	double z,w,t,r,fn;
	double tx[3],ty[2];
	uint32_t ix;
	int sign, n, ex, ey, i;

	sign = u.i>>63;
	ix = u.i>>32 & 0x7fffffff;
	if (ix <= 0x400f6a7a) {  /* |x| ~<= 5pi/4 */
		if ((ix & 0xfffff) == 0x921fb)  /* |x| ~= pi/2 or 2pi/2 */
			goto medium;  /* cancellation -- use medium case */
		if (ix <= 0x4002d97c) {  /* |x| ~<= 3pi/4 */
			if (!sign) {
				z = x - pio2_1;  /* one round good to 85 bits */
				y[0] = z - pio2_1t;
				y[1] = (z-y[0]) - pio2_1t;
				return 1;
			} else {
				z = x + pio2_1;
				y[0] = z + pio2_1t;
				y[1] = (z-y[0]) + pio2_1t;
				return -1;
			}
		} else {
			if (!sign) {
				z = x - 2*pio2_1;
				y[0] = z - 2*pio2_1t;
				y[1] = (z-y[0]) - 2*pio2_1t;
				return 2;
			} else {
				z = x + 2*pio2_1;
				y[0] = z + 2*pio2_1t;
				y[1] = (z-y[0]) + 2*pio2_1t;
				return -2;
			}
		}
	}
	if (ix <= 0x401c463b) {  /* |x| ~<= 9pi/4 */
		if (ix <= 0x4015fdbc) {  /* |x| ~<= 7pi/4 */
			if (ix == 0x4012d97c)  /* |x| ~= 3pi/2 */
				goto medium;
			if (!sign) {
				z = x - 3*pio2_1;
				y[0] = z - 3*pio2_1t;
				y[1] = (z-y[0]) - 3*pio2_1t;
				return 3;
			} else {
				z = x + 3*pio2_1;
				y[0] = z + 3*pio2_1t;
				y[1] = (z-y[0]) + 3*pio2_1t;
				return -3;
			}
		} else {
			if (ix == 0x401921fb)  /* |x| ~= 4pi/2 */
				goto medium;
			if (!sign) {
				z = x - 4*pio2_1;
				y[0] = z - 4*pio2_1t;
				y[1] = (z-y[0]) - 4*pio2_1t;
				return 4;
			} else {
				z = x + 4*pio2_1;
				y[0] = z + 4*pio2_1t;
				y[1] = (z-y[0]) + 4*pio2_1t;
				return -4;
			}
		}
	}
	if (ix < 0x413921fb) {  /* |x| ~< 2^20*(pi/2), medium size */
medium:
		/* rint(x/(pi/2)), Assume round-to-nearest. */
		fn = (double)x*invpio2 + toint - toint;
		n = (int32_t)fn;
		r = x - fn*pio2_1;
		w = fn*pio2_1t;  /* 1st round, good to 85 bits */
		y[0] = r - w;
		u.f = y[0];
		ey = u.i>>52 & 0x7ff;
		ex = ix>>20;
		if (ex - ey > 16) { /* 2nd round, good to 118 bits */
			t = r;
			w = fn*pio2_2;
			r = t - w;
			w = fn*pio2_2t - ((t-r)-w);
			y[0] = r - w;
			u.f = y[0];
			ey = u.i>>52 & 0x7ff;
			if (ex - ey > 49) {  /* 3rd round, good to 151 bits, covers all cases */
				t = r;
				w = fn*pio2_3;
				r = t - w;
				w = fn*pio2_3t - ((t-r)-w);
				y[0] = r - w;
			}
		}
		y[1] = (r - y[0]) - w;
		return n;
	}
	/*
	 * all other (large) arguments
	 */
	if (ix >= 0x7ff00000) {  /* x is inf or NaN */
		y[0] = y[1] = x - x;
		return 0;
	}
	/* set z = scalbn(|x|,-ilogb(x)+23) */
	u.f = x;
	u.i &= (uint64_t)-1>>12;
	u.i |= (uint64_t)(0x3ff + 23)<<52;
	z = u.f;
	for (i=0; i < 2; i++) {
		tx[i] = (double)(int32_t)z;
		z     = (z-tx[i])*0x1p24;
	}
	tx[i] = z;
	/* skip zero terms, first term is non-zero */
	while (tx[i] == 0.0)
		i--;
	n = __mrem_pio2_large(tx,ty,(int)(ix>>20)-(0x3ff+23),i+1,1);
	if (sign) {
		y[0] = -ty[0];
		y[1] = -ty[1];
		return -n;
	}
	y[0] = ty[0];
	y[1] = ty[1];
	return n;
}

void __mcos(double x, double y, double* ret)
{
	double hz,z,r,w;

	z  = x*x;
	w  = z*z;
	r  = z*(C1+z*(C2+z*C3)) + w*w*(C4+z*(C5+z*C6));
	hz = 0.5*z;
	w  = 1.0-hz;
        *ret = w + (((1.0-w)-hz) + (z*r-x*y));
	return;
}

void __msin(double x, double y, int iy, double* ret)
{
	double z,r,v,w;

	z = x*x;
	w = z*z;
	r = S2 + z*(S3 + z*S4) + z*w*(S5 + z*S6);
	v = z*x;
	if (iy == 0)
	{
	    *ret = x + v*(S1 + z*r);
	    return;
	}
	else
	{
	    *ret = x - ((z*(0.5*y - v*r) - y) - v*S1);
	    return;
	}
}

int __mrem_pio2_large(double *x, double *y, int e0, int nx, int prec)
{
	int32_t jz,jx,jv,jp,jk,carry,n,iq[20],i,j,k,m,q0,ih;
	double z,fw,f[20],fq[20],q[20],tmp;

	/* initialize jk*/
	jk = init_jk[prec];
	jp = jk;

	/* determine jx,jv,q0, note that 3>q0 */
	jx = nx-1;
	jv = (e0-3)/24;  if(jv<0) jv=0;
	q0 = e0-24*(jv+1);

	/* set up f[0] to f[jx+jk] where f[jx+jk] = ipio2[jv+jk] */
	j = jv-jx; m = jx+jk;
	for (i=0; i<=m; i++,j++)
		f[i] = j<0 ? 0.0 : (double)ipio2[j];

	/* compute q[0],q[1],...q[jk] */
	for (i=0; i<=jk; i++) {
		for (j=0,fw=0.0; j<=jx; j++)
			fw += x[j]*f[jx+i-j];
		q[i] = fw;
	}

	jz = jk;
recompute:
	/* distill q[] into iq[] reversingly */
	for (i=0,j=jz,z=q[jz]; j>0; i++,j--) {
		fw    = (double)(int32_t)(0x1p-24*z);
		iq[i] = (int32_t)(z - 0x1p24*fw);
		z     = q[j-1]+fw;
	}

	/* compute n */
	//z  = scalbn(z,q0);       /* actual value of z */
	mscalbn(z, q0, &z);
	
	mfloor(z*0.125, &tmp);
	
	//z -= 8.0*floor(z*0.125); /* trim off integer >= 8 */
	z -= 8.0 * tmp;
	
	n  = (int32_t)z;
	z -= (double)n;
	ih = 0;
	if (q0 > 0) {  /* need iq[jz-1] to determine n */
		i  = iq[jz-1]>>(24-q0); n += i;
		iq[jz-1] -= i<<(24-q0);
		ih = iq[jz-1]>>(23-q0);
	}
	else if (q0 == 0) ih = iq[jz-1]>>23;
	else if (z >= 0.5) ih = 2;

	if (ih > 0) {  /* q > 0.5 */
		n += 1; carry = 0;
		for (i=0; i<jz; i++) {  /* compute 1-q */
			j = iq[i];
			if (carry == 0) {
				if (j != 0) {
					carry = 1;
					iq[i] = 0x1000000 - j;
				}
			} else
				iq[i] = 0xffffff - j;
		}
		if (q0 > 0) {  /* rare case: chance is 1 in 12 */
			switch(q0) {
			case 1:
				iq[jz-1] &= 0x7fffff; break;
			case 2:
				iq[jz-1] &= 0x3fffff; break;
			}
		}
		if (ih == 2) {
			z = 1.0 - z;
			if (carry != 0)
			{
			    double tmp;
			    mscalbn(1.0, q0, &tmp);
			    z -= tmp;
			    //z -= scalbn(1.0,q0);
			}
		}
	}

	/* check if recomputation is needed */
	if (z == 0.0) {
		j = 0;
		for (i=jz-1; i>=jk; i--) j |= iq[i];
		if (j == 0) {  /* need recomputation */
			for (k=1; iq[jk-k]==0; k++);  /* k = no. of terms needed */

			for (i=jz+1; i<=jz+k; i++) {  /* add q[jz+1] to q[jz+k] */
				f[jx+i] = (double)ipio2[jv+i];
				for (j=0,fw=0.0; j<=jx; j++)
					fw += x[j]*f[jx+i-j];
				q[i] = fw;
			}
			jz += k;
			goto recompute;
		}
	}

	/* chop off zero terms */
	if (z == 0.0) {
		jz -= 1;
		q0 -= 24;
		while (iq[jz] == 0) {
			jz--;
			q0 -= 24;
		}
	} else { /* break z into 24-bit if necessary */
	    //z = scalbn(z,-q0);
	    mscalbn(z, -q0, &z);
		if (z >= 0x1p24) {
			fw = (double)(int32_t)(0x1p-24*z);
			iq[jz] = (int32_t)(z - 0x1p24*fw);
			jz += 1;
			q0 += 24;
			iq[jz] = (int32_t)fw;
		} else
			iq[jz] = (int32_t)z;
	}

	/* convert integer "bit" chunk to floating-point value */
	//fw = scalbn(1.0,q0);
	mscalbn(1.0, q0, &fw);
	for (i=jz; i>=0; i--) {
		q[i] = fw*(double)iq[i];
		fw *= 0x1p-24;
	}

	/* compute PIo2[0,...,jp]*q[jz,...,0] */
	for(i=jz; i>=0; i--) {
		for (fw=0.0,k=0; k<=jp && k<=jz-i; k++)
			fw += PIo2[k]*q[i+k];
		fq[jz-i] = fw;
	}

	/* compress fq[] into y[] */
	switch(prec) {
	case 0:
		fw = 0.0;
		for (i=jz; i>=0; i--)
			fw += fq[i];
		y[0] = ih==0 ? fw : -fw;
		break;
	case 1:
	case 2:
		fw = 0.0;
		for (i=jz; i>=0; i--)
			fw += fq[i];
		// TODO: drop excess precision here once double_t is used
		fw = (double)fw;
		y[0] = ih==0 ? fw : -fw;
		fw = fq[0]-fw;
		for (i=1; i<=jz; i++)
			fw += fq[i];
		y[1] = ih==0 ? fw : -fw;
		break;
	case 3:  /* painful */
		for (i=jz; i>0; i--) {
			fw      = fq[i-1]+fq[i];
			fq[i]  += fq[i-1]-fw;
			fq[i-1] = fw;
		}
		for (i=jz; i>1; i--) {
			fw      = fq[i-1]+fq[i];
			fq[i]  += fq[i-1]-fw;
			fq[i-1] = fw;
		}
		for (fw=0.0,i=jz; i>=2; i--)
			fw += fq[i];
		if (ih==0) {
			y[0] =  fq[0]; y[1] =  fq[1]; y[2] =  fw;
		} else {
			y[0] = -fq[0]; y[1] = -fq[1]; y[2] = -fw;
		}
	}
	return n&7;
}

void mfloor(double x, double* ret)
{
	union {double f; uint64_t i;} u = {x};
	int e = u.i >> 52 & 0x7ff;
	double y;

	if (e >= 0x3ff+52 || x == 0)
	{
	    *ret = x;
	    goto done;
	    //return x;
	}
	/* y = int(x) - x, where int(x) is an integer neighbor of x */
	if (u.i >> 63)
		y = x - toint2 + toint2 - x;
	else
		y = x + toint2 - toint2 - x;
	/* special case because of non-nearest rounding modes */
	if (e <= 0x3ff-1) 
	{
	    FORCE_EVAL(y);
	    *ret = u.i >> 63 ? -1 : 0;
	    goto done;
		//return u.i >> 63 ? -1 : 0;
	}
	if (y > 0)
	{
	    *ret = x + y - 1;
	    goto done;
	    //return x + y - 1;
	}
	*ret = x + y;
	goto done;
	//return x + y;

done:
	return;
}

void msin(double x, double* ret)
{
	double y[2];
	uint32_t ix;
	unsigned n;
	double tmp;
	
	/* High word of x. */
	GET_HIGH_WORD(ix, x);
	ix &= 0x7fffffff;

	/* |x| ~< pi/4 */
	if (ix <= 0x3fe921fb) {
		if (ix < 0x3e500000) {  /* |x| < 2**-26 */
			/* raise inexact if x != 0 and underflow if subnormal*/
			FORCE_EVAL(ix < 0x00100000 ? x/0x1p120f : x+0x1p120f);
			*ret = x;
			goto done;
			//return x;
		}
		//return __sin(x, 0.0, 0);
		__msin(x, 0.0, 0, ret);
		goto done;
	}

	/* sin(Inf or NaN) is NaN */
	if (ix >= 0x7ff00000)
	{
	    //return x - x;
	    *ret = x - x;
	    goto done;
	}

	/* argument reduction needed */
	n = __mrem_pio2(x, y);
	switch (n&3) 
	{
	case 0: 
	    //return  __sin(y[0], y[1], 1);
	    __msin(y[0], y[1], 1, ret);
	    goto done;
	case 1: 
	    //return  __cos(y[0], y[1]);
	    __mcos(y[0], y[1], ret);
	    goto done;
	case 2: 
	    //return -__sin(y[0], y[1], 1);
	    __msin(y[0], y[1], 1, &tmp);
	    *ret = -tmp;
	    goto done;
	default:
	    //return -__cos(y[0], y[1]);
	    __mcos(y[0], y[1], &tmp);
	    *ret = -tmp;
	    goto done;
	}

done:
	return;
}

void mcos(double x, double* ret)
{
	double y[2];
	uint32_t ix;
	unsigned n;
	double tmp;
	
	GET_HIGH_WORD(ix, x);
	ix &= 0x7fffffff;

	/* |x| ~< pi/4 */
	if (ix <= 0x3fe921fb) {
		if (ix < 0x3e46a09e) {  /* |x| < 2**-27 * sqrt(2) */
			/* raise inexact if x!=0 */
			FORCE_EVAL(x + 0x1p120f);
			*ret = 1.0;
			return;
		}
		//return __cos(x, 0);
		__mcos(x, 0, ret);
		return;
	}

	/* cos(Inf or NaN) is NaN */
	if (ix >= 0x7ff00000)
	{
	    //return x-x;
	    *ret = x - x;
	    return;
	}

	/* argument reduction */
	n = __mrem_pio2(x, y);
	switch (n&3) {
	case 0: 
	    __mcos(y[0], y[1], ret);
	    return;
	    //return  __cos(y[0], y[1]);
	case 1: 
	    __msin(y[0], y[1], 1, &tmp);
	    *ret = -tmp;
	    return;
	    //return -__sin(y[0], y[1], 1);
	case 2: 
	    __mcos(y[0], y[1], &tmp);
	    *ret = -tmp;
	    return;
	    //return -__cos(y[0], y[1]);
	default:
	    __msin(y[0], y[1], 1, ret);
	    return;
	    //return  __sin(y[0], y[1], 1);
	}
	return;
}

void mmodf(double x, double *iptr, double *ret)
{
	union {double f; uint64_t i;} u = {x};
	uint64_t mask;
	int e = (int)(u.i>>52 & 0x7ff) - 0x3ff;

	/* no fractional part */
	if (e >= 52) {
		*iptr = x;
		if (e == 0x400 && u.i<<12 != 0) /* nan */
		{
		    *ret = x;
		    return;
		}
		u.i &= 1ULL<<63;
		*ret = u.f;
		return;
	}

	/* no integral part*/
	if (e < 0) {
		u.i &= 1ULL<<63;
		*iptr = u.f;
		*ret = x;
		return;
	}

	mask = -1ULL>>12>>e;
	if ((u.i & mask) == 0) {
		*iptr = x;
		u.i &= 1ULL<<63;
		*ret = u.f;
	}
	u.i &= ~mask;
	*iptr = u.f;
	*ret = x - u.f;
	return;
}

int icomp(double x)
{
    double fracp, intp;
    
    mmodf(x, &intp, &fracp);

    return (int)intp;
}

int fcomp(double x)
{
    double fracp, intp;
    
    mmodf(x, &intp, &fracp);

    if(fracp < 0.1)
    {
	return -1*(int)(fracp * 1000000.0);
    }
    else
    {
	return (int)(fracp * 1000000.0);
    }
}

void mftoa(double f, char * buf, int precision)
{
	char * ptr = buf;
	char * p = ptr;
	char * p1;
	char c;
	long intPart;

	// check precision bounds
	if (precision > MAX_PRECISION)
		precision = MAX_PRECISION;

	// sign stuff
	if (f < 0)
	{
		f = -f;
		*ptr++ = '-';
	}

	if (precision < 0)  // negative precision == automatic precision guess
	{
		if (f < 1.0) precision = 6;
		else if (f < 10.0) precision = 5;
		else if (f < 100.0) precision = 4;
		else if (f < 1000.0) precision = 3;
		else if (f < 10000.0) precision = 2;
		else if (f < 100000.0) precision = 1;
		else precision = 0;
	}

	// round value according the precision
	if (precision)
		f += rounders[precision];

	// integer part...
	intPart = f;
	f -= intPart;

	if (!intPart)
		*ptr++ = '0';
	else
	{
		// save start pointer
		p = ptr;

		// convert (reverse order)
		while (intPart)
		{
			*p++ = '0' + intPart % 10;
			intPart /= 10;
		}

		// save end pos
		p1 = p;

		// reverse result
		while (p > ptr)
		{
			c = *--p;
			*p = *ptr;
			*ptr++ = c;
		}

		// restore end pos
		ptr = p1;
	}

	// decimal part
	if (precision)
	{
		// place decimal point
		*ptr++ = '.';

		// convert
		while (precision--)
		{
			f *= 10.0;
			c = f;
			*ptr++ = '0' + c;
			f -= c;
		}
	}

	// terminating zero
	*ptr = 0;

	return;
}
