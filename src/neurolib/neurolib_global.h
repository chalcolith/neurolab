#ifndef NEUROLIB_GLOBAL_H
#define NEUROLIB_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(NEUROLIB_LIBRARY)
#  define NEUROLIBSHARED_EXPORT Q_DECL_EXPORT
#else
#  define NEUROLIBSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // NEUROLIB_GLOBAL_H
