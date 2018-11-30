#include "cbt.hpp"

const char* to_string(Status s)
{
  switch (s) {
  case Invalid: return "Invalid";
  case Failure: return "Failure";
  case Success: return "Success";
  default: return "Unknown";
  }
}


void Behavior::run()
{
  start();
}

void Behavior::resume()
{
  CBT_LOG();
  return _continue(_status);
}

void Root::start()
{
  return _child->run([self = std::move(*this)](Status s) mutable {
    self._status = s;
    self.resume();
  });
}

void InvertDecorator::start()
{
  CBT_LOG();
  return _child->run([this] (Status s) {
    CBT_LOG("continue with %s", to_string(s));
    switch (s) {
    case Success: _status = Failure; break;
    case Failure: _status = Success; break;
    default:      _status = s;
    }
    resume();
  });
}

void RepeatDecorator::start()
{
  CBT_LOG();
  _count = 0;
  _status = Success;

  //resume earlier
  if (_count == _limit) return resume();
  else return _child->run([this](Status s){
    CBT_LOG("continue %zu/%zu with status %s", _count, _limit, to_string(s));
    _status = s;
    if (++_count == _limit || s == Failure) return resume();
    else return _child->run();
  });
}


auto Composite::at(int i) -> Behavior&
{
  return *_children[i];
}

void SequenceUntil::start()
{
  CBT_LOG();
  _index  = 0;
  _status = _exit_on == Success ? Failure : Success;
  if (_children.size() == 0) return resume();
  else return next();
}
void SequenceUntil::next()
{
  return at(_index).run([this](Status s) {
    CBT_LOG("Continuing %zu/%zu with status %s", _index, _children.size(), to_string(s));
    _status = s;
    if (++_index == _children.size() || s == _exit_on) return resume();
    else return next();
  });
}

