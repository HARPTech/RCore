#ifndef LRT_LIBRCORE_DEFAULTS_H
#define LRT_LIBRCORE_DEFAULTS_H

#include "rcomm.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define LRT_RCOMM_BLOCKSIZE 16
#define LRT_RCOMM_UNIVERSAL_DEFINITIONS()      \
  LRT_RCORE_RCOMM_DEFINE_PROTOCOL_DEFINITIONS( \
    rcomm, LRT_RCOMM_BLOCKSIZE, LRT_LIBRSP_STREAM_MESSAGE, 6u, 16u, 128u)
#define LRT_RCOMM_UNIVERSAL()      \
  LRT_RCORE_RCOMM_DEFINE_PROTOCOL( \
    rcomm, LRT_RCOMM_BLOCKSIZE, LRT_LIBRSP_STREAM_MESSAGE, 6u, 16u, 128u)

  typedef struct rcomm_handle_t rcomm_handle_t;

#define LRT_RCOMM_SPI_BLOCKSIZE LRT_RCOMM_BLOCKSIZE
#define LRT_RCOMM_SPI()                                      \
  LRT_RCORE_RCOMM_DEFINE_PROTOCOL(rcomm_spi,                 \
                                  LRT_RCOMM_SPI_BLOCKSIZE,   \
                                  LRT_LIBRSP_STREAM_MESSAGE, \
                                  2u,                        \
                                  4u,                        \
                                  32u)

#define LRT_RCOMM_WIFI_BLOCKSIZE LRT_RCOMM_BLOCKSIZE
#define LRT_RCOMM_WIFI()                                     \
  LRT_RCORE_RCOMM_DEFINE_PROTOCOL(rcomm_wifi,                \
                                  LRT_RCOMM_WIFI_BLOCKSIZE,  \
                                  LRT_LIBRSP_STREAM_MESSAGE, \
                                  3u,                        \
                                  8u,                        \
                                  128u)

  // Crossovers for SPI and WIFI
#define LRT_RCOMM_WIFI_SPI_CROSSOVER() \
  LRT_RCORE_STACK_CROSSOVER_DUPLEX(    \
    rcomm_wifi, rcomm_spi, LRT_RCOMM_SPI_BLOCKSIZE, LRT_RCOMM_WIFI_BLOCKSIZE)

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
