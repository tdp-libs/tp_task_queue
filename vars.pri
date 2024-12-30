TARGET = tp_task_queue
TEMPLATE = lib

DEFINES += TP_TASK_QUEUE_LIBRARY

#SOURCES += src/Globals.cpp
HEADERS += inc/tp_task_queue/Globals.h

SOURCES += src/TaskQueue.cpp
HEADERS += inc/tp_task_queue/TaskQueue.h

SOURCES += src/Task.cpp
HEADERS += inc/tp_task_queue/Task.h

SOURCES += src/SynchronizationPoint.cpp
HEADERS += inc/tp_task_queue/SynchronizationPoint.h

SOURCES += src/WorkQueue.cpp
HEADERS += inc/tp_task_queue/WorkQueue.h

SOURCES += src/AsyncWorkQueue.cpp
HEADERS += inc/tp_task_queue/AsyncWorkQueue.h
