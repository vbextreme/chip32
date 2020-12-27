#ifndef __SOS_TYPE_H__
#define __SOS_TYPE_H__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <errno.h>
#include <math.h>

typedef long ssize_t ;

#define KiB (1024UL)
#define MiB (KiB*KiB)
#define GiB (MiB*MiB)

#ifdef ARDUINO_ARCH_ESP32
#	define __ESP32
#	define DEFAULT_VREF 3300000
#	define DEFAULT_ADC_BITS 12
#	define DEFAULT_ADC_MED  8
#	define BUF_SIZE 256
#	define UART0_RX 3
#	define UART0_TX 1
#	define UART1_RX 3
#	define UART1_TX 1
#	define UART2_RX 3
#	define UART2_TX 1
#else
#	define __UNKNOW
#	define DEFAULT_ADC_BITS 10
#	define DEFAULT_VREF 5000000
#	define DEFAULT_ADC_MED 8 
#	define BUF_SIZE 16
#	define UART0_RX 0
#	define UART0_TX 1
#endif

#ifdef __unused
#undef __unused
#endif
#ifdef __packed
#undef __packed
#endif
#ifdef __const
#undef __const
#endif

#define __private static
#define __unused __attribute__((unused))
#define __cleanup(FNC) __attribute__((__cleanup__(FNC)))
#define __printf(FRMT,VA) __attribute__((format(printf, FRMT, VA)))
#define __const __attribute__((const))
#define __packed __attribute__((packed))
#define __weak __attribute__((weak))

#ifdef __cplusplus
#	define __extern_begin extern "C" {
#	define __extern_end }
#else
#	define __extern_begin
#	define __extern_end
#endif

#define DO_PRAGMA(DOP) _Pragma(#DOP)
#define UNSAFE_BEGIN(FLAGS) DO_PRAGMA(GCC diagnostic push); DO_PRAGMA(GCC diagnostic ignored FLAGS)
#define UNSAFE_END DO_PRAGMA(GCC diagnostic pop)

#define __error(STR) DO_PRAGMA(GCC error STR)
#define __warning(STR) DO_PRAGMA(GCC warning STR)


#define MANY(T,N) (T*)malloc(sizeof(T)*(N))
#define NEW(T)    MANY(T,1)
#define CLEAR(V,T,N) memset(V, 0, sizeof(T) * N)
#define ZERO(V,T) CLEAR(V,T,1)

#define ADDR(VAR) ((char*)(VAR))
#define ADDRTO(VAR, SO, I) ( ADDR(VAR) + ((SO)*(I)))

#define FAST_MOD_POW_TWO(N,M) ((N) & ((M) - 1))

#define ROUND_UP(N,S) ((((N)+(S)-1)/(S))*(S))

#define ROUND_UP_POW_TWO32(N) ({\
	   	unsigned int r = (N);\
	   	--r;\
		r |= r >> 1;\
		r |= r >> 2;\
		r |= r >> 4;\
		r |= r >> 8;\
		r |= r >> 16;\
		++r;\
		r;\
	})

#define SWAP(A,B) ({ \
		__auto_type __tmp__ = A;\
		A = B;\
		B = __tmp__;\
	})

#define VSTATIC_SIZE(V) (sizeof(V) / sizeof(V[0]))

#endif
