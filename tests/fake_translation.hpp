#ifndef FAKE_TRANSLATION_H_
#define FAKE_TRANSLATION_H_

#include <rime/candidate.h>
#include <rime/translation.h>

using namespace rime;

class FakeTranslation : public Translation {
public:
  FakeTranslation() { set_exhausted(true); }
  bool Next() override {
    if (exhausted()) {
      return false;
    }
    set_exhausted(++iter_ >= candidates_.size());
    return true;
  }

  an<Candidate> Peek() override { return candidates_.at(iter_); }

  void append(const an<Candidate>& candidate) {
    candidates_.push_back(candidate);
    set_exhausted(iter_ >= candidates_.size());
  }

private:
  std::vector<an<Candidate>> candidates_;
  size_t iter_ = 0;
};

#endif  // FAKE_TRANSLATION_H_
