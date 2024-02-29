#include "tp_task_queue/WorkQueue.h"

#include "tp_utils/MutexUtils.h"

#include "lib_platform/SetThreadName.h"

#include <queue>
#include <thread>

namespace tp_task_queue
{

//##################################################################################################
struct WorkQueue::Private
{
  TPMutex mutex{TPM};
  TPWaitCondition waitCondition;
  std::queue<std::function<void()>> queue;
  bool finish{false};
  std::thread thread;

  //################################################################################################
  Private(const std::string& threadName):
    thread([=]{run(threadName);})
  {

  }

  //################################################################################################
  ~Private()
  {
    {
      TP_MUTEX_LOCKER(mutex);
      finish = true;
      waitCondition.wakeAll();
    }

    thread.join();
  }

  //################################################################################################
  void run(const std::string& threadName)
  {
    lib_platform::setThreadName(threadName);

    TPMutexLocker lock(mutex);
    while(!finish || !queue.empty())
    {
      if(queue.empty())
        waitCondition.wait(TPMc lock);
      else
      {
        auto task = queue.front();
        queue.pop();

        {
          TP_MUTEX_UNLOCKER(lock);
          task();
        }
      }
    }
  }
};

//##################################################################################################
WorkQueue::WorkQueue(const std::string& threadName):
  d(new Private(threadName))
{

}

//##################################################################################################
WorkQueue::~WorkQueue()
{
  delete d;
}

//##################################################################################################
void WorkQueue::addTask(const std::function<void()>& task)
{
  TP_MUTEX_LOCKER(d->mutex);
  d->queue.push(task);
  d->waitCondition.wakeOne();
}

}
