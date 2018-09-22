#ifndef LRT_LIBRCP_MESSAGE_H
#define LRT_LIBRCP_MESSAGE_H

#include "../../../../librbp/include/RCore/librbp/message.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(__STDC_NO_THREADS__) || defined(NO_THREADS_H)
#define THREAD_LOCAL
#else
#include <threads.h>
#define THREAD_LOCAL thread_local
#endif

  /* Encodes the message type. The rest of the bits is held free for an inner
   * sequence number for dropping old messages in fast paced environments (many
   * updates per second). LiteComm Sequence numbers have to be handled by the
   * application, because it is not certain if data is important or can be
   * thrown away. */
  typedef enum lrt_rcp_message_type_t
  {
    LRT_RCP_MESSAGE_TYPE_UPDATE = 0b00000000,
    LRT_RCP_MESSAGE_TYPE_REQUEST = 0b10000000,
    LRT_RCP_MESSAGE_TYPE_SUBSCRIBE = 0b01000000,
    LRT_RCP_MESSAGE_TYPE_UNSUBSCRIBE = 0b11000000,
  } lrt_rcp_message_type_t;

#define LRT_RCP_MESSAGE_TYPE_COUNT 4

  inline const char* lrt_rcp_message_type_to_str(lrt_rcp_message_type_t msg)
  {
    switch(msg) {
      case LRT_RCP_MESSAGE_TYPE_UPDATE:
        return "Update";
      case LRT_RCP_MESSAGE_TYPE_REQUEST:
        return "Request";
      case LRT_RCP_MESSAGE_TYPE_SUBSCRIBE:
        return "Subscribe";
      case LRT_RCP_MESSAGE_TYPE_UNSUBSCRIBE:
        return "Unsubscribe";
      default:
        return "Unknown Message Type";
    }
  }

  inline size_t lrt_rcp_message_type_to_num(lrt_rcp_message_type_t msg)
  {
    return (size_t)(((uint8_t)msg) >> 6u);
  }

  inline lrt_rcp_message_type_t lrt_rcp_message_type_from_num(size_t num)
  {
    return (lrt_rcp_message_type_t)(num << 6u);
  }

  /* Conversion Unions
   * -----------------------------------------------------------------
   */

#define LRT_LIBRCP_CONVERSION_UNION(sTYPENAME, tTYPE)                \
  union lrt_librcp_##sTYPENAME##_t                                   \
  {                                                                  \
    tTYPE val;                                                       \
    uint8_t bytes[sizeof(tTYPE)];                                    \
  };                                                                 \
  static THREAD_LOCAL union lrt_librcp_##sTYPENAME##_t               \
    lrt_librcp_union_##sTYPENAME;                                    \
  const uint8_t* lrt_librcp_##sTYPENAME##_to_data(const tTYPE val);  \
  tTYPE lrt_librcp_##sTYPENAME##_from_data(const uint8_t* data,      \
                                           size_t length);           \
  void lrt_librcp_##sTYPENAME##_set_data(lrt_rbp_message_t* message, \
                                         const tTYPE val);           \
  tTYPE lrt_librcp_##sTYPENAME##_get_data(lrt_rbp_message_t* message);

  LRT_LIBRCP_CONVERSION_UNION(Bool, bool)
  LRT_LIBRCP_CONVERSION_UNION(Uint8, uint8_t)
  LRT_LIBRCP_CONVERSION_UNION(Int8, int8_t)
  LRT_LIBRCP_CONVERSION_UNION(Uint16, uint16_t)
  LRT_LIBRCP_CONVERSION_UNION(Int16, int16_t)
  LRT_LIBRCP_CONVERSION_UNION(Uint32, uint32_t)
  LRT_LIBRCP_CONVERSION_UNION(Int32, int32_t)
  LRT_LIBRCP_CONVERSION_UNION(Uint64, uint64_t)
  LRT_LIBRCP_CONVERSION_UNION(Int64, int64_t)
  LRT_LIBRCP_CONVERSION_UNION(Float, float)
  LRT_LIBRCP_CONVERSION_UNION(Double, double)

  inline lrt_rcp_message_type_t rcomm_get_litecomm_message_type(
    lrt_rbp_message_t* message)
  {
    return (lrt_rcp_message_type_t)(message->data[4] & 0b11000000u);
  }
  inline void rcomm_set_litecomm_message_type(lrt_rbp_message_t* message,
                                              lrt_rcp_message_type_t type)
  {
    message->data[4] = (message->data[4] & 0b00111111u) | type;
  }

#ifdef __cplusplus
}// closing brace for extern "C"
#endif

#endif
