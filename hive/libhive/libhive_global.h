#ifndef LIBHIVE_GLOBAL_H
#define LIBHIVE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIBHIVE_LIBRARY)
#  define LIBHIVESHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBHIVESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBHIVE_GLOBAL_H
