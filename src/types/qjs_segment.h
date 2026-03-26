#pragma once

#include <rime/menu.h>
#include <rime/segmentation.h>
#include "engines/js_macros.h"
#include "js_wrapper.h"
#include "types/qjs_candidate.h"

using namespace rime;

template <>
class JsWrapper<Segment> {
  DEFINE_GETTER(Segment, start, obj->start)
  DEFINE_GETTER(Segment, end, obj->end)
  DEFINE_GETTER(Segment, selectedCandidate, obj->GetSelectedCandidate())
  DEFINE_GETTER(Segment, candidateSize, obj->menu->candidate_count())

  DEFINE_CFUNCTION_ARGC(getCandidateAt, 1, {
    auto obj = engine.unwrap<Segment>(thisVal);
    int32_t index = engine.toInt(argv[0]);
    if (index < 0 || size_t(index) >= obj->menu->candidate_count()) {
      return engine.null();
    }
    return engine.wrap(obj->menu->GetCandidateAt(index));
  })

  DEFINE_CFUNCTION_ARGC(hasTag, 1, {
    auto obj = engine.unwrap<Segment>(thisVal);
    std::string tag = engine.toStdString(argv[0]);
    return engine.wrap(obj->HasTag(tag));
  })

public:
  EXPORT_CLASS_WITH_RAW_POINTER(Segment,
                                WITHOUT_CONSTRUCTOR,
                                WITH_PROPERTIES(AUTO_PROPERTIES(prompt,
                                                                (selectedIndex, selected_index))),
                                WITH_GETTERS(start, end, selectedCandidate, candidateSize),
                                WITH_FUNCTIONS(getCandidateAt, hasTag));
};
