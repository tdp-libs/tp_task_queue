#include "tp_task_queue/TaskQueue.h"

#include "tp_utils/MutexUtils.h"
#include "tp_utils/TimeUtils.h"
#include "tp_utils/DebugUtils.h"

#include <atomic>
#include <thread>

namespace tp_task_queue
{

namespace
{
//##################################################################################################
struct TaskDetails_lt
{
  TP_NONCOPYABLE(TaskDetails_lt);

  Task* task{nullptr};
  int64_t nextRun{0};
  bool active{false};

  TaskDetails_lt()=default;

  ~TaskDetails_lt()
  {
    delete task;
  }
};
}

//##################################################################################################
struct TaskQueue::Private
{
  TPMutex mutex;
  TPWaitCondition waitCondition;
  TPWaitCondition updateWaitingMessagesWaitCondition;
  std::vector<TaskDetails_lt*> tasks;

  TPMutex taskStatusMutex;
  std::vector<TaskStatus> taskStatuses;

  TPMutex statusChangedCallbacksMutex;
  std::vector<const std::function<void()>*> statusChangedCallbacks;

  std::vector<std::thread*> threads;
  std::atomic_bool finish{false};

  //################################################################################################
  void taskStatusChanged()
  {
    TP_MUTEX_LOCKER(statusChangedCallbacksMutex);
    for(auto c : statusChangedCallbacks)
      (*c)();
  }

