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
  TP_NONCOPYABLE(SynchronizationPoint);
  TP_REF_COUNT_OBJECTS("SynchronizationPoint");
public:
  //################################################################################################
  SynchronizationPoint();

  //################################################################################################
  ~SynchronizationPoint();

  //################################################################################################
  void join();

  //################################################################################################
  void addTask(Task* task, size_t maxActive=std::numeric_limits<size_t>::max());

  //################################################################################################
  void cancelTasks();

  //################################################################################################
  size_t activeTasks();

private:
  friend class Task;
  void removeTask(Task* task);

  struct Private;
  friend struct Private;
  Private* d;
};

}

#endif
