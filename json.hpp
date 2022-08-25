#ifndef __JSON_HPP__
#define __JSON_HPP__

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

struct Json {

  struct JsonBase {
    JsonBase(const std::string &type) : type(type) {}
    virtual ~JsonBase() {}
    std::string type;
  };

  struct JsonString : JsonBase {
    JsonString(const std::string &val) : JsonBase("string"), value(val) {}
    std::string value;
  };

  struct JsonNumber : JsonBase {
    JsonNumber(double val) : JsonBase("number"), numType("floating"), value{ .floating = val } {}
    JsonNumber(long val) : JsonBase("number"), numType("integer"), value{ .integer = val } {}
    std::string numType;
    union {
      double floating;
      long integer;
    } value;
  };

  struct JsonBoolean : JsonBase {
    JsonBoolean(bool val) : JsonBase("boolean"), value(val) {}
    bool value;
  };

  using JsonValue = std::shared_ptr<JsonBase>;

  struct JsonObject : JsonBase {
    JsonObject(bool isNull = false) : JsonBase("object"), isNull(isNull) {}
    std::unordered_map<std::string, JsonValue> pairs;
    bool isNull;
  };

  struct JsonArray : JsonBase {
    JsonArray() : JsonBase("array") {}
    std::vector<JsonValue> values;
  };

  static std::pair<JsonValue, size_t> parse(const std::string_view &buff);

  static void print(JsonValue value, int indent = 0, bool narrow = false);
};

#endif //__JSON_HPP__
