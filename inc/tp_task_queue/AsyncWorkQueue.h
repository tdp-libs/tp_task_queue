#ifndef tp_task_queue_AsyncWorkQueue_h
#define tp_task_queue_AsyncWorkQueue_h

#include "tp_task_queue/Globals.h"

#include "tp_utils/Progress.h"

#include <functional>

namespace tp_utils
{
class Progress;
}

namespace tp_task_queue
{

//##################################################################################################
class TP_TASK_QUEUE_EXPORT AsyncWorkQueue
{
  TP_NONCOPYABLE(AsyncWorkQueue);
  TP_DQ;
public:
  //################################################################################################
  AsyncWorkQueue(const std::function<bool()>& poll);

  //################################################################################################
  ~AsyncWorkQueue();

  //################################################################################################
  //! Add a task to the queue to be processed.
  void addTask(const std::function<RunAgain(AsyncWorkQueue*)>& task);

  //################################################################################################
  const std::function<bool()>& poll() const;

  //################################################################################################
  bool wait();

  //################################################################################################
  void setOk(bool ok);

  //################################################################################################
  bool ok() const;

  //################################################################################################
  void setShouldStop(bool shouldStop);

  //################################################################################################
  bool shouldStop() const;

  //################################################################################################
  //! Add an error message and set ok to false
  void addError(const std::string& message);

  //################################################################################################
  void addMessage(const std::string& message);

  //################################################################################################
  const std::vector<tp_utils::ProgressMessage>& messages() const;

  //################################################################################################
  tp_utils::CallbackCollection<void()> changed;
};

}

#endif
