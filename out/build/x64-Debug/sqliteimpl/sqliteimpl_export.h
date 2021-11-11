
#ifndef SQLITEIMPL_EXPORT_H
#define SQLITEIMPL_EXPORT_H

#ifdef SQLITEIMPL_STATIC_DEFINE
#  define SQLITEIMPL_EXPORT
#  define SQLITEIMPL_NO_EXPORT
#else
#  ifndef SQLITEIMPL_EXPORT
#    ifdef SqLiteImpl_EXPORTS
        /* We are building this library */
#      define SQLITEIMPL_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define SQLITEIMPL_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef SQLITEIMPL_NO_EXPORT
#    define SQLITEIMPL_NO_EXPORT 
#  endif
#endif

#ifndef SQLITEIMPL_DEPRECATED
#  define SQLITEIMPL_DEPRECATED __declspec(deprecated)
#endif

#ifndef SQLITEIMPL_DEPRECATED_EXPORT
#  define SQLITEIMPL_DEPRECATED_EXPORT SQLITEIMPL_EXPORT SQLITEIMPL_DEPRECATED
#endif

#ifndef SQLITEIMPL_DEPRECATED_NO_EXPORT
#  define SQLITEIMPL_DEPRECATED_NO_EXPORT SQLITEIMPL_NO_EXPORT SQLITEIMPL_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SQLITEIMPL_NO_DEPRECATED
#    define SQLITEIMPL_NO_DEPRECATED
#  endif
#endif

#endif /* SQLITEIMPL_EXPORT_H */
