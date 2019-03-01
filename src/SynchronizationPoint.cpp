#include "tp_task_queue/SynchronizationPoint.h"
#include "tp_task_queue/Task.h"

#include "tp_utils/MutexUtils.h"

namespace tp_task_queue
{

//##################################################################################################
struct SynchronizationPoint::Private
{
  size_t count{0};
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
  {
    TP_MUTEX_LOCKER(d->mutex);
    while(d->count>0)
      d->waitCondition.wait(TPMc d->mutex);
  }

  delete d;
}

//##################################################################################################
void SynchronizationPoint::addTask(Task* task, size_t maxActive)
{
  task->setSynchronizationPoint(this);
  TP_MUTEX_LOCKER(d->mutex);
  while(d->count>maxActive)
    d->waitCondition.wait(TPMc d->mutex);
  d->count++;
}

//##################################################################################################
void SynchronizationPoint::removeTask()
{
  TP_MUTEX_LOCKER(d->mutex);
  d->count--;
  d->waitCondition.wakeAll();
}

}
