#ifndef AUTOMATA_GLOBAL_H
#define AUTOMATA_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(AUTOMATA_LIBRARY)
#  define AUTOMATASHARED_EXPORT Q_DECL_EXPORT
#else
#  define AUTOMATASHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // AUTOMATA_GLOBAL_H
