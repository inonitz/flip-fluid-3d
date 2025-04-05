#ifndef _GLBINDING_AUX_API_H_
#define _GLBINDING_AUX_API_H_


#ifdef GLBINDING_AUX_STATIC_DEFINE /* Define if compiling as a static library (-DGLBINDING_AUX_STATIC_DEFINE) */
#	define GLBINDING_AUX_API
#	define GLBINDING_AUX_NO_EXPORT
#else
#	ifndef GLBINDING_AUX_API
#		ifdef glbinding_aux_EXPORTS /* We are building this library */
#			if defined _WIN32 || defined _WIN64 || defined __CYGWIN__ || defined __MINGW64__
#				ifdef __GNUC__
#					define GLBINDING_AUX_API __attribute__ ((dllexport))
#				else
#					define GLBINDING_AUX_API __declspec(dllexport)
#				endif
#			else 
#				if defined __GNUC__ && __GNUC__ >= 4
#					define GLBINDING_AUX_API __attribute__ ((visibility ("default")))
#				endif
#			endif
#    	else /* We are using this library */
#			if defined _WIN32 || defined _WIN64 || defined __CYGWIN__ || defined __MINGW64__
#				ifdef __GNUC__
#					define GLBINDING_AUX_API __attribute__ ((dllimport))
#				else
#					define GLBINDING_AUX_API __declspec(dllimport)
#				endif
#			else 
#				if defined __GNUC__ && __GNUC__ >= 4
#					define GLBINDING_AUX_API
#				endif
#			endif
#       endif
#	else /* Should Only reach here for non-*nix, un-supported platforms */
#		define GLBINDING_AUX_API
#   endif
#	ifndef GLBINDING_AUX_NO_EXPORT
#       if defined __GNUC__ && __GNUC__ >= 4 /* Symbols exported by default on *nix systems */
#           define GLBINDING_AUX_NO_EXPORT __attribute__((visibility ("hidden")))
#       else /* (DLL) Symbols on platforms like windows must be exported manually [__declspec(dllexport)] */
#		    define GLBINDING_AUX_NO_EXPORT 
#       endif
#	endif
#endif


#ifndef GLBINDING_AUX_DEPRECATED
#   if defined(__cplusplus)
#       if __cplusplus > 201402L /* [[deprecated]] Supported since C++14 */
#           define GLBINDING_AUX_DEPRECATED [[deprecated]]
#           define GLBINDING_AUX_DEPRECATED_MSG(MSG) [[deprecated(MSG)]]
#       endif
#   else
#       if defined _WIN32 || defined _WIN64
#           if defined __GNUC__ || defined __clang__ /* Cygwin, MinGW32/64 */
#               define GLBINDING_AUX_DEPRECATED          __attribute__((deprecated))
#               define GLBINDING_AUX_DEPRECATED_MSG(MSG) __attribute__((deprecated(MSG)))
#           else
#               define GLBINDING_AUX_DEPRECATED          __declspec(deprecated)
#               define GLBINDING_AUX_DEPRECATED_MSG(MSG) __declspec(deprecated(MSG))
#           endif
#       elif defined __GNUC__ || defined __clang__
#           define GLBINDING_AUX_DEPRECATED __attribute__((deprecated))
#           define GLBINDING_AUX_DEPRECATED_MSG(MSG) __declspec(deprecated(MSG))
#       else /* Should Only reach here for non-*nix, un-supported platforms */
#           define GLBINDING_AUX_DEPRECATED
#           define GLBINDING_AUX_DEPRECATED_MSG(MSG)
#       endif
#   endif
#endif


#ifndef GLBINDING_AUX_DEPRECATED_EXPORT
#  define GLBINDING_AUX_DEPRECATED_EXPORT GLBINDING_AUX_API GLBINDING_AUX_DEPRECATED
#endif


#ifndef GLBINDING_AUX_DEPRECATED_NO_EXPORT
#  define GLBINDING_AUX_DEPRECATED_NO_EXPORT GLBINDING_AUX_NO_EXPORT GLBINDING_AUX_DEPRECATED
#endif


/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef GLBINDING_AUX_NO_DEPRECATED
#    define GLBINDING_AUX_NO_DEPRECATED
#  endif
#endif

#endif /* _GLBINDING_AUX_API_H_ */