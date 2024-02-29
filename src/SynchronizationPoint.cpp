#include "tp_task_queue/SynchronizationPoint.h"
#include "tp_task_queue/Task.h"

#include "tp_utils/MutexUtils.h"

namespace tp_task_queue
{

//##################################################################################################
struct SynchronizationPoint::Private
{
  TP_REF_COUNT_OBJECTS("tp_task_queue::SynchronizationPoint::Private");
  TP_NONCOPYABLE(Private);

  std::vector<Task*> tasks;
  TPMutex mutex{TPM};
  TPWaitCondition waitCondition;

  std::function<void()> taskRemoved;

  //################################################################################################
  Private(const std::function<void()>& taskRemoved_):
    taskRemoved(taskRemoved_)
  {

  }
};

//##################################################################################################
SynchronizationPoint::SynchronizationPoint(const std::function<void()>& taskRemoved):
  d(new Private(taskRemoved))
{

}

//##################################################################################################
SynchronizationPoint::~SynchronizationPoint()
{
  join();
  delete d;
}

//##################################################################################################
void SynchronizationPoint::join()
{
  TPMutexLocker lock(d->mutex);
  while(!d->tasks.empty())
    d->waitCondition.wait(TPMc lock);
}

//##################################################################################################
void SynchronizationPoint::addTask(Task* task, size_t maxActive)
{
  task->setSynchronizationPoint(this);
  TPMutexLocker lock(d->mutex);
  while(d->tasks.size()>=maxActive)
    d->waitCondition.wait(TPMc lock);
  d->tasks.push_back(task);
}

//##################################################################################################
void SynchronizationPoint::cancelTasks()
{
  TP_MUTEX_LOCKER(d->mutex);
  for(auto task : d->tasks)
    task->cancelTask();
}

//##################################################################################################
size_t SynchronizationPoint::activeTasks()
{
  TP_MUTEX_LOCKER(d->mutex);
  return d->tasks.size();
}

//##################################################################################################
void SynchronizationPoint::removeTask(Task* task)
{
  {
    TP_MUTEX_LOCKER(d->mutex);
    tpRemoveOne(d->tasks, task);
    d->waitCondition.wakeAll();
  }

  if(d->taskRemoved)
    d->taskRemoved();
}

}
