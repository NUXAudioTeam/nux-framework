#include <nxf/project/Value.h>
#include "TestSupport.h"
int main() {
    using namespace nxf::project;
    NXF_CHECK(Value::null().isNull());
    NXF_CHECK(Value::boolean(true).asBool() == true);
    NXF_CHECK(Value::integer(-42).asInt() == -42);
    NXF_CHECK(Value::real(2.5).asReal() == 2.5);
    NXF_CHECK(Value::string("hello").asString() == "hello");
    NXF_CHECK(Value::integer(3).asString().empty());          // wrong type: empty, no throw

    Value root = Value::object();
    root.set("name", Value::string("My Song"));
    root.set("tempo", Value::real(128.5));
    root.set("tracks", Value::array({Value::integer(1), Value::integer(2)}));
    NXF_CHECK(root.get("name")->asString() == "My Song");
    NXF_CHECK(root.get("tracks")->at(1)->asInt() == 2);
    NXF_CHECK(root.get("missing") == nullptr);

    const std::string text = serializeValue(root);
    const auto parsed = parseValue(text);
    NXF_CHECK(parsed.hasValue());
    NXF_CHECK(parsed.value() == root);                        // exact round trip
    // canonical form: keys sorted, format is ours not JSON (key = value;)
    NXF_CHECK(text.find("name = ") != std::string::npos);
    NXF_CHECK(text.find("tempo = 128.5") != std::string::npos);

    const auto escaped = parseValue(serializeValue(Value::string("a\"b\n c")));
    NXF_CHECK(escaped.hasValue());
    NXF_CHECK(escaped.value().asString() == "a\"b\n c");
    NXF_CHECK(!parseValue("{ name = }").hasValue());          // honest parse failures
    NXF_CHECK(!parseValue("hello").hasValue());
    NXF_CHECK(!parseValue("").hasValue());
    NXF_CHECK(!parseValue("{ a = 1; } trailing").hasValue());
    return nxf::test::report("ProjectValueTests");
}
