/*
 #ifndef FIXP_H
 #define FIXP_H
 
 typedef int32_t FixP_t;
 
 #define kIntegerPart 8
 
 #define  fixToInt(fp)  ((FixP_t)((fp) >> kIntegerPart))
 
 #define intToFix(v)  ((int32_t)((v) << kIntegerPart))
 
 #define Mul(v1, v2) ((FixP_t)((((v1) >> 3) * ((v2) >> 3)) >> 2))
 
 #define Div(v1, v2)  ((FixP_t)((((int32_t) (v1)) * (1 << kIntegerPart)) / (v2)))
 
 #endif

 */

#ifndef FIXP_H
#define FIXP_H

#ifndef FIX16
typedef int32_t FixP_t;
#else
typedef int32_t FixP_t;
#endif


#ifndef FIX16

int32_t
#else
int16_t
#endif
fixToInt(const FixP_t fp);

FixP_t intToFix(const
#ifndef FIX16
                int32_t
#else
                int16_t
#endif
                v);

FixP_t Mul(const FixP_t v1, const FixP_t v2);

FixP_t Div(const FixP_t v1, const FixP_t v2);

#endif
