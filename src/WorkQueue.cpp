#include "tp_task_queue/WorkQueue.h"
#include "tp_task_queue/TaskQueue.h"
#include "tp_task_queue/Task.h"
#include "tp_task_queue/SynchronizationPoint.h"

#include "tp_utils/MutexUtils.h"

#include "lib_platform/SetThreadName.h"

#include <queue>
#include <thread>

namespace tp_task_queue
{

//##################################################################################################
struct WorkQueue::Private
{
  std::string taskName;
  TaskQueue* taskQueue{nullptr};

  TPMutex mutex{TPM};
  TPWaitCondition waitCondition;
  std::queue<std::function<void()>> queue;
  bool finish{false};

  std::unique_ptr<std::thread> thread;

  int activeTask{false};
  SynchronizationPoint synchronizationPoint;

  //################################################################################################
  Private(const std::string& threadName):
    thread(new std::thread([=]{run(threadName);}))
  {

  }

  //################################################################################################
  Private(const std::string& taskName_, TaskQueue* taskQueue_):
    taskName(taskName_),
    taskQueue(taskQueue_)
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

    if(thread)
      thread->join();

    synchronizationPoint.cancelTasks();
    synchronizationPoint.join();
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

  //################################################################################################
  void addNextTask()
  {
    if(queue.empty())
      return;

    activeTask = true;

    auto task = queue.front();
    queue.pop();

    auto t = new Task(taskName, [=](Task&)
    {
      task();

      {
        TP_MUTEX_LOCKER(mutex);
        activeTask = false;
        addNextTask();
      }

      return RunAgain::No;
    });
    synchronizationPoint.addTask(t);
    taskQueue->addTask(t);
  }
};

//##################################################################################################
WorkQueue::WorkQueue(const std::string& threadName):
  d(new Private(threadName))
{

}


//##################################################################################################
WorkQueue::WorkQueue(const std::string& taskName, TaskQueue* taskQueue):
  d(new Private(taskName, taskQueue))
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

  if(d->taskQueue && !d->activeTask)
    d->addNextTask();
}

}
