#pragma once

#include <rime/menu.h>
#include <rime/segmentation.h>
#include "engines/js_macros.h"
#include "js_wrapper.h"
#include "types/qjs_candidate.h"

using namespace rime;

template <>
class JsWrapper<Segment> {
  DEFINE_CFUNCTION_ARGC(getCandidateAt, 1, {
    int32_t index = engine.toInt(argv[0]);
    if (index < 0 || size_t(index) >= obj->menu->candidate_count()) {
      return engine.null();
    }
    return engine.wrap(obj->menu->GetCandidateAt(index));
  })

  DEFINE_CFUNCTION_ARGC(hasTag, 1, {
    std::string tag = engine.toStdString(argv[0]);
    return engine.wrap(obj->HasTag(tag));
  })

  DEFINE_PROPERTIES(AUTO_PROPERTIES(prompt, (selectedIndex, selected_index)))
  DEFINE_GETTERS(start,
                 end,
                 (selectedCandidate, obj->GetSelectedCandidate()),
                 (candidateSize, obj->menu->candidate_count()))

public:
  static const JsTypeBinder<raw_ptr_type<Segment>::type>& binder() {
    static const auto b = [] {
      JsTypeBinder<raw_ptr_type<Segment>::type> binder;
#ifdef _ENABLE_JAVASCRIPTCORE
      binder.property("prompt", get_prompt, set_prompt, get_promptJsc, set_promptJsc);
      binder.property("selectedIndex", get_selectedIndex, set_selectedIndex, get_selectedIndexJsc,
                      set_selectedIndexJsc);

      binder.getter("start", get_start, get_startJsc);
      binder.getter("end", get_end, get_endJsc);
      binder.getter("selectedCandidate", get_selectedCandidate, get_selectedCandidateJsc);
      binder.getter("candidateSize", get_candidateSize, get_candidateSizeJsc);

      binder.function("getCandidateAt", static_cast<int>(getCandidateAt_argc), getCandidateAt,
                      getCandidateAtJsc);
      binder.function("hasTag", static_cast<int>(hasTag_argc), hasTag, hasTagJsc);
#else
      binder.property("prompt", get_prompt, set_prompt);
      binder.property("selectedIndex", get_selectedIndex, set_selectedIndex);

      binder.getter("start", get_start);
      binder.getter("end", get_end);
      binder.getter("selectedCandidate", get_selectedCandidate);
      binder.getter("candidateSize", get_candidateSize);

      binder.function("getCandidateAt", static_cast<int>(getCandidateAt_argc), getCandidateAt);
      binder.function("hasTag", static_cast<int>(hasTag_argc), hasTag);
#endif
      return binder;
    }();
    return b;
  }

  static const JSCFunctionListEntry* propertiesQjs() { return binder().propertiesQjs(); }
  static size_t propertiesSize() { return binder().propertiesSize(); }
  static const JSCFunctionListEntry* gettersQjs() { return binder().gettersQjs(); }
  static size_t gettersSize() { return binder().gettersSize(); }
  static const JSCFunctionListEntry* functionsQjs() { return binder().functionsQjs(); }
  static size_t functionsSize() { return binder().functionsSize(); }

#ifdef _ENABLE_JAVASCRIPTCORE
  static const JSStaticValue* binderPropertiesJsc() { return binder().propertiesJsc(); }
  static const JSStaticValue* binderGettersJsc() { return binder().gettersJsc(); }
  static const JSStaticFunction* binderFunctionsJsc() { return binder().functionsJsc(); }
#endif

  EXPORT_CLASS_WITH_RAW_POINTER(Segment,
                                WITH_CONSTRUCTOR(),
                                WITH_PROPERTIES(),
                                WITH_GETTERS(),
                                WITH_FUNCTIONS());
};
