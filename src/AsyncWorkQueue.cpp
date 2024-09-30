#include "tp_task_queue/AsyncWorkQueue.h"

#include "tp_utils/Progress.h"
#include "tp_utils/TPCheckMainThread.h"

namespace tp_task_queue
{

//##################################################################################################
struct AsyncWorkQueue::Private
{
  TPCheckMainThread checkMainThread;

  tp_utils::Progress* progress{nullptr};
  std::function<bool()> poll;

  bool ok{true};
  bool shouldStop{false};
  std::vector<tp_utils::ProgressMessage> messages;

  std::vector<std::function<RunAgain(AsyncWorkQueue*)>*> tasks;

  //################################################################################################
  Private(const std::function<bool()>& poll_):
    poll(poll_)
  {

  }

  //################################################################################################
  Private(tp_utils::Progress* progress_):
    progress(progress_),
    poll([&]{return progress->poll();})
  {

  }
};

//##################################################################################################
AsyncWorkQueue::AsyncWorkQueue(const std::function<bool()>& poll):
  d(new Private(poll))
{

}

//##################################################################################################
AsyncWorkQueue::AsyncWorkQueue(tp_utils::Progress* progress):
  d(new Private(progress))
{

}

//##################################################################################################
AsyncWorkQueue::~AsyncWorkQueue()
{
  d->checkMainThread("AsyncWorkQueue::~AsyncWorkQueue");
  tpDeleteAll(d->tasks);
  delete d;
}

//##################################################################################################
void AsyncWorkQueue::addTask(const std::function<RunAgain(AsyncWorkQueue*)>& task)
{
  d->checkMainThread("AsyncWorkQueue::addTask");
  d->tasks.emplace_back(new std::function<RunAgain(AsyncWorkQueue*)>(task));
}

//##################################################################################################
const std::function<bool()>& AsyncWorkQueue::poll() const
{
  d->checkMainThread("AsyncWorkQueue::poll");
  return d->poll;
}

//##################################################################################################
tp_utils::Progress* AsyncWorkQueue::progress() const
{
  d->checkMainThread("AsyncWorkQueue::poll");
  return d->progress;
}

//##################################################################################################
bool AsyncWorkQueue::wait()
{
  d->checkMainThread("AsyncWorkQueue::wait");

  size_t nextIndex=0;
  while(d->poll() && !d->tasks.empty())
  {
    if(nextIndex>=d->tasks.size())
      nextIndex=0;

    if((*d->tasks.at(nextIndex))(this) == RunAgain::No)
      delete tpTakeAt(d->tasks, nextIndex);
    else
      nextIndex++;
  }

  return d->tasks.empty();
}


//##################################################################################################
void AsyncWorkQueue::setOk(bool ok)
{
  d->checkMainThread("AsyncWorkQueue::setOk");
  d->ok = ok;
  changed();
}

//##################################################################################################
bool AsyncWorkQueue::ok() const
{
  d->checkMainThread("AsyncWorkQueue::ok");
  return d->ok;
}


//##################################################################################################
void AsyncWorkQueue::setShouldStop(bool shouldStop)
{
  d->checkMainThread("AsyncWorkQueue::setShouldStop");
  d->shouldStop = shouldStop;
}

//##################################################################################################
bool AsyncWorkQueue::shouldStop() const
{
  d->checkMainThread("AsyncWorkQueue::shouldStop");
  return d->shouldStop;
}

//##################################################################################################
void AsyncWorkQueue::addError(const std::string& error)
{
  d->checkMainThread("AsyncWorkQueue::addError");

  if(d->progress)
    d->progress->addError(error);

  d->ok = false;
  d->messages.emplace_back(error, true, 0);
  changed();
}

//##################################################################################################
void AsyncWorkQueue::addMessage(const std::string& message)
{
  d->checkMainThread("AsyncWorkQueue::addMessage");

  if(d->progress)
    d->progress->addMessage(message);

  d->messages.emplace_back(message, false, 0);
  changed();
}

//##################################################################################################
const std::vector<tp_utils::ProgressMessage>& AsyncWorkQueue::messages() const
{
  d->checkMainThread("AsyncWorkQueue::messages");
  return d->messages;
}

}
