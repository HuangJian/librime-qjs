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
  DEFINE_GETTER(Candidate, type, obj->type())
  DEFINE_GETTER(Candidate, start, obj->start())
  DEFINE_GETTER(Candidate, end, obj->end())
  DEFINE_GETTER(Candidate, quality, obj->quality())
  DEFINE_GETTER(Candidate, preedit, obj->preedit())

  DEFINE_STRING_SETTER(Candidate, text, {
    if (auto simpleCandidate = dynamic_cast<rime::SimpleCandidate*>(obj.get())) {
      simpleCandidate->set_text(str);
    }
  })

  DEFINE_STRING_SETTER(Candidate, comment, {
    if (auto simpleCandidate = dynamic_cast<rime::SimpleCandidate*>(obj.get())) {
      simpleCandidate->set_comment(str);
    } else if (auto phrase = dynamic_cast<rime::Phrase*>(obj.get())) {
      phrase->set_comment(str);
    }
  })

  DEFINE_STRING_SETTER(Candidate, type, obj->set_type(str);)

  DEFINE_SETTER(Candidate, start, engine.toInt, obj->set_start(value))
  DEFINE_SETTER(Candidate, quality, engine.toDouble, obj->set_quality(value))
  DEFINE_SETTER(Candidate, end, engine.toInt, obj->set_end(value))

  DEFINE_STRING_SETTER(Candidate, preedit, {
    if (auto simpleCandidate = dynamic_cast<rime::SimpleCandidate*>(obj.get())) {
      simpleCandidate->set_preedit(str);
    } else if (auto phrase = dynamic_cast<rime::Phrase*>(obj.get())) {
      phrase->set_preedit(str);
    }
  })

  DEFINE_CFUNCTION_ARGC(makeCandidate, MIN_ARGC_NEW_CANDIDATE, {
    auto obj = std::make_shared<rime::SimpleCandidate>();
    obj->set_type(engine.toStdString(argv[0]));
    obj->set_start(engine.toInt(argv[1]));
    obj->set_end(engine.toInt(argv[2]));
    obj->set_text(engine.toStdString(argv[3]));
    obj->set_comment(engine.toStdString(argv[4]));
    if (argc > MIN_ARGC_NEW_CANDIDATE) {
      obj->set_quality(engine.toDouble(argv[5]));
    }
    return engine.wrap<an<Candidate>>(obj);
  });

public:
  EXPORT_CLASS_WITH_SHARED_POINTER(
      Candidate,
      WITH_CONSTRUCTOR(makeCandidate, MIN_ARGC_NEW_CANDIDATE),
      WITH_PROPERTIES(text, comment, type, start, end, quality, preedit),
      WITHOUT_GETTERS,
      WITHOUT_FUNCTIONS);
};
