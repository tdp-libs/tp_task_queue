#ifndef tp_task_queue_Globals_h
#define tp_task_queue_Globals_h

#include "tp_utils/Globals.h" // IWYU pragma: keep

#if defined(TP_TASK_QUEUE_LIBRARY)
#  define TP_TASK_QUEUE_EXPORT TP_EXPORT
#else
#  define TP_TASK_QUEUE_EXPORT TP_IMPORT
#endif

//##################################################################################################
//! An engine for processing background tasks
namespace tp_task_queue
{

//##################################################################################################
enum class RunAgain
{
  Yes,
  No
};

}

#endif
