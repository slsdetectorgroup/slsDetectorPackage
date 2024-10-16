#pragma once
#include <stddef.h>
#include <stdint.h>
// Wrapper to be used with pthreads
void thread_pmdecode(void *args);

void pm_decode(uint16_t *src, uint16_t *dst, uint32_t *pm, size_t n_frames,
               size_t n_pixels);