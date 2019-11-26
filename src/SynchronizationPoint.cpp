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
  Private() = default;

  std::vector<Task*> tasks;
  TPMutex mutex{TPM};
  TPWaitCondition waitCondition;
};

//##################################################################################################
SynchronizationPoint::SynchronizationPoint():
  d(new Private())
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
  TP_MUTEX_LOCKER(d->mutex);
  while(!d->tasks.empty())
    d->waitCondition.wait(TPMc d->mutex);
}

//##################################################################################################
void SynchronizationPoint::addTask(Task* task, size_t maxActive)
{
  task->setSynchronizationPoint(this);
  TP_MUTEX_LOCKER(d->mutex);
  while(d->tasks.size()>=maxActive)
    d->waitCondition.wait(TPMc d->mutex);
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
  TP_MUTEX_LOCKER(d->mutex);
  tpRemoveOne(d->tasks, task);
  d->waitCondition.wakeAll();
}

}
