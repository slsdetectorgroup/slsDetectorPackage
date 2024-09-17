#include "pm_decode.h"
#include "thread_utils.h"

void thread_pmdecode(void *args) {
    thread_args *a;
    a = (thread_args *)args;
    pm_decode(a->src, a->dst, a->pm, a->n_frames, a->n_pixels);
}

void pm_decode(uint16_t *src, uint16_t *dst, uint32_t *pm, size_t n_frames,
               size_t n_pixels) {
    for (size_t i = 0; i < n_frames; i++) {
        for (size_t j = 0; j < n_pixels; j++) {
            *dst++ = src[pm[j]];
        }
        src += n_pixels;
    }
}