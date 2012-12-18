#include <mruby.h>
#include <mruby/string.h>
#include <mruby/array.h>
#include <mruby/hash.h>
#include "parson.h"

static struct RClass *_class_json;

/*********************************************************
 * main
 *********************************************************/

static mrb_value
mrb_value_to_string(mrb_state* mrb, mrb_value value) {
  switch (mrb_type(value)) {
  case MRB_TT_FIXNUM:
  case MRB_TT_FLOAT:
  case MRB_TT_TRUE:
  case MRB_TT_FALSE:
  case MRB_TT_UNDEF:
  case MRB_TT_STRING:
    return value;
  case MRB_TT_HASH:
    return value;
  case MRB_TT_ARRAY:
  default:
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid argument");
  }
  return mrb_false_value();
}

static mrb_value
json_value_to_mrb_value(mrb_state* mrb, JSON_Value* value) {
  mrb_value hash;
  mrb_value ary;
  JSON_Object* object;
  JSON_Array* array;
  size_t count;
  int n;

  switch (json_value_get_type(value)) {
  case JSONError:
  case JSONNull:
    return mrb_nil_value();
  case JSONString:
    return mrb_str_new_cstr(mrb, json_value_get_string(value));
  case JSONNumber:
    return mrb_float_value(json_value_get_number(value));
  case JSONObject:
    hash = mrb_hash_new(mrb);
    object = json_value_get_object(value);
    count = json_object_get_count(object);
    for (n = 0; n < count; n++) {
      const char* name = json_object_get_name(object, n);
      mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, name),
        json_value_to_mrb_value(mrb, json_object_get_value(object, name)));
    }
    return hash;
  case JSONArray:
    ary = mrb_ary_new(mrb);
    array = json_value_get_array(value);
    count = json_array_get_count(array);
    for (n = 0; n < count; n++) {
      JSON_Value* elem = json_array_get_value(array, n);
      mrb_ary_push(mrb, ary, json_value_to_mrb_value(mrb, elem));
    }
    return ary;
  case JSONBoolean:
    if (json_value_get_boolean(value)) {
      return mrb_true_value();
    }
    return mrb_false_value();
  }
  return mrb_nil_value();
}

static mrb_value
mrb_json_parse(mrb_state *mrb, mrb_value self)
{
  mrb_value json = mrb_nil_value();
  mrb_get_args(mrb, "o", &json);
  if (!mrb_string_p(json)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid argument");
  }

  JSON_Value *root_value = json_parse_string(RSTRING_PTR(json));
  if (!root_value) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid json");
  }

  return json_value_to_mrb_value(mrb, root_value);
}

static mrb_value
mrb_json_stringify(mrb_state *mrb, mrb_value self)
{
  int argc;
  mrb_value* argv = NULL;
  if (mrb_get_args(mrb, "*", &argv, &argc) != 1) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid argument");
  }

  return mrb_value_to_string(mrb, argv[0]);
}

/*********************************************************
 * register
 *********************************************************/

void
mrb_mruby_json_gem_init(mrb_state* mrb) {
  _class_json = mrb_define_module(mrb, "JSON");
  mrb_define_class_method(mrb, _class_json, "parse", mrb_json_parse, ARGS_REQ(1));
  mrb_define_class_method(mrb, _class_json, "stringify", mrb_json_stringify, ARGS_REQ(1));
}

/* vim:set et ts=2 sts=2 sw=2 tw=0: */