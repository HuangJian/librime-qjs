#pragma once

#include "engines/common.h"
#include "qjs_candidate_iterator.h"
#include "qjs_commit_history.h"
#include "qjs_commit_record.h"
#include "qjs_config.h"
#include "qjs_config_item.h"
#include "qjs_config_list.h"
#include "qjs_config_map.h"
#include "qjs_config_value.h"
#include "qjs_context.h"
#include "qjs_engine.h"
#include "qjs_environment.h"
#include "qjs_key_event.h"
#include "qjs_leveldb.h"
#include "qjs_notifier.h"
#include "qjs_notifier_connection.h"
#include "qjs_os_info.h"
#include "qjs_preedit.h"
#include "qjs_schema.h"
#include "qjs_segment.h"
#include "qjs_trie.h"

template <typename T_JS_VALUE>
void registerTypesToJsEngine() {
  JsEngine<T_JS_VALUE>& engine = JsEngine<T_JS_VALUE>::instance();
  DLOG(INFO) << "[qjs] registering rime types to the " << engine.engineName << " engine...";

  // expose all types
  engine.template registerType<Candidate>();
  engine.template registerType<Translation>();
  engine.template registerType<Trie>();
  engine.template registerType<LevelDb>();
  engine.template registerType<Segment>();
  engine.template registerType<KeyEvent>();
  engine.template registerType<Context>();
  engine.template registerType<Preedit>();
  engine.template registerType<Schema>();
  engine.template registerType<Config>();
  engine.template registerType<Engine>();
  engine.template registerType<ConfigItem>();
  engine.template registerType<ConfigValue>();
  engine.template registerType<ConfigList>();
  engine.template registerType<ConfigMap>();
  engine.template registerType<Environment>();
  engine.template registerType<SystemInfo>();
  engine.template registerType<CommitRecord>();
  engine.template registerType<CommitHistory>();
  engine.template registerType<NotifierConnection>();
  engine.template registerType<Notifier>();
}
