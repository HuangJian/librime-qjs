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

public:
  EXPORT_CLASS_WITH_RAW_POINTER(
      Context,
      WITH_CONSTRUCTOR(),
      WITH_PROPERTIES(AUTO_PROPERTIES(input, (caretPos, caret_pos))),
      WITH_GETTERS((preedit, std::make_shared<Preedit>(obj->GetPreedit())),
                   (lastSegment, obj->composition().empty() ? nullptr : &obj->composition().back()),
                   (commitNotifier, &obj->commit_notifier()),
                   (selectNotifier, &obj->select_notifier()),
                   (updateNotifier, &obj->update_notifier()),
                   (deleteNotifier, &obj->delete_notifier()),
                   (commitHistory, &obj->commit_history())),
      WITH_FUNCTIONS(commit, getCommitText, clear, hasMenu, getOption, setOption));
};
