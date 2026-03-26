#pragma once

#include <rime/candidate.h>
#include <rime/gear/translator_commons.h>

#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

constexpr int MIN_ARGC_NEW_CANDIDATE = 5;

template <>
class JsWrapper<Candidate> {
  DEFINE_GETTER(Candidate, text, obj->text())
  DEFINE_GETTER(Candidate, comment, obj->comment())
  DEFINE_GETTER(Candidate, preedit, obj->preedit())

  DEFINE_SETTER(Candidate, text, {
    if (auto simpleCandidate = dynamic_cast<rime::SimpleCandidate*>(obj.get())) {
      simpleCandidate->set_text(value);
    }
  })

  DEFINE_SETTER(Candidate, comment, {
    if (auto simpleCandidate = dynamic_cast<rime::SimpleCandidate*>(obj.get())) {
      simpleCandidate->set_comment(value);
    } else if (auto phrase = dynamic_cast<rime::Phrase*>(obj.get())) {
      phrase->set_comment(value);
    }
  })

  DEFINE_SETTER(Candidate, preedit, {
    if (auto simpleCandidate = dynamic_cast<rime::SimpleCandidate*>(obj.get())) {
      simpleCandidate->set_preedit(value);
    } else if (auto phrase = dynamic_cast<rime::Phrase*>(obj.get())) {
      phrase->set_preedit(value);
    }
  })

  DEFINE_CFUNCTION_ARGC(makeCandidate, MIN_ARGC_NEW_CANDIDATE, {
    auto candidate = std::make_shared<rime::SimpleCandidate>();
    candidate->set_type(engine.toStdString(argv[0]));
    candidate->set_start(engine.toInt(argv[1]));
    candidate->set_end(engine.toInt(argv[2]));
    candidate->set_text(engine.toStdString(argv[3]));
    candidate->set_comment(engine.toStdString(argv[4]));
    if (argc > MIN_ARGC_NEW_CANDIDATE) {
      candidate->set_quality(engine.toDouble(argv[5]));
    }
    return engine.wrap<an<Candidate>>(candidate);
  });

public:
  EXPORT_CLASS_WITH_SHARED_POINTER(Candidate,
                                   WITH_CONSTRUCTOR(makeCandidate),
                                   WITH_PROPERTIES(CUSTOM_PROPERTIES(text, comment, preedit),
                                                   AUTO_PROPERTIES(type, start, end, quality)),
                                   WITHOUT_GETTERS,
                                   WITHOUT_FUNCTIONS);
};
