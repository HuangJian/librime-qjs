#include <rime/gear/translator_commons.h>
#include "qjs_candidate.h"
#include "qjs_macros.h"
#include <memory>

namespace rime {

DEFINE_JS_CLASS(Candidate,
  DEFINE_PROPERTIES(text, comment, type, start, end, quality, preedit),
  // no functions to register
)

DEFINE_GETTER(Candidate, text, const string&, js_new_string_from_std)
DEFINE_GETTER(Candidate, comment, const string&, js_new_string_from_std)
DEFINE_GETTER(Candidate, type, const string&, js_new_string_from_std)
DEFINE_GETTER(Candidate, start, size_t, JS_NewInt64)
DEFINE_GETTER(Candidate, end, size_t, JS_NewInt64)
DEFINE_GETTER(Candidate, quality, int, JS_NewInt32)
DEFINE_GETTER(Candidate, preedit, const string&, js_new_string_from_std)

DEFINE_STRING_SETTER(Candidate, text,
  if (auto simpleCandidate = dynamic_cast<SimpleCandidate*>(obj.get())) {
    simpleCandidate->set_text(str);
  }
)

DEFINE_STRING_SETTER(Candidate, comment,
  if (auto simpleCandidate = dynamic_cast<SimpleCandidate*>(obj.get())) {
    simpleCandidate->set_comment(str);
  } else if (auto phrase = dynamic_cast<Phrase*>(obj.get())) {
    phrase->set_comment(str);
  }
)

DEFINE_STRING_SETTER(Candidate, type,
  obj->set_type(str);
)

DEFINE_NUMERIC_SETTER(Candidate, start, int64_t, JS_ToInt64)
DEFINE_NUMERIC_SETTER(Candidate, end, int64_t, JS_ToInt64)
DEFINE_NUMERIC_SETTER(Candidate, quality, int32_t, JS_ToInt32)

DEFINE_STRING_SETTER(Candidate, preedit,
  if (auto simpleCandidate = dynamic_cast<SimpleCandidate*>(obj.get())) {
    simpleCandidate->set_preedit(str);
  } else if (auto phrase = dynamic_cast<Phrase*>(obj.get())) {
    phrase->set_preedit(str);
  }
)

} // namespace rime
