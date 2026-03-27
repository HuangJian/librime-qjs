#pragma once

#include <rime/menu.h>
#include <rime/segmentation.h>
#include "engines/js_macros.h"
#include "js_wrapper.h"
#include "types/qjs_candidate.h"

using namespace rime;

template <>
class JsWrapper<Segment> {
  JS_API_DEFINE_CFUNCTION_ARGC(getCandidateAt, 1, {
    int32_t index = engine.toInt(argv[0]);
    if (index < 0 || size_t(index) >= obj->menu->candidate_count()) {
      return engine.null();
    }
    return engine.wrap(obj->menu->GetCandidateAt(index));
  })

  JS_API_DEFINE_CFUNCTION_ARGC(hasTag, 1, {
    std::string tag = engine.toStdString(argv[0]);
    return engine.wrap(obj->HasTag(tag));
  })

public:
  JS_API_EXPORT_CLASS_WITH_RAW_POINTER(
      Segment,
      JS_API_WITH_CONSTRUCTOR(),
      JS_API_WITH_PROPERTIES(JS_API_AUTO_PROPERTIES(prompt, (selectedIndex, selected_index))),
      JS_API_WITH_GETTERS(start,
                          end,
                          (selectedCandidate, obj->GetSelectedCandidate()),
                          (candidateSize, obj->menu->candidate_count())),
      JS_API_WITH_FUNCTIONS(getCandidateAt, hasTag));
};
