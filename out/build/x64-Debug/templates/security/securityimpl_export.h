
#ifndef SECURITYIMPL_EXPORT_H
#define SECURITYIMPL_EXPORT_H

#ifdef SECURITYIMPL_STATIC_DEFINE
#  define SECURITYIMPL_EXPORT
#  define SECURITYIMPL_NO_EXPORT
#else
#  ifndef SECURITYIMPL_EXPORT
#    ifdef securityImpl_EXPORTS
        /* We are building this library */
#      define SECURITYIMPL_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define SECURITYIMPL_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef SECURITYIMPL_NO_EXPORT
#    define SECURITYIMPL_NO_EXPORT 
#  endif
#endif

#ifndef SECURITYIMPL_DEPRECATED
#  define SECURITYIMPL_DEPRECATED __declspec(deprecated)
#endif

#ifndef SECURITYIMPL_DEPRECATED_EXPORT
#  define SECURITYIMPL_DEPRECATED_EXPORT SECURITYIMPL_EXPORT SECURITYIMPL_DEPRECATED
#endif

#ifndef SECURITYIMPL_DEPRECATED_NO_EXPORT
#  define SECURITYIMPL_DEPRECATED_NO_EXPORT SECURITYIMPL_NO_EXPORT SECURITYIMPL_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SECURITYIMPL_NO_DEPRECATED
#    define SECURITYIMPL_NO_DEPRECATED
#  endif
#endif

#endif /* SECURITYIMPL_EXPORT_H */
