#ifndef tp_task_queue_SynchronizationPoint_h
#define tp_task_queue_SynchronizationPoint_h

#include "tp_task_queue/Globals.h"

#include "tp_utils/RefCount.h"

#include <limits>

namespace tp_task_queue
{
class Task;

//##################################################################################################
//! Waits for tasks to finish before returning from its destructor.
class SynchronizationPoint
{
  TDP_REF_COUNT_OBJECTS("SynchronizationPoint");
public:
  //################################################################################################
  SynchronizationPoint();

  //################################################################################################
  ~SynchronizationPoint();

  //################################################################################################
  void addTask(Task* task, size_t maxActive=std::numeric_limits<size_t>::max());

private:
  friend class Task;
  void removeTask();

  struct Private;
  friend struct Private;
  Private* d;
};

}

#endif
