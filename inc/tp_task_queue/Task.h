#ifndef tp_task_queue_Task_h
#define tp_task_queue_Task_h

#include "tp_task_queue/Globals.h" // IWYU pragma: keep

#include "tp_utils/RefCount.h"

#include <string>

namespace tp_task_queue
{
class Task;
class TaskQueue;
class SynchronizationPoint;

//##################################################################################################
using TaskCallback = std::function<RunAgain(Task&)>;

//##################################################################################################
//! The status of a running task.
struct TaskStatus
{
  int64_t taskID{0};    //!< The unique ID assigned to the task when it is created.
  int64_t rev{0};       //!< Incremented each time the status changes.
  std::string taskName; //!< The name give to the task eg. 'Save models'.
  std::string message;  //!< Some message eg. 'Next save in 10s' or 'Saving model 5'.
  int progress{-1};     //!< The current progress as a percent or -1 where not aplicable.
  bool complete{false}; //!< Set true when the task has been completed.
  bool pauseable{false};//!< True if the task can be paused.
  bool paused{false};   //!< True if the task is paused.
};

//##################################################################################################
//! A task that can be added to a task queue.
class Task
{
  TP_NONCOPYABLE(Task);
  TP_REF_COUNT_OBJECTS("Task");
  TP_DQ;
public:

  //################################################################################################
  //! Create a task
  /*!
  Constructs a task that can be added to a task queue for processing.

  \param taskName A user visible name for the task.
  \param performTask A function to call to perform the task, return true to rerun the task.
  \param timeoutMS If this is positive the task will be rerun at this interval.
  */
  Task(const std::string& taskName, const std::function<RunAgain(Task&)>& performTask, int64_t timeoutMS=0, const std::string& timeoutMessage=std::string(), bool pauseable=false);

  //################################################################################################
  ~Task();

  //################################################################################################
  //! A unique ID assigned to the task when it is created.
  int64_t taskID() const;

  //################################################################################################
  const std::string& taskName() const;

  //################################################################################################
  int64_t timeoutMS() const;

  //################################################################################################
  void setTimeoutMS(int64_t timeoutMS);

  //################################################################################################
  const std::string& timeoutMessage() const;

  //################################################################################################
  bool pauseable() const;

  //################################################################################################
  bool paused() const;

  //################################################################################################
  void setPaused(bool paused);

  //################################################################################################
  //! Returns true if the task should finish now.
  bool shouldFinish() const;

  //################################################################################################
  //! This will try and stop the task.
  void cancelTask();

  //################################################################################################
  //! Executes the task.
  RunAgain performTask();

  //################################################################################################
  //! This is used to get task status for the UI
  TaskStatus taskStatus()const;

  //################################################################################################
  //! Task callbacks should call this to update the UI with progress
  void updateTaskStatus(const TaskStatus& taskStatus);

  //################################################################################################
  //! Task callbacks should call this to update the UI with progress
  void updateTaskStatus(const std::string& message, int progress);

  //################################################################################################
  //! This should only be called before performTask is called.
  void setStatusChangedCallback(std::function<void(const TaskStatus&)> statusChangedCallback);

  //################################################################################################
  //! This should only be called before performTask is called.
  void setTaskQueue(TaskQueue* taskQueue);

  //################################################################################################
  TaskQueue* taskQueue()const;

private:
  friend class SynchronizationPoint;
  void setSynchronizationPoint(SynchronizationPoint* synchronizationPoint);
};

}

#endif
