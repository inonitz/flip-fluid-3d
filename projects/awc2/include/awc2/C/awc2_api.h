#ifndef _AWC2_API_H_
#define _AWC2_API_H_


#ifdef AWC2_STATIC_DEFINE /* Define if compiling as a static library (-DAWC2_STATIC_DEFINE) */
#	define AWC2_API
#	define AWC2_NO_EXPORT
#else
#	ifndef AWC2_API
#		ifdef AWC2_EXPORTS /* We are building this library */
#			if defined _WIN32 || defined _WIN64 || defined __CYGWIN__ || defined __MINGW64__
#				ifdef __GNUC__
#					define AWC2_API __attribute__ ((dllexport))
#				else
#					define AWC2_API __declspec(dllexport)
#				endif
#			else 
#				if defined __GNUC__ && __GNUC__ >= 4
#					define AWC2_API __attribute__ ((visibility ("default")))
#				endif
#			endif
#    	else /* We are using this library */
#			if defined _WIN32 || defined _WIN64 || defined __CYGWIN__ || defined __MINGW64__
#				ifdef __GNUC__
#					define AWC2_API __attribute__ ((dllimport))
#				else
#					define AWC2_API __declspec(dllimport)
#				endif
#			else 
#				if defined __GNUC__ && __GNUC__ >= 4
#					define AWC2_API
#				endif
#			endif
#       endif
#	else /* Should Only reach here for non-*nix, un-supported platforms */
#		define AWC2_API
#   endif
#	ifndef AWC2_NO_EXPORT
#       if defined __GNUC__ && __GNUC__ >= 4 /* Symbols exported by default on *nix systems */
#           define AWC2_NO_EXPORT __attribute__((visibility ("hidden")))
#       else /* (DLL) Symbols on platforms like windows must be exported manually [__declspec(dllexport)] */
#		    define AWC2_NO_EXPORT 
#       endif
#	endif
#endif


#ifndef AWC2_DEPRECATED
#   if defined(__cplusplus)
#       if __cplusplus > 201402L /* [[deprecated]] Supported since C++14 */
#           define AWC2_DEPRECATED [[deprecated]]
#           define AWC2_DEPRECATED_MSG(MSG) [[deprecated(MSG)]]
#       endif
#   else
#       if defined _WIN32 || defined _WIN64
#           if defined __GNUC__ || defined __clang__ /* Cygwin, MinGW32/64 */
#               define AWC2_DEPRECATED          __attribute__((deprecated))
#               define AWC2_DEPRECATED_MSG(MSG) __attribute__((deprecated(MSG)))
#           else
#               define AWC2_DEPRECATED          __declspec(deprecated)
#               define AWC2_DEPRECATED_MSG(MSG) __declspec(deprecated(MSG))
#           endif
#       elif defined __GNUC__ || defined __clang__
#           define AWC2_DEPRECATED __attribute__((deprecated))
#           define AWC2_DEPRECATED_MSG(MSG) __declspec(deprecated(MSG))
#       else /* Should Only reach here for non-*nix, un-supported platforms */
#           define AWC2_DEPRECATED
#           define AWC2_DEPRECATED_MSG(MSG)
#       endif
#   endif
#endif


#ifndef AWC2_DEPRECATED_EXPORT
#  define AWC2_DEPRECATED_EXPORT AWC2_API AWC2_DEPRECATED
#endif


#ifndef AWC2_DEPRECATED_NO_EXPORT
#  define AWC2_DEPRECATED_NO_EXPORT AWC2_NO_EXPORT AWC2_DEPRECATED
#endif


/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef AWC2_NO_DEPRECATED
#    define AWC2_NO_DEPRECATED
#  endif
#endif

#endif /* _AWC2_API_H_ */