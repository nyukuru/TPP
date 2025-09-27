#pragma once

#ifdef TPP_STATIC
#  define TPP_EXPORT
#else
#  if defined _WIN32 || defined __CYGWIN__
#    ifdef TPP_BUILD
#      define TPP_EXPORT __declspec(dllexport)
#    else
#      define TPP_EXPORT __declspec(dllimport)
#    endif
#  else
#    ifdef TPP_BUILD
#      define TPP_EXPORT __attribute__((visibility("default")))
#    else
#      define TPP_EXPORT
#    endif
#  endif
#endif

