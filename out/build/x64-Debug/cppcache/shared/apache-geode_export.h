
#ifndef APACHE_GEODE_EXPORT_H
#define APACHE_GEODE_EXPORT_H

#ifdef APACHE_GEODE_STATIC_DEFINE
#  define APACHE_GEODE_EXPORT
#  define APACHE_GEODE_NO_EXPORT
#else
#  ifndef APACHE_GEODE_EXPORT
#    ifdef apache_geode_EXPORTS
        /* We are building this library */
#      define APACHE_GEODE_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define APACHE_GEODE_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef APACHE_GEODE_NO_EXPORT
#    define APACHE_GEODE_NO_EXPORT 
#  endif
#endif

#ifndef APACHE_GEODE_DEPRECATED
#  define APACHE_GEODE_DEPRECATED __declspec(deprecated)
#endif

#ifndef APACHE_GEODE_DEPRECATED_EXPORT
#  define APACHE_GEODE_DEPRECATED_EXPORT APACHE_GEODE_EXPORT APACHE_GEODE_DEPRECATED
#endif

#ifndef APACHE_GEODE_DEPRECATED_NO_EXPORT
#  define APACHE_GEODE_DEPRECATED_NO_EXPORT APACHE_GEODE_NO_EXPORT APACHE_GEODE_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef APACHE_GEODE_NO_DEPRECATED
#    define APACHE_GEODE_NO_DEPRECATED
#  endif
#endif

  #define APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT APACHE_GEODE_EXPORT

  #define APACHE_GEODE_EXTERN_TEMPLATE_EXPORT
  
#endif /* APACHE_GEODE_EXPORT_H */
