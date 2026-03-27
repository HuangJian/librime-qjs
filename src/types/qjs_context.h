#pragma once

#include <rime/context.h>
#include <memory>
#include "engines/js_macros.h"
#include "js_wrapper.h"

using namespace rime;

template <>
class JsWrapper<Context> {
  JS_API_DEFINE_CFUNCTION(commit, {
    obj->Commit();
    return engine.undefined();
  })

  JS_API_DEFINE_CFUNCTION(getCommitText, { return engine.wrap(obj->GetCommitText()); })

  JS_API_DEFINE_CFUNCTION(clear, {
    obj->Clear();
    return engine.undefined();
  })

  JS_API_DEFINE_CFUNCTION(hasMenu, { return engine.wrap(obj->HasMenu()); })

  JS_API_DEFINE_CFUNCTION_ARGC(getOption, 1, {
    std::string optionName = engine.toStdString(argv[0]);
    return engine.wrap(obj->get_option(optionName));
  })

  JS_API_DEFINE_CFUNCTION_ARGC(setOption, 2, {
    std::string optionName = engine.toStdString(argv[0]);
    bool value = engine.toBool(argv[1]);
    obj->set_option(optionName, value);
    return engine.undefined();
  })

public:
  JS_API_EXPORT_CLASS_WITH_RAW_POINTER(
      Context,
      JS_API_WITH_CONSTRUCTOR(),
      JS_API_WITH_PROPERTIES(JS_API_AUTO_PROPERTIES(input, (caretPos, caret_pos))),
      JS_API_WITH_GETTERS((preedit, std::make_shared<Preedit>(obj->GetPreedit())),
                          (lastSegment,
                           obj->composition().empty() ? nullptr : &obj->composition().back()),
                          (commitNotifier, &obj->commit_notifier()),
                          (selectNotifier, &obj->select_notifier()),
                          (updateNotifier, &obj->update_notifier()),
                          (deleteNotifier, &obj->delete_notifier()),
                          (commitHistory, &obj->commit_history())),
      JS_API_WITH_FUNCTIONS(commit, getCommitText, clear, hasMenu, getOption, setOption));
};
