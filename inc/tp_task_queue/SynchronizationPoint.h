#ifndef tp_task_queue_SynchronizationPoint_h
#define tp_task_queue_SynchronizationPoint_h

#include "tp_task_queue/Globals.h"// IWYU pragma: keep

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
  TP_DQ;
public:
  //################################################################################################
  /*!
  The taskRemoved callback must be thread safe and can't call addTask directly as it is being called
  from a task thread.
  */
  SynchronizationPoint(const std::function<void()>& taskRemoved={});

  //################################################################################################
  ~SynchronizationPoint();

  //################################################################################################
  void join();

  //################################################################################################
  void addTask(Task* task, size_t maxActive=std::numeric_limits<size_t>::max());

  //################################################################################################
  void cancelTasks();

  //################################################################################################
  bool cancelTasksCalled() const;

  //################################################################################################
  size_t activeTasks();

private:
  friend class Task;
  void removeTask(Task* task);
};

}

#endif
