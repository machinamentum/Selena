#ifndef COMPILER_H
#define COMPILER_H

#ifdef __cplusplus
extern "C" {
#endif

void  SelenaSetErrorHandler(void (*ErrorFunc)(const char *));
char *SelenaCompileShaderSource(const char *Src, int *BinSize);

#ifdef __cplusplus
}
#endif

#endif
