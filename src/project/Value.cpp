#include <nxf/project/Value.h>

#include <charconv>
#include <cmath>
#include <sstream>

namespace nxf::project {
namespace {

using nxf::core::Error;
using R = nxf::core::Result<Value>;

void writeEscaped(std::string& out, const std::string& text)
{
    out += '"';
    for (const char c : text) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '	': out += "\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    const char* hex = "0123456789ABCDEF";
                    out += "\\u00";
                    out += hex[(c >> 4) & 0xF];
                    out += hex[c & 0xF];
                } else {
                    out += c;
                }
        }
    }
    out += '"';
}

bool isKeyChar(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
}

void writeValue(std::string& out, const Value& value)
{
    if (value.isNull()) { out += "null"; return; }
    if (value.isBool()) { out += value.asBool() ? "true" : "false"; return; }
    if (value.isInt()) { out += std::to_string(value.asInt()); return; }
    if (value.isReal()) {
        char buffer[64];
        const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value.asReal());
        std::string number{buffer, result.ptr};
        if (number.find_first_of(".eE") == std::string::npos && number.find("inf") == std::string::npos
            && number.find("nan") == std::string::npos)
            number += ".0";
        out += number;
        return;
    }
    if (value.isString()) { writeEscaped(out, value.asString()); return; }
    if (value.isArray()) {
        out += '[';
        bool first = true;
        for (const auto& element : *value.asArray()) {
            if (!first) out += ", ";
            first = false;
            writeValue(out, element);
        }
        out += ']';
        return;
    }
    out += "{ ";
    bool first = true;
    for (const auto& [key, element] : *value.asObject()) {
        if (!first) out += ' ';
        first = false;
        bool plainKey = !key.empty();
        for (const char c : key) plainKey = plainKey && isKeyChar(c);
        if (plainKey) out += key; else writeEscaped(out, key);
        out += " = ";
        writeValue(out, element);
        out += ';';
    }
    out += " }";
}

struct Parser {
    const std::string& text;
    std::size_t pos = 0;

    [[nodiscard]] bool atEnd() const { return pos >= text.size(); }
    void skipWhitespace() {
    {
        while (!atEnd()) {
            const char c = text[pos];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') { ++pos; continue; }
            if (c == '#') { while (!atEnd() && text[pos] != '\n') ++pos; continue; }
            break;
        }
    }
    }
    [[nodiscard]] bool consume(char expected) {
        skipWhitespace();
        if (!atEnd() && text[pos] == expected) { ++pos; return true; }
        return false;
    }
    [[nodiscard]] R fail(const char* message) const { return R::failure(Error::invalidArgument(message)); }

    R parseString(std::string& out) {
        // assumes text[pos] == '"'
        ++pos;
        out.clear();
        while (!atEnd()) {
            const char c = text[pos++];
            if (c == '"') return R::success(Value::null());
            if (c != '\\') { out += c; continue; }
            if (atEnd()) break;
            const char escape = text[pos++];
            switch (escape) {
                case '"': out += '"'; break;
                case '\\': out += "\\\\"; break;
                case '/': out += '/'; break;
                case 'n': out += '\n'; break;
                case 'r': out += '\r'; break;
                case 't': out += '	'; break;
                case 'u': {
                    if (pos + 4 > text.size()) return fail("bad unicode escape");
                    unsigned code = 0;
                    for (int i = 0; i < 4; ++i) {
                        const char h = text[pos++];
                        code <<= 4;
                        if (h >= '0' && h <= '9') code |= static_cast<unsigned>(h - '0');
                        else if (h >= 'a' && h <= 'f') code |= static_cast<unsigned>(h - 'a' + 10);
                        else if (h >= 'A' && h <= 'F') code |= static_cast<unsigned>(h - 'A' + 10);
                        else return fail("bad unicode escape");
                    }
                    if (code > 0x7F) return fail("only ASCII unicode escapes are supported");
                    out += static_cast<char>(code);
                    break;
                }
                default: return fail("unknown escape");
            }
        }
        return fail("unterminated string");
    }

    R parseNumber() {
        const std::size_t start = pos;
        if (!atEnd() && (text[pos] == '-' || text[pos] == '+')) ++pos;
        bool isReal = false;
        while (!atEnd()) {
            const char c = text[pos];
            if (c >= '0' && c <= '9') { ++pos; continue; }
            if (c == '.' || c == 'e' || c == 'E') { isReal = true; ++pos; continue; }
            if ((c == '-' || c == '+') && pos > start && (text[pos - 1] == 'e' || text[pos - 1] == 'E')) { ++pos; continue; }
            break;
        }
        const std::string token = text.substr(start, pos - start);
        if (token.empty() || token == "-" || token == "+") return fail("bad number");
        try {
            if (isReal) {
                std::size_t used = 0;
                const double value = std::stod(token, &used);
                if (used != token.size() || !std::isfinite(value)) return fail("bad number");
                return R::success(Value::real(value));
            }
            std::size_t used = 0;
            const auto value = std::stoll(token, &used);
            if (used != token.size()) return fail("bad number");
            return R::success(Value::integer(value));
        } catch (...) {
            return fail("bad number");
        }
    }

    R parseKeyword(const char* word, const Value& value) {
        const std::string expected{word};
        if (text.compare(pos, expected.size(), expected) == 0) {
            pos += expected.size();
            return R::success(value);
        }
        return fail("unknown literal");
    }

    R parse() {
        skipWhitespace();
        if (atEnd()) return fail("empty input");
        const char c = text[pos];
        if (c == '{') {
            ++pos;
            Value::Object object;
            while (true) {
                skipWhitespace();
                if (consume('}')) return R::success(Value::object(std::move(object)));
                if (atEnd()) return fail("unterminated object");
                std::string key;
                if (text[pos] == '"') {
                    if (auto parsed = parseString(key); !parsed.hasValue()) return parsed;
                } else {
                    const std::size_t begin = pos;
                    while (!atEnd() && isKeyChar(text[pos])) ++pos;
                    if (pos == begin) return fail("expected a key");
                    key = text.substr(begin, pos - begin);
                }
                if (!consume('=')) return fail("expected = after key");
                auto element = parse();
                if (!element.hasValue()) return element;
                object[key] = element.value();
                if (!consume(';')) return fail("expected ; after value");
            }
        }
        if (c == '[') {
            ++pos;
            Value::Array array;
            if (consume(']')) return R::success(Value::array(std::move(array)));
            while (true) {
                auto element = parse();
                if (!element.hasValue()) return element;
                array.push_back(element.value());
                if (consume(']')) return R::success(Value::array(std::move(array)));
                if (!consume(',')) return fail("expected , or ] in array");
            }
        }
        if (c == '"') {
            std::string out;
            if (auto parsed = parseString(out); !parsed.hasValue()) return parsed;
            return R::success(Value::string(std::move(out)));
        }
        if (c == 't') return parseKeyword("true", Value::boolean(true));
        if (c == 'f') return parseKeyword("false", Value::boolean(false));
        if (c == 'n') return parseKeyword("null", Value::null());
        if (c == '-' || c == '+' || (c >= '0' && c <= '9')) return parseNumber();
        return fail("unexpected character");
    }
};

} // namespace

std::string serializeValue(const Value& value)
{
    std::string out;
    writeValue(out, value);
    return out;
}

R parseValue(const std::string& text)
{
    Parser parser{text};
    auto value = parser.parse();
    if (!value.hasValue()) return value;
    parser.skipWhitespace();
    if (!parser.atEnd()) return R::failure(Error::invalidArgument("trailing content after value"));
    return value;
}

} // namespace nxf::project
