// -*- mode: C++; indent-tabs-mode: nil; -*-
#ifndef   _BLACKBOX_Timer_hh
#define   _BLACKBOX_Timer_hh

extern "C" {
#ifdef    TIME_WITH_SYS_TIME
#  include <sys/time.h>
#  include <time.h>
#else // !TIME_WITH_SYS_TIME
#  ifdef    HAVE_SYS_TIME_H
#    include <sys/time.h>
#  else // !HAVE_SYS_TIME_H
#    include <time.h>
#  endif // HAVE_SYS_TIME_H
#endif // TIME_WITH_SYS_TIME
}

#include <queue>
#include <algorithm>
#include <vector>

namespace ob {

// forward declaration
class OBTimerQueueManager;

class TimeoutHandler {
public:
  virtual void timeout(void) = 0;
};

class OBTimer {
private:
  TimeoutHandler *handler;
  bool timing, recur;

  timeval _start, _timeout;

  OBTimer(const OBTimer&);
  OBTimer& operator=(const OBTimer&);

public:
  OBTimer(TimeoutHandler *h);
  virtual ~OBTimer(void);

  void fireTimeout(void);

  inline bool isTiming(void) const { return timing; }
  inline bool isRecurring(void) const { return recur; }

  inline const timeval &getTimeout(void) const { return _timeout; }
  inline const timeval &getStartTime(void) const { return _start; }

  timeval timeRemaining(const timeval &tm) const;
  bool shouldFire(const timeval &tm) const;
  timeval endpoint(void) const;

  inline void recurring(bool b) { recur = b; }

  void setTimeout(long t);
  void setTimeout(const timeval &t);

  void start(void);  // manager acquires timer
  void stop(void);   // manager releases timer
  void halt(void);   // halts the timer

  bool operator<(const OBTimer& other) const
  { return shouldFire(other.endpoint()); }
};


template <class _Tp, class _Sequence, class _Compare>
class _timer_queue: protected std::priority_queue<_Tp, _Sequence, _Compare> {
public:
  typedef std::priority_queue<_Tp, _Sequence, _Compare> _Base;

  _timer_queue(void): _Base() {}
  ~_timer_queue(void) {}

  void release(const _Tp& value) {
    c.erase(std::remove(c.begin(), c.end(), value), c.end());
    // after removing the item we need to make the heap again
    std::make_heap(c.begin(), c.end(), comp);
  }
  bool empty(void) const { return _Base::empty(); }
  size_t size(void) const { return _Base::size(); }
  void push(const _Tp& value) { _Base::push(value); }
  void pop(void) { _Base::pop(); }
  const _Tp& top(void) const { return _Base::top(); }
private:
  // no copying!
  _timer_queue(const _timer_queue&) {}
  _timer_queue& operator=(const _timer_queue&) {}
};

struct TimerLessThan {
  bool operator()(const OBTimer* const l, const OBTimer* const r) const {
    return *r < *l;
  }
};

typedef _timer_queue<OBTimer*,
                     std::vector<OBTimer*>, TimerLessThan> TimerQueue;

class OBTimerQueueManager {
private:
  TimerQueue timerList;
public:
  OBTimerQueueManager() {}
  virtual ~OBTimerQueueManager() {}

  //! Will wait for and fire the next timer in the queue.
  /*!
    The function will stop waiting if an event is received from the X server.
  */
  virtual void fire();
  
  virtual void addTimer(OBTimer* timer);
  virtual void removeTimer(OBTimer* timer);
};

}

#endif // _BLACKBOX_Timer_hh
