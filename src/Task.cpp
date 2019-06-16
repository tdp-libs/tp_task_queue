#include "tp_task_queue/Task.h"
#include "tp_task_queue/SynchronizationPoint.h"

#include "tp_utils/MutexUtils.h"

#include <atomic>

namespace tp_task_queue
{

//##################################################################################################
struct Task::Private
{
  int64_t taskID{generateTaskID()};
  std::string taskName;
  TaskCallback performTask;
  int64_t timeout;
  std::string timeoutMessage;

  std::function<void(const TaskStatus&)> statusChangedCallback;

  SynchronizationPoint* synchronizationPoint{nullptr};

  TaskQueue* taskQueue{nullptr};

  TPMutex taskStatusMutex{TPM};
  TaskStatus taskStatus;

  bool pauseable;
  std::atomic_bool finish{false};
  std::atomic_bool paused{false};

  //################################################################################################
  Private(std::string taskName_,
          TaskCallback performTask_,
          int64_t timeout_,
          std::string timeoutMessage_,
          bool pauseable_):
    taskName(std::move(taskName_)),
    performTask(std::move(performTask_)),
    timeout(timeout_),
    timeoutMessage(std::move(timeoutMessage_)),
    pauseable(pauseable_)
  {

  }

  //################################################################################################
  int64_t generateTaskID()
  {
    static TPMutex mutex{TPM};
    static int64_t count{0};
    TP_MUTEX_LOCKER(mutex);
    count++;
    return count;
  }
};

//##################################################################################################
Task::Task(const std::string& taskName, const TaskCallback& performTask, int64_t timeout, const std::string& timeoutMessage, bool pauseable):
  d(new Private(taskName, performTask, timeout, timeoutMessage, pauseable))
{

}

//##################################################################################################
Task::~Task()
{
  if(d->synchronizationPoint)
    d->synchronizationPoint->removeTask(this);
  delete d;
}

//##################################################################################################
int64_t Task::taskID() const
{
  return d->taskID;
}

//##################################################################################################
const std::string& Task::taskName() const
{
  return d->taskName;
}

//##################################################################################################
int64_t Task::timeoutMS() const
{
  return d->timeout;
}

//##################################################################################################
void Task::setTimeoutMS(int64_t timeout)
{
  d->timeout = timeout;;
}

//##################################################################################################
const std::string& Task::timeoutMessage() const
{
  return d->timeoutMessage;
}

//##################################################################################################
bool Task::pauseable() const
{
  return d->pauseable;
}

//################################################################################################
bool Task::paused() const
{
  return d->paused;
}

//################################################################################################
void Task::setPaused(bool paused)
{
  d->paused = paused;

  if(d->statusChangedCallback)
    d->statusChangedCallback(taskStatus());
}

//##################################################################################################
bool Task::shouldFinish() const
{
  return d->finish;
}

//##################################################################################################
void Task::cancelTask()
{
  d->finish=true;
}

//##################################################################################################
RunAgain Task::performTask()
{
  return d->performTask(*this);
}

//##################################################################################################
TaskStatus Task::taskStatus()const
{
  TP_MUTEX_LOCKER(d->taskStatusMutex);
  d->taskStatus.taskID = d->taskID;
  d->taskStatus.taskName = d->taskName;
  d->taskStatus.pauseable = d->pauseable;
  d->taskStatus.paused = d->paused;
  return d->taskStatus;
}

//##################################################################################################
void Task::updateTaskStatus(const TaskStatus& taskStatus)
{
  d->taskStatusMutex.lock(TPM);
  d->taskStatus = taskStatus;
  d->taskStatus.pauseable = d->pauseable;
  d->taskStatus.paused = d->paused;
  auto ts = d->taskStatus;
  d->taskStatusMutex.unlock(TPM);

  if(d->statusChangedCallback)
    d->statusChangedCallback(ts);
}

//##################################################################################################
void Task::updateTaskStatus(const std::string& message, int progress)
{
  d->taskStatusMutex.lock(TPM);
  d->taskStatus.message = message;
  d->taskStatus.progress = progress;
  d->taskStatus.pauseable = d->pauseable;
  d->taskStatus.paused = d->paused;
  auto ts = d->taskStatus;
  d->taskStatusMutex.unlock(TPM);

  if(d->statusChangedCallback)
    d->statusChangedCallback(ts);
}

//##################################################################################################
void Task::setStatusChangedCallback(std::function<void(const TaskStatus&)> statusChangedCallback)
{
  d->statusChangedCallback = std::move(statusChangedCallback);
}

//##################################################################################################
void Task::setTaskQueue(TaskQueue* taskQueue)
{
  d->taskQueue = taskQueue;
}

//##################################################################################################
TaskQueue* Task::taskQueue()const
{
  return d->taskQueue;
}

//##################################################################################################
void Task::setSynchronizationPoint(SynchronizationPoint* synchronizationPoint)
{
  d->synchronizationPoint = synchronizationPoint;
}

}
