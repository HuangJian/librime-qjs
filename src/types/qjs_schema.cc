#include "qjs_schema.h"
#include "qjs_config.h"

namespace rime {

DEFINE_GETTER(Schema, id, js_new_string_from_std(ctx, obj->schema_id()))
DEFINE_GETTER(Schema, name, js_new_string_from_std(ctx, obj->schema_name()))
DEFINE_GETTER(Schema, config, QjsConfig::Wrap(ctx, obj->config()))
DEFINE_GETTER(Schema, pageSize, JS_NewInt32(ctx, obj->page_size()))
DEFINE_GETTER(Schema, selectKeys, js_new_string_from_std(ctx, obj->select_keys()))

DEFINE_JS_CLASS_WITH_RAW_POINTER(
  Schema,
  NO_CONSTRUCTOR_TO_REGISTER,
  DEFINE_GETTERS(id, name, config, pageSize, selectKeys),
  NO_FUNCTION_TO_REGISTER
)

} // namespace rime
