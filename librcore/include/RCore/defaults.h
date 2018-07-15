#ifndef LRT_LIBRCORE_DEFAULTS_H
#define LRT_LIBRCORE_DEFAULTS_H

#include "rcomm.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define LRT_RCOMM_SPI_BLOCKSIZE 16
#define LRT_RCOMM_SPI()                                      \
  LRT_RCORE_RCOMM_DEFINE_PROTOCOL(rcomm_spi,                 \
                                  LRT_RCOMM_SPI_BLOCKSIZE,   \
                                  LRT_LIBRSP_STREAM_MESSAGE, \
                                  2u,                        \
                                  4u,                        \
                                  32u)

#define LRT_RCOMM_WIFI_BLOCKSIZE 16
#define LRT_RCOMM_WIFI()                                     \
  LRT_RCORE_RCOMM_DEFINE_PROTOCOL(rcomm_wifi,                \
                                  LRT_RCOMM_WIFI_BLOCKSIZE,  \
                                  LRT_LIBRSP_STREAM_MESSAGE, \
                                  3u,                        \
                                  8u,                        \
                                  128u)

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