  //################################################################################################
  void updateWaitingMessages()
  {
    bool changed = false;

    mutex.lock(TPM);
    for(TaskDetails_lt* taskDetails : tasks)
    {
      if(!taskDetails->active && (taskDetails->nextRun>0 || taskDetails->task->paused()))
      {
        int64_t timeToRun = taskDetails->nextRun - tp_utils::currentTimeMS();
        if(timeToRun<0)
          taskDetails->nextRun=0;
        timeToRun = tpMax(int64_t(0), timeToRun/1000);
        taskStatusMutex.lock(TPM);
        for(TaskStatus& ts : taskStatuses)
        {
          if(ts.taskID == taskDetails->task->taskID())
          {
            if(ts.paused)
              ts.message = "Paused.";
            else if(timeToRun==0)
              ts.message = "Waiting for thread.";
            else
              ts.message = taskDetails->task->timeoutMessage() + std::to_string(timeToRun);
            changed = true;
            break;
          }
        }
        taskStatusMutex.unlock(TPM);
      }
    }
    mutex.unlock(TPM);

    if(changed)
      taskStatusChanged();
  }
};

//##################################################################################################
TaskQueue::TaskQueue(int nThreads):
  d(new Private())
{
  for(int i=0; i<nThreads; i++)
  {
    d->threads.push_back(new std::thread([&]()
    {
      d->mutex.lock(TPM);
      while(!d->finish)
      {
        bool workDone = false;
        int64_t waitFor = INT64_MAX;
        for(TaskDetails_lt* taskDetails : d->tasks)
        {
          if(taskDetails->active)
            continue;

          if(taskDetails->task->paused())
            continue;

          int64_t timeToRun = taskDetails->nextRun - tp_utils::currentTimeMS();

          if(timeToRun<waitFor)
            waitFor = timeToRun;

          if(timeToRun>0)
            continue;

          taskDetails->active = true;

          d->mutex.unlock(TPM);
          workDone = true;
          bool keep = taskDetails->task->performTask();
          d->mutex.lock(TPM);

          if(taskDetails->task->timeoutMS()<1 || !keep)
          {
            tpRemoveOne(d->tasks, taskDetails);
            d->mutex.unlock(TPM);
            TaskStatus taskStatus = taskDetails->task->taskStatus();
            taskStatus.complete = true;
            taskDetails->task->updateTaskStatus(taskStatus);
            delete taskDetails;
            d->mutex.lock(TPM);
          }
          else
          {
            if(taskDetails->task->timeoutMS()>0)
              taskDetails->nextRun = tp_utils::currentTimeMS() + taskDetails->task->timeoutMS();
            taskDetails->active = false;
          }
          break;
        }

        if(!workDone)
          d->waitCondition.wait(d->mutex, waitFor);
      }
      d->mutex.unlock(TPM);
    }));
  }

  d->threads.push_back(new std::thread([&]()
  {
    d->mutex.lock(TPM);
    while(!d->finish)
    {
      d->updateWaitingMessagesWaitCondition.wait(d->mutex, 1000);
      d->mutex.unlock(TPM);
      d->updateWaitingMessages();
      d->mutex.lock(TPM);
    }
    d->mutex.unlock(TPM);
  }));
}

//##################################################################################################
TaskQueue::~TaskQueue()
{  
  d->finish = true;

  d->mutex.lock(TPM);
  for(TaskDetails_lt* taskDetails : d->tasks)
    taskDetails->task->cancelTask();
  d->waitCondition.wakeAll();
  d->mutex.unlock(TPM);

  for(std::thread* thread : d->threads)
  {
    thread->join();
    delete thread;
  }

  d->mutex.lock(TPM);
  for(TaskDetails_lt* taskDetails : d->tasks)
    delete taskDetails;
  d->mutex.unlock(TPM);

  delete d;
}

//##################################################################################################
void TaskQueue::addTask(Task* task)
{
  task->setTaskQueue(this);

  d->mutex.lock(TPM);
  auto taskDetails = new TaskDetails_lt();
  taskDetails->task = task;
  taskDetails->nextRun = tp_utils::currentTimeMS();
  d->tasks.push_back(taskDetails);
  d->waitCondition.wakeOne();

  d->taskStatusMutex.lock(TPM);
  d->taskStatuses.push_back(taskDetails->task->taskStatus());
  d->taskStatusMutex.unlock(TPM);

  taskDetails->task->setStatusChangedCallback([&](const TaskStatus& taskStatus)
  {
    d->taskStatusMutex.lock(TPM);
    for(TaskStatus& ts : d->taskStatuses)
    {
      if(ts.taskID == taskStatus.taskID)
      {
        int64_t rev = ts.rev;
        ts = taskStatus;
        ts.rev = rev;
        break;
      }
    }
    d->taskStatusMutex.unlock(TPM);
    d->taskStatusChanged();
  });
  d->mutex.unlock(TPM);
  d->taskStatusChanged();
}

//##################################################################################################
void TaskQueue::cancelTask(int64_t taskID)
{
  TP_MUTEX_LOCKER(d->mutex);
  for(TaskDetails_lt* taskDetails : d->tasks)
  {
    if(taskDetails->task->taskID() == taskID)
    {
      taskDetails->task->cancelTask();
      d->waitCondition.wakeAll();
      break;
    }
  }
}

//##################################################################################################
void TaskQueue::pauseTask(int64_t taskID, bool paused)
{
  TP_MUTEX_LOCKER(d->mutex);
  for(TaskDetails_lt* taskDetails : d->tasks)
  {
    if(taskDetails->task->taskID() == taskID)
    {
      taskDetails->task->setPaused(paused);
      d->waitCondition.wakeAll();
      break;
    }
  }
}

//##################################################################################################
void TaskQueue::togglePauseTask(int64_t taskID)
{
  TP_MUTEX_LOCKER(d->mutex);
  for(TaskDetails_lt* taskDetails : d->tasks)
  {
    if(taskDetails->task->taskID() == taskID)
    {
      taskDetails->task->setPaused(!taskDetails->task->paused());
      d->waitCondition.wakeAll();
      break;
    }
  }
}

//##################################################################################################
void TaskQueue::viewTaskStatus(const std::function<void(const std::vector<TaskStatus>&)>& closure)
{
  TP_MUTEX_LOCKER(d->taskStatusMutex);
  closure(d->taskStatuses);
}

//##################################################################################################
void TaskQueue::addStatusChangedCallback(const std::function<void()>* statusChangedCallback)
{
  TP_MUTEX_LOCKER(d->statusChangedCallbacksMutex);
  d->statusChangedCallbacks.push_back(statusChangedCallback);
}

//##################################################################################################
void TaskQueue::removeStatusChangedCallback(const std::function<void()>* statusChangedCallback)
{
  TP_MUTEX_LOCKER(d->statusChangedCallbacksMutex);
  tpRemoveOne(d->statusChangedCallbacks, statusChangedCallback);
}

}
