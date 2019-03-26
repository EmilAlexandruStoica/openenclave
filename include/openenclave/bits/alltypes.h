/*
 * #define _Addr long
#define _Int64 long
#define _Reg long

#define TYPEDEF typedef
TYPEDEF __builtin_va_list va_list;
TYPEDEF __builtin_va_list __isoc_va_list;

#ifndef __cplusplus
TYPEDEF int wchar_t;
#endif

#if defined(__FLT_EVAL_METHOD__) && __FLT_EVAL_METHOD__ == 2
TYPEDEF long double float_t;
TYPEDEF long double double_t;
#else
TYPEDEF float float_t;
TYPEDEF double double_t;
#endif

TYPEDEF struct { long long __ll; long double __ld; } max_align_t;

TYPEDEF long time_t;
TYPEDEF long suseconds_t;

//TYPEDEF struct { union { int __i[14]; volatile int __vi[14]; unsigned long
__s[7]; } __u; } pthread_attr_t;
//TYPEDEF struct { union { int __i[10]; volatile int __vi[10]; volatile void
*volatile __p[5]; } __u; } pthread_mutex_t; TYPEDEF struct { union { int
__i[10]; volatile int __vi[10]; volatile void *volatile __p[5]; } __u; } mtx_t;
//TYPEDEF struct { union { int __i[12]; volatile int __vi[12]; void *__p[6]; }
__u; } pthread_cond_t; TYPEDEF struct { union { int __i[12]; volatile int
__vi[12]; void *__p[6]; } __u; } cnd_t;
//TYPEDEF struct { union { int __i[14]; volatile int __vi[14]; void *__p[7]; }
__u; } pthread_rwlock_t; TYPEDEF struct { union { int __i[8]; volatile int
__vi[8]; void *__p[4]; } __u; } pthread_barrier_t;
*/

#ifndef PID_T
#define PID_T
typedef uint32_t pid_t;
#endif /* PID_T */

#ifndef CLOCK_T
#define CLOCK_T
typedef uint32_t clock_t;
#endif /* CLOCK_T */