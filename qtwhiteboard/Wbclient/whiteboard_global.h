#ifndef WHITEBOARD_GLOBAL_H
#define WHITEBOARD_GLOBAL_H

#include <QtCore/qglobal.h>

#include <painterscene.h>
#include <painterview.h>
#include <shape.h>
#include <wbconnect.h>


#if defined(WHITEBOARD_LIBRARY)
#  define WHITEBOARDSHARED_EXPORT Q_DECL_EXPORT
#else
#  define WHITEBOARDSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // WHITEBOARD_GLOBAL_H

