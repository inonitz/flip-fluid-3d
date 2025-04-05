#ifndef _IMGUI_API_H_
#define _IMGUI_API_H_


/*
    Custom Export/Import API For ImGui [not original repo!] - since ocornut doesn't recommend
    using ImGui as a shared library.
*/
#ifdef IMGUI_STATIC_DEFINE /* Define if compiling as a static library (-DIMGUI_STATIC_DEFINE) */
#	define IMGUI_API
#	define IMGUI_NO_EXPORT
#else
#	ifndef IMGUI_API
#		ifdef IMGUI_EXPORTS /* We are building this library */
#			if defined _WIN32 || defined _WIN64 || defined __CYGWIN__ || defined __MINGW64__
#				ifdef __GNUC__
#					define IMGUI_API __attribute__ ((dllexport))
#				else
#					define IMGUI_API __declspec(dllexport)
#				endif
#			else 
#				if defined __GNUC__ && __GNUC__ >= 4
#					define IMGUI_API __attribute__ ((visibility ("default")))
#				endif
#			endif
#    	else /* We are using this library */
#			if defined _WIN32 || defined _WIN64 || defined __CYGWIN__ || defined __MINGW64__
#				ifdef __GNUC__
#					define IMGUI_API __attribute__ ((dllimport))
#				else
#					define IMGUI_API __declspec(dllimport)
#				endif
#			else 
#				if defined __GNUC__ && __GNUC__ >= 4
#					define IMGUI_API
#				endif
#			endif
#       endif
#	else /* Should Only reach here for non-*nix, un-supported platforms */
#		define IMGUI_API
#   endif
#	ifndef IMGUI_NO_EXPORT
#       if defined __GNUC__ && __GNUC__ >= 4 /* Symbols exported by default on *nix systems */
#           define IMGUI_NO_EXPORT __attribute__((visibility ("hidden")))
#       else /* (DLL) Symbols on platforms like windows must be exported manually [__declspec(dllexport)] */
#		    define IMGUI_NO_EXPORT 
#       endif
#	endif
#endif


#ifndef IMGUI_DEPRECATED
#   if defined(__cplusplus)
#       if __cplusplus > 201402L /* [[deprecated]] Supported since C++14 */
#           define IMGUI_DEPRECATED [[deprecated]]
#           define IMGUI_DEPRECATED_MSG(MSG) [[deprecated(MSG)]]
#       endif
#   else
#       if defined _WIN32 || defined _WIN64
#           if defined __GNUC__ || defined __clang__ /* Cygwin, MinGW32/64 */
#               define IMGUI_DEPRECATED          __attribute__((deprecated))
#               define IMGUI_DEPRECATED_MSG(MSG) __attribute__((deprecated(MSG)))
#           else
#               define IMGUI_DEPRECATED          __declspec(deprecated)
#               define IMGUI_DEPRECATED_MSG(MSG) __declspec(deprecated(MSG))
#           endif
#       elif defined __GNUC__ || defined __clang__
#           define IMGUI_DEPRECATED __attribute__((deprecated))
#           define IMGUI_DEPRECATED_MSG(MSG) __declspec(deprecated(MSG))
#       else /* Should Only reach here for non-*nix, un-supported platforms */
#           define IMGUI_DEPRECATED
#           define IMGUI_DEPRECATED_MSG(MSG)
#       endif
#   endif
#endif


#ifndef IMGUI_DEPRECATED_EXPORT
#  define IMGUI_DEPRECATED_EXPORT IMGUI_API IMGUI_DEPRECATED
#endif


#ifndef IMGUI_DEPRECATED_NO_EXPORT
#  define IMGUI_DEPRECATED_NO_EXPORT IMGUI_NO_EXPORT IMGUI_DEPRECATED
#endif


/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef IMGUI_NO_DEPRECATED
#    define IMGUI_NO_DEPRECATED
#  endif
#endif

#endif /* _IMGUI_API_H_ */