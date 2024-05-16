#ifndef tp_task_queue_TaskQueue_h
#define tp_task_queue_TaskQueue_h

#include "tp_task_queue/Task.h"

namespace tp_task_queue
{

//##################################################################################################
class TP_TASK_QUEUE_EXPORT TaskQueue
{
  TP_NONCOPYABLE(TaskQueue);
  TP_DQ;
public:
  //################################################################################################
  TaskQueue(const std::string& threadName, size_t nThreads=1);

  //################################################################################################
  ~TaskQueue();

  //################################################################################################
  size_t numberOfTaskThreads() const;

  //################################################################################################
  void setNumberOfTaskThreads(size_t numberOfTaskThreads);

  //################################################################################################
  //! Add a task to the queue to be processed.
  /*!
  This will add a task to the queue to be processed.
  \param task The task to process, this will take ownership.
  */
  void addTask(Task* task);

  //################################################################################################
  //! Try to cancel a task
  void cancelTask(int64_t taskID);

  //################################################################################################
  //! Set the pause state of a task
  void pauseTask(int64_t taskID, bool paused=false);

  //################################################################################################
  //! Toggle the pause state of a task
  void togglePauseTask(int64_t taskID);

  //################################################################################################
  //! View the progress of current tasks.
  /*!
  While inside the closure the task status mutex is locked so keep it simple and do not call any
  code that may update the task status.

  \param closure The closure that is called with the task statuses.
  */
  void viewTaskStatus(const std::function<void(const std::vector<TaskStatus>&)>& closure);

  //################################################################################################
  void addStatusChangedCallback(const std::function<void()>* statusChangedCallback);

  //################################################################################################
  void removeStatusChangedCallback(const std::function<void()>* statusChangedCallback);
};

}

#endif
