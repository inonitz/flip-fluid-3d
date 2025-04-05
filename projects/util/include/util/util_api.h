#ifndef _UTIL_API_H_
#define _UTIL_API_H_


#ifdef UTIL_STATIC_DEFINE /* Define if compiling as a static library (-DUTIL_STATIC_DEFINE) */
#	define UTIL_API
#	define UTIL_NO_EXPORT
#else
#	ifndef UTIL_API
#		ifdef UTIL_EXPORTS /* We are building this library */
#			if defined _WIN32 || defined _WIN64 || defined __CYGWIN__ || defined __MINGW64__
#				if defined __GNUC__ || defined __clang__
#					define UTIL_API __attribute__ ((dllexport))
#				else
#					define UTIL_API __declspec(dllexport)
#				endif
#			else 
#				if (defined __GNUC__ && __GNUC__ >= 4) || defined __clang__
#					define UTIL_API __attribute__ ((visibility ("default")))
#				endif
#			endif
#    	else /* We are using this library */
#			if defined _WIN32 || defined _WIN64 || defined __CYGWIN__ || defined __MINGW64__
#				if defined __GNUC__ || defined __clang__
#					define UTIL_API __attribute__ ((dllimport))
#				else
#					define UTIL_API __declspec(dllimport)
#				endif
#			else 
#				if defined __GNUC__ && __GNUC__ >= 4
#					define UTIL_API
#				endif
#			endif
#       endif
#	else /* Should Only reach here for non-*nix, un-supported platforms */
#       warning "Platform Unsupported - Either Not a derivative of Unix // Not Windows"
#		define UTIL_API
#   endif
#	ifndef UTIL_NO_EXPORT
#       if defined __GNUC__ && __GNUC__ >= 4 /* Symbols exported by default on *nix systems */
#           define UTIL_NO_EXPORT __attribute__((visibility ("hidden")))
#       else /* (DLL) Symbols on platforms like windows must be exported manually [__declspec(dllexport)] */
#		    define UTIL_NO_EXPORT 
#       endif
#	endif
#endif


#ifndef UTIL_DEPRECATED
#   if defined(__cplusplus)
#       if __cplusplus > 201402L /* [[deprecated]] Supported since C++14 */
#           define UTIL_DEPRECATED [[deprecated]]
#           define UTIL_DEPRECATED_MSG(MSG) [[deprecated(MSG)]]
#       endif
#   else
#       if defined _WIN32 || defined _WIN64
#           if defined __GNUC__ || defined __clang__ /* Cygwin, MinGW32/64 */
#               define UTIL_DEPRECATED          __attribute__((deprecated))
#               define UTIL_DEPRECATED_MSG(MSG) __attribute__((deprecated(MSG)))
#           else
#               define UTIL_DEPRECATED          __declspec(deprecated)
#               define UTIL_DEPRECATED_MSG(MSG) __declspec(deprecated(MSG))
#           endif
#       elif defined __GNUC__ || defined __clang__
#           define UTIL_DEPRECATED __attribute__((deprecated))
#           define UTIL_DEPRECATED_MSG(MSG) __declspec(deprecated(MSG))
#       else /* Should Only reach here for non-*nix, un-supported platforms */
#           define UTIL_DEPRECATED
#           define UTIL_DEPRECATED_MSG(MSG)
#       endif
#   endif
#endif


#ifndef UTIL_DEPRECATED_EXPORT
#  define UTIL_DEPRECATED_EXPORT UTIL_API UTIL_DEPRECATED
#endif


#ifndef UTIL_DEPRECATED_NO_EXPORT
#  define UTIL_DEPRECATED_NO_EXPORT UTIL_NO_EXPORT UTIL_DEPRECATED
#endif


/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef UTIL_NO_DEPRECATED
#    define UTIL_NO_DEPRECATED
#  endif
#endif

#endif /* _UTIL_API_H_ */