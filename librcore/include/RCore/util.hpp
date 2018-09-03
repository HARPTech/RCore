#ifndef LRT_RCORE_UTIL_HPP
#define LRT_RCORE_UTIL_HPP

#if defined(__cplusplus) && !defined(DISABLE_ADVANCED_CPP)
#define LRT_RCOMM_PTR(sPREFIX, sPREFIX_UPPER)                            \
  namespace lrt {                                                        \
  namespace RCore {                                                      \
  using sPREFIX_UPPER##HandlePtr =                                       \
    std::unique_ptr<sPREFIX##_handle_t,                                  \
                    void (*)(sPREFIX##_handle_t * handle)>;              \
  inline sPREFIX_UPPER##HandlePtr Create##sPREFIX_UPPER##HandlePtr()     \
  {                                                                      \
    auto handle = sPREFIX_UPPER##HandlePtr(rcomm_create(), &rcomm_free); \
    sPREFIX##_init(handle.get());                                        \
    return handle;                                                       \
  }                                                                      \
  }                                                                      \
  }
#else
#define LRT_RCOMM_PTR(sPREFIX, sPREFIX_UPPER)
#endif

#if defined(__cplusplus) && !defined(DISABLE_ADVANCED_CPP)
#define LRT_RCOMM_PTR_DEF(sPREFIX, sPREFIX_UPPER)           \
  namespace lrt {                                           \
  namespace RCore {                                         \
  using sPREFIX_UPPER##HandlePtr =                          \
    std::unique_ptr<sPREFIX##_handle_t,                     \
                    void (*)(sPREFIX##_handle_t * handle)>; \
  }                                                         \
  }
#else
#define LRT_RCOMM_PTR_DEF(sPREFIX, sPREFIX_UPPER)
#endif

#endif
