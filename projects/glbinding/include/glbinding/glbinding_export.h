#ifndef GLBINDING_API_H
#define GLBINDING_API_H

#ifdef GLBINDING_STATIC_DEFINE /* Define if compiling as a static library (-DGLBINDING_STATIC_DEFINE) */
#	define GLBINDING_API
#	define GLBINDING_NO_EXPORT
#else
#	ifndef GLBINDING_API
#		ifdef glbinding_EXPORTS /* We are building this library */
#			if defined _WIN32 || defined _WIN64 || defined __CYGWIN__ || defined __MINGW64__
#				ifdef __GNUC__
#					define GLBINDING_API __attribute__ ((dllexport))
#				else
#					define GLBINDING_API __declspec(dllexport)
#				endif
#			else 
#				ifdef __GNUC__
#					define GLBINDING_API __attribute__ ((visibility ("default")))
#				endif
#			endif
#    	else /* We are using this library */
#			if defined _WIN32 || defined _WIN64 || defined __CYGWIN__ || defined __MINGW64__
#				ifdef __GNUC__
#					define GLBINDING_API __attribute__ ((dllimport))
#				else
#					define GLBINDING_API __declspec(dllimport)
#				endif
#			else 
#				ifdef __GNUC__
#					define GLBINDING_API
#				endif
#			endif
#       endif
#	else /* Should Only reach here for non-*nix, un-supported platforms */
#		define GLBINDING_API
#   endif
#	ifndef GLBINDING_NO_EXPORT
#       ifdef __GNUC__ /* Symbols exported by default on *nix systems */
#           define GLBINDING_NO_EXPORT __attribute__((visibility ("hidden")))
#       else /* (DLL) Symbols on platforms like windows must be exported manually [__declspec(dllexport)] */
#		    define GLBINDING_NO_EXPORT 
#       endif
#	endif
#endif


#ifndef GLBINDING_DEPRECATED
#   if defined(__cplusplus)
#       if __cplusplus > 201402L /* [[deprecated]] Supported since C++14 */
#           define GLBINDING_DEPRECATED [[deprecated]]
#       endif
#   else
#       if defined _WIN32 || defined _WIN64
#           if defined __GNUC__ || defined __clang__ /* Cygwin, MinGW32/64 */
#               define GLBINDING_DEPRECATED __attribute__((deprecated))
#           else
#               define GLBINDING_DEPRECATED __declspec(deprecated)
#           endif
#       elif defined __GNUC__ || defined __clang__
#           define GLBINDING_DEPRECATED __attribute__((deprecated))
#       else /* Should Only reach here for non-*nix, un-supported platforms */
#           define GLBINDING_DEPRECATED 
#       endif
#   endif
#endif


#ifndef GLBINDING_DEPRECATED_EXPORT
#  define GLBINDING_DEPRECATED_EXPORT GLBINDING_API GLBINDING_DEPRECATED
#endif


#ifndef GLBINDING_DEPRECATED_NO_EXPORT
#  define GLBINDING_DEPRECATED_NO_EXPORT GLBINDING_NO_EXPORT GLBINDING_DEPRECATED
#endif


/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef GLBINDING_NO_DEPRECATED
#    define GLBINDING_NO_DEPRECATED
#  endif
#endif

#endif /* GLBINDING_API_H */