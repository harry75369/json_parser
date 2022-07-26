#include "json.hpp"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>

inline void skipSpaces(std::string_view::const_iterator &p,
                       const std::string_view::const_iterator &end) {
  while (p != end && std::isspace(*p)) {
    p++;
  }
}

inline std::string readUntil(std::string_view::const_iterator &p,
                             const std::string_view::const_iterator &end, char ch) {
  std::string buff;
  while (p != end && *p != ch) {
    if ((*p == '\\') && (p + 1 != end)) {
      buff += "\\";
      buff += *(p + 1);
      p += 2;
    } else {
      buff += *p;
      p++;
    }
  }
  return buff;
}

std::pair<Json::JsonValue, size_t> Json::parse(const std::string_view &buff) {
  if (buff.empty()) {
    return {nullptr, 0};
  }

  const auto begin = buff.begin();
  const auto end = buff.end();
  auto p = buff.begin();
  skipSpaces(p, end);

  if (*p == '"') { // ---------- for JsonString
    p += 1; // "
    auto str = readUntil(p, end, '"');
    p += 1; // "
    skipSpaces(p, end);
    return {
        std::make_shared<JsonString>(str),
        p - begin,
    };
  } else if (*p == '\'') { // ---------- for JsonString
    p += 1; // '
    auto str = readUntil(p, end, '\'');
    p += 1; // '
    skipSpaces(p, end);
    return {
        std::make_shared<JsonString>(str),
        p - begin,
    };
  } else if (std::isdigit(*p) || *p == '.' || *p == '-') { // ---------- for JsonNumber
    char* pos = 0;
    const char* start = &(*p);
    auto v = std::strtod(start, &pos); // strtod performs better than stod
    if (0 == pos - start) {
      throw "invalid number";
    }
    p += (pos - start);
    skipSpaces(p, end);
    auto parsed = std::string_view(start, pos - start);
    if (parsed.find('.') == std::string_view::npos) {
      auto v = std::strtol(start, nullptr, 10);
      return {
        std::make_shared<JsonNumber>(v),
        p - begin,
      };
    }
    return {
      std::make_shared<JsonNumber>(v),
      p - begin,
    };
  } else if (std::string(p, p + 4) == "true") { // ---------- for JsonBoolean
    p += 4;
    skipSpaces(p, end);
    return {
        std::make_shared<JsonBoolean>(true),
        p - begin,
    };
  } else if (std::string(p, p + 5) == "false") { // ---------- for JsonBoolean
    p += 5;
    skipSpaces(p, end);
    return {
        std::make_shared<JsonBoolean>(false),
        p - begin,
    };
  } else if (std::string(p, p + 4) == "null") { // ---------- for JsonObject
    p += 4;
    skipSpaces(p, end);
    return {
        std::make_shared<JsonObject>(true),
        p - begin,
    };
  } else if (*p == '{') { // ---------- for JsonObject
    p += 1; // {
    skipSpaces(p, end);

    auto obj = std::make_shared<JsonObject>();

    while (true) {
      if (*p == '"') {
        p += 1; // "
        auto key = readUntil(p, end, '"');
        p += 1; // "
        skipSpaces(p, end);
        if (*p != ':') {
          throw "syntax error: colon expected";
        }
        p += 1; // :
        skipSpaces(p, end);
        auto [val, sz] = parse(std::string_view(&(*p), end - p));
        obj->pairs[key] = val;
        p += sz;
        skipSpaces(p, end);
        if (*p == ',') {
          p += 1; // ,
        }
        skipSpaces(p, end);
      } else if (*p != '}') {
        auto key = readUntil(p, end, ':');
        p += 1; // :
        skipSpaces(p, end);
        auto [val, sz] = parse(std::string_view(&(*p), end - p));
        obj->pairs[key] = val;
        p += sz;
        skipSpaces(p, end);
        if (*p == ',') {
          p += 1; // ,
        }
        skipSpaces(p, end);
      } else {
        break;
      }
    }

    p += 1; // }
    skipSpaces(p, end);
    return {obj, p - begin};
  } else if (*p == '[') { // ---------- for JsonArray
    p += 1; // [
    skipSpaces(p, end);

    auto arr = std::make_shared<JsonArray>();

    while (*p != ']') {
      auto [val, sz] = parse(std::string_view(&(*p), end - p));
      arr->values.push_back(val);
      p += sz;
      skipSpaces(p, end);
      if (*p == ',') {
        p += 1; // ,
      }
      skipSpaces(p, end);
    }

    p += 1; // ]
    skipSpaces(p, end);
    return {arr, p - begin};
  }

  return {nullptr, 0};
}

#if 0
std::string_view spaces(int indent) {
  static std::string buff(8, ' ');
  if (indent > buff.size()) {
    buff.resize(2 * buff.size(), ' ');
    return spaces(indent);
  }
  return std::string_view(buff.c_str(), std::max(0, indent));
}

void Json::print(Json::JsonValue value, int indent, bool narrow) {
  if (!value) {
    std::cout << "undefined";
    return;
  }
  if (value->type == "string") {
    if (auto p = std::dynamic_pointer_cast<JsonString>(value)) {
      std::cout << (narrow ? "" : spaces(indent)) << "\"" << p->value << "\"";
    } else {
      throw "type error: string";
    }
  } else if (value->type == "number") {
    if (auto p = std::dynamic_pointer_cast<JsonNumber>(value)) {
      if (p->numType == "integer") {
        std::cout << (narrow ? "" : spaces(indent)) << std::fixed << p->value.integer;
      } else if (p->numType == "floating") {
        static const auto pre = std::numeric_limits<double>::digits10 + 1;
        std::cout << (narrow ? "" : spaces(indent)) << std::defaultfloat << std::setprecision(pre) << p->value.floating;
      } else {
        throw "invalid number type";
      }
    } else {
      throw "type error: number";
    }
  } else if (value->type == "boolean") {
    if (auto p = std::dynamic_pointer_cast<JsonBoolean>(value)) {
      std::cout << (narrow ? "" : spaces(indent))
                << (p->value ? "true" : "false");
    } else {
      throw "type error: boolean";
    }
  } else if (value->type == "object") {
    if (auto p = std::dynamic_pointer_cast<JsonObject>(value)) {
      if (p->isNull) {
        std::cout << (narrow ? "" : spaces(indent)) << "null";
      } else if (p->pairs.size() == 0) {
        std::cout << (narrow ? "" : spaces(indent)) << "{}";
      } else {
        std::cout << (narrow ? "" : spaces(indent)) << "{" << std::endl;
        std::vector<std::string> keys(p->pairs.size());
        std::transform(p->pairs.begin(), p->pairs.end(), keys.begin(), [](auto& pair) { return pair.first; });
        std::sort(keys.begin(), keys.end());
        for (auto &key : keys) {
          std::cout << spaces(indent + 2) << "\"" << key << "\"" << ": ";
          print(p->pairs[key], indent + 2, true);
          if (key != keys.back()) {
            std::cout << ",";
          }
          std::cout << std::endl;
        }
        std::cout << spaces(indent) << "}";
      }
    } else {
      throw "type error: object";
    }
  } else if (value->type == "array") {
    if (auto p = std::dynamic_pointer_cast<JsonArray>(value)) {
      if (p->values.size() == 0) {
        std::cout << (narrow ? "" : spaces(indent)) << "[]";
      } else {
        std::cout << (narrow ? "" : spaces(indent)) << "[" << std::endl;
        for (auto &v : p->values) {
          print(v, indent + 2);
          if (v != p->values.back()) {
            std::cout << ",";
          }
          std::cout << std::endl;
        }
        std::cout << spaces(indent) << "]";
      }
    } else {
      throw "type error: array";
    }
  }
}
#else
void Json::print(Json::JsonValue value, int indent, bool narrow) {
  static const char* spaces = "                                ";
  if (!value) {
    printf("undefined");
    return;
  }
  if (value->type == "string") {
    if (auto p = std::dynamic_pointer_cast<JsonString>(value)) {
      printf("%.*s\"%s\"", narrow ? 0 : indent, spaces, p->value.c_str());
    } else {
      throw "type error: string";
    }
  } else if (value->type == "number") {
    if (auto p = std::dynamic_pointer_cast<JsonNumber>(value)) {
      if (p->numType == "integer") {
        printf("%.*s%ld", narrow ? 0 : indent, spaces, p->value.integer);
      } else if (p->numType == "floating") {
        printf("%.*s%.16g", narrow ? 0 : indent, spaces, p->value.floating);
      } else {
        throw "invalid number type";
      }
    } else {
      throw "type error: number";
    }
  } else if (value->type == "boolean") {
    if (auto p = std::dynamic_pointer_cast<JsonBoolean>(value)) {
      printf("%.*s%s", narrow ? 0 : indent, spaces, p->value ? "true" : "false");
    } else {
      throw "type error: boolean";
    }
  } else if (value->type == "object") {
    if (auto p = std::dynamic_pointer_cast<JsonObject>(value)) {
      if (p->isNull) {
        printf("%.*snull", narrow ? 0 : indent, spaces);
      } else if (p->pairs.size() == 0) {
        printf("%.*s{}", narrow ? 0 : indent, spaces);
      } else {
        printf("%.*s{\n", narrow ? 0 : indent, spaces);
        std::vector<std::string> keys(p->pairs.size());
        std::transform(p->pairs.begin(), p->pairs.end(), keys.begin(), [](auto& pair) { return pair.first; });
        std::sort(keys.begin(), keys.end());
        for (auto &key : keys) {
          printf("%.*s\"%s\": ", indent + 2, spaces, key.c_str());
          print(p->pairs[key], indent + 2, true);
          if (key != keys.back()) {
            printf(",");
          }
          printf("\n");
        }
        printf("%.*s}", indent, spaces);
      }
    } else {
      throw "type error: object";
    }
  } else if (value->type == "array") {
    if (auto p = std::dynamic_pointer_cast<JsonArray>(value)) {
      if (p->values.size() == 0) {
        printf("%.*s[]", narrow ? 0 : indent, spaces);
      } else {
        printf("%.*s[\n", narrow ? 0 : indent, spaces);
        for (auto &v : p->values) {
          print(v, indent + 2);
          if (v != p->values.back()) {
            printf(",");
          }
          printf("\n");
        }
        printf("%.*s]", indent, spaces);
      }
    } else {
      throw "type error: array";
    }
  }
}
#endif
