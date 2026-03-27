#pragma once

#include <rime/candidate.h>
#include <rime/gear/translator_commons.h>

#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

constexpr int MIN_ARGC_NEW_CANDIDATE = 5;

template <>
class JsWrapper<Candidate> {
  JS_API_DEFINE_GETTER(Candidate, text, obj->text())
  JS_API_DEFINE_GETTER(Candidate, comment, obj->comment())
  JS_API_DEFINE_GETTER(Candidate, preedit, obj->preedit())

  JS_API_DEFINE_SETTER(Candidate, text, {
    if (auto simpleCandidate = dynamic_cast<rime::SimpleCandidate*>(obj.get())) {
      simpleCandidate->set_text(value);
    }
  })

  JS_API_DEFINE_SETTER(Candidate, comment, {
    if (auto simpleCandidate = dynamic_cast<rime::SimpleCandidate*>(obj.get())) {
      simpleCandidate->set_comment(value);
    } else if (auto phrase = dynamic_cast<rime::Phrase*>(obj.get())) {
      phrase->set_comment(value);
    }
  })

  JS_API_DEFINE_SETTER(Candidate, preedit, {
    if (auto simpleCandidate = dynamic_cast<rime::SimpleCandidate*>(obj.get())) {
      simpleCandidate->set_preedit(value);
    } else if (auto phrase = dynamic_cast<rime::Phrase*>(obj.get())) {
      phrase->set_preedit(value);
    }
  })

  JS_API_DEFINE_CFUNCTION_ARGC(makeCandidate, MIN_ARGC_NEW_CANDIDATE, {
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
  JS_API_EXPORT_CLASS_WITH_SHARED_POINTER(
      Candidate,
      JS_API_WITH_CONSTRUCTOR(makeCandidate),
      JS_API_WITH_PROPERTIES(JS_API_AUTO_PROPERTIES(type, start, end, quality),
                             JS_API_CUSTOM_PROPERTIES(text, comment, preedit)),
      JS_API_WITH_GETTERS(),
      JS_API_WITH_FUNCTIONS());
};
