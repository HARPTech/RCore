#ifndef LRT_RCORE_UTIL_HPP
#define LRT_RCORE_UTIL_HPP

#include "defaults.h"

#if defined(__cplusplus) && !defined(DISABLE_ADVANCED_CPP)
namespace lrt {
namespace RCore {
using RCommHandlePtr =
  std::unique_ptr<rcomm_handle_t, void (*)(rcomm_handle_t* handle)>;
inline RCommHandlePtr
CreateRCommHandlePtr(
  lrt_rcore_rcomm_config_t config = lrt_rcore_rcomm_universal_defaults)
{
  auto handle =
    RCommHandlePtr(rcomm_handle_create_from_config(config), &rcomm_free);
  return handle;
}
}
}
#endif

#endif
