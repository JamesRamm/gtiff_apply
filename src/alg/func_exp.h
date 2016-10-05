
#ifndef GALGFUNC_DLL_H
#define GALGFUNC_DLL_H

#ifdef GeoAlg_BUILT_AS_STATIC
#  define GALGFUNC_DLL
#  define GEOALG_NO_EXPORT
#else
#  ifndef GALGFUNC_DLL
#    ifdef galgfunc_EXPORTS
        /* We are building this library */
#      define GALGFUNC_DLL __declspec(dllexport)
#    else
        /* We are using this library */
#      define GALGFUNC_DLL __declspec(dllimport)
#    endif
#  endif

#  ifndef GEOALG_NO_EXPORT
#    define GEOALG_NO_EXPORT 
#  endif
#endif

#ifndef GEOALG_DEPRECATED
#  define GEOALG_DEPRECATED __declspec(deprecated)
#endif

#ifndef GEOALG_DEPRECATED_EXPORT
#  define GEOALG_DEPRECATED_EXPORT GALGFUNC_DLL GEOALG_DEPRECATED
#endif

#ifndef GEOALG_DEPRECATED_NO_EXPORT
#  define GEOALG_DEPRECATED_NO_EXPORT GEOALG_NO_EXPORT GEOALG_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef GEOALG_NO_DEPRECATED
#    define GEOALG_NO_DEPRECATED
#  endif
#endif

#endif
