#pragma once

#include <rime/context.h>
#include <memory>
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<Context> {
  DEFINE_CFUNCTION(commit, {
    obj->Commit();
    return engine.undefined();
  })

  DEFINE_CFUNCTION(getCommitText, { return engine.wrap(obj->GetCommitText()); })

  DEFINE_CFUNCTION(clear, {
    obj->Clear();
    return engine.undefined();
  })

  DEFINE_CFUNCTION(hasMenu, { return engine.wrap(obj->HasMenu()); })

  DEFINE_CFUNCTION_ARGC(getOption, 1, {
    std::string optionName = engine.toStdString(argv[0]);
    return engine.wrap(obj->get_option(optionName));
  })

  DEFINE_CFUNCTION_ARGC(setOption, 2, {
    std::string optionName = engine.toStdString(argv[0]);
    bool value = engine.toBool(argv[1]);
    obj->set_option(optionName, value);
    return engine.undefined();
  })

  DEFINE_PROPERTIES(AUTO_PROPERTIES(input, (caretPos, caret_pos)))
  DEFINE_GETTERS((preedit, std::make_shared<Preedit>(obj->GetPreedit())),
                 (lastSegment, obj->composition().empty() ? nullptr : &obj->composition().back()),
                 (commitNotifier, &obj->commit_notifier()),
                 (selectNotifier, &obj->select_notifier()),
                 (updateNotifier, &obj->update_notifier()),
                 (deleteNotifier, &obj->delete_notifier()),
                 (commitHistory, &obj->commit_history()))

public:
  static const JsTypeBinder<raw_ptr_type<Context>::type>& binder() {
    static const auto b = [] {
      JsTypeBinder<raw_ptr_type<Context>::type> binder;
#ifdef _ENABLE_JAVASCRIPTCORE
      binder.property("input", get_input, set_input, get_inputJsc, set_inputJsc);
      binder.property("caretPos", get_caretPos, set_caretPos, get_caretPosJsc, set_caretPosJsc);

      binder.getter("preedit", get_preedit, get_preeditJsc);
      binder.getter("lastSegment", get_lastSegment, get_lastSegmentJsc);
      binder.getter("commitNotifier", get_commitNotifier, get_commitNotifierJsc);
      binder.getter("selectNotifier", get_selectNotifier, get_selectNotifierJsc);
      binder.getter("updateNotifier", get_updateNotifier, get_updateNotifierJsc);
      binder.getter("deleteNotifier", get_deleteNotifier, get_deleteNotifierJsc);
      binder.getter("commitHistory", get_commitHistory, get_commitHistoryJsc);

      binder.function("commit", static_cast<int>(commit_argc), commit, commitJsc);
      binder.function("getCommitText", static_cast<int>(getCommitText_argc), getCommitText,
                      getCommitTextJsc);
      binder.function("clear", static_cast<int>(clear_argc), clear, clearJsc);
      binder.function("hasMenu", static_cast<int>(hasMenu_argc), hasMenu, hasMenuJsc);
      binder.function("getOption", static_cast<int>(getOption_argc), getOption, getOptionJsc);
      binder.function("setOption", static_cast<int>(setOption_argc), setOption, setOptionJsc);
#else
      binder.property("input", get_input, set_input);
      binder.property("caretPos", get_caretPos, set_caretPos);

      binder.getter("preedit", get_preedit);
      binder.getter("lastSegment", get_lastSegment);
      binder.getter("commitNotifier", get_commitNotifier);
      binder.getter("selectNotifier", get_selectNotifier);
      binder.getter("updateNotifier", get_updateNotifier);
      binder.getter("deleteNotifier", get_deleteNotifier);
      binder.getter("commitHistory", get_commitHistory);

      binder.function("commit", static_cast<int>(commit_argc), commit);
      binder.function("getCommitText", static_cast<int>(getCommitText_argc), getCommitText);
      binder.function("clear", static_cast<int>(clear_argc), clear);
      binder.function("hasMenu", static_cast<int>(hasMenu_argc), hasMenu);
      binder.function("getOption", static_cast<int>(getOption_argc), getOption);
      binder.function("setOption", static_cast<int>(setOption_argc), setOption);
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

  EXPORT_CLASS_WITH_RAW_POINTER(Context,
                                WITH_CONSTRUCTOR(),
                                WITH_PROPERTIES(),
                                WITH_GETTERS(),
                                WITH_FUNCTIONS());
};
