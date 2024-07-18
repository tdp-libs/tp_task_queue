#include "tp_task_queue/AsyncWorkQueue.h"

#include "tp_utils/Progress.h"

namespace tp_task_queue
{

//##################################################################################################
struct AsyncWorkQueue::Private
{
  std::function<bool()> poll;

  bool ok{true};
  bool shouldStop{false};
  std::vector<tp_utils::ProgressMessage> messages;

  std::vector<std::function<RunAgain(AsyncWorkQueue*)>*> tasks;

  Private(const std::function<bool()>& poll_):
    poll(poll_)
  {

  }
};

//##################################################################################################
AsyncWorkQueue::AsyncWorkQueue(const std::function<bool()>& poll):
  d(new Private(poll))
{

}

//##################################################################################################
AsyncWorkQueue::~AsyncWorkQueue()
{
  tpDeleteAll(d->tasks);
  delete d;
}

//##################################################################################################
void AsyncWorkQueue::addTask(const std::function<RunAgain(AsyncWorkQueue*)>& task)
{
  d->tasks.emplace_back(new std::function<RunAgain(AsyncWorkQueue*)>(task));
}

//##################################################################################################
const std::function<bool()>& AsyncWorkQueue::poll() const
{
  return d->poll;
}

//##################################################################################################
bool AsyncWorkQueue::wait()
{
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
  d->ok = ok;
  changed();
}

//##################################################################################################
bool AsyncWorkQueue::ok() const
{
  return d->ok;
}


//##################################################################################################
void AsyncWorkQueue::setShouldStop(bool shouldStop)
{
  d->shouldStop = shouldStop;
}

//##################################################################################################
bool AsyncWorkQueue::shouldStop() const
{
  return d->shouldStop;
}

//##################################################################################################
void AsyncWorkQueue::addError(const std::string& message)
{
  d->ok = false;
  d->messages.emplace_back(message, true, 0);
  changed();
}

//##################################################################################################
void AsyncWorkQueue::addMessage(const std::string& message)
{
  d->messages.emplace_back(message, false, 0);
  changed();
}

//##################################################################################################
const std::vector<tp_utils::ProgressMessage>& AsyncWorkQueue::messages() const
{
  return d->messages;
}

}
