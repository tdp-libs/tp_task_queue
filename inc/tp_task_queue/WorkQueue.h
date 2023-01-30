#ifndef tp_task_queue_WorkQueue_h
#define tp_task_queue_WorkQueue_h

#include "tp_task_queue/Globals.h"

#include <functional>

namespace tp_task_queue
{

//##################################################################################################
//! Similar to the TaskQueue but far more simple and guarantees to complete all task on destruction.
class TP_TASK_QUEUE_EXPORT WorkQueue
{
  TP_NONCOPYABLE(WorkQueue);
public:
  //################################################################################################
  WorkQueue(const std::string& threadName);

  //################################################################################################
  ~WorkQueue();

  //################################################################################################
  //! Add a task to the queue to be processed.
  void addTask(const std::function<void()>& task);

private:
  struct Private;
  friend struct Private;
  Private* d;
};

}

#endif
