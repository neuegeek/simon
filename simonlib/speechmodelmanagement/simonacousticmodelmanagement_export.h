#ifndef ACOUSTICMODELMANAGEMENT_EXPORT_H
#define ACOUSTICMODELMANAGEMENT_EXPORT_H
 
// needed for KDE_EXPORT and KDE_IMPORT macros
#include <kdemacros.h>
 
#ifndef ACOUSTICMODELMANAGEMENT_EXPORT
# if defined(MAKE_ACOUSTICMODELMANAGEMENT_LIB)
   // We are building this library
#  define ACOUSTICMODELMANAGEMENT_EXPORT KDE_EXPORT
# else
   // We are using this library
#  define ACOUSTICMODELMANAGEMENT_EXPORT KDE_IMPORT
# endif
#endif
 
# ifndef ACOUSTICMODELMANAGEMENT_EXPORT_DEPRECATED
#  define ACOUSTICMODELMANAGEMENT_EXPORT_DEPRECATED KDE_DEPRECATED ACOUSTICMODELMANAGEMENT_EXPORT
# endif
 
#endif