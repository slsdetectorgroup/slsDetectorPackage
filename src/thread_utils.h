#pragma once
#include <stdint.h>
#include <stddef.h>
typedef struct{
    uint16_t* src;
    uint16_t* dst; 
    uint32_t* pm;
    size_t n_frames;
    size_t n_pixels;
}thread_args;