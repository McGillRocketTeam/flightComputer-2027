#ifndef MRT_HBL_HBL_HELPERS_H_
#define MRT_HBL_HBL_HELPERS_H_

#ifdef __cplusplus
extern "C" {
#endif

// Standard includes
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

// Watch out for double-evaluation
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

// General prototypes
void              println(const char* s);
void              print(const char* s);
int               printf(const char* s, ...);
void              no_print(const char* s);       // Buffer print
int               no_printf(const char* s, ...); // Buffer printf
void              delay(int ms);
long unsigned int getMillis(void);

#ifdef __cplusplus
}
#endif

#endif /* MRT_HBL_HBL_HELPERS_H_ */
