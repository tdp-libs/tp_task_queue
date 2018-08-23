TARGET = tp_task_queue
TEMPLATE = lib

DEFINES += TP_TASK_QUEUE_LIBRARY

SOURCES += src/Globals.cpp
HEADERS += inc/tp_task_queue/Globals.h

SOURCES += src/TaskQueue.cpp
HEADERS += inc/tp_task_queue/TaskQueue.h

SOURCES += src/Task.cpp
HEADERS += inc/tp_task_queue/Task.h

