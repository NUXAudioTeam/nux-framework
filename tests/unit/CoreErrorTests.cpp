#include <nxf/core/Error.h>
#include <nxf/core/Result.h>
#include "TestSupport.h"
int main() {
    using namespace nxf::core;
    const Error ok;
    NXF_CHECK(ok.isSuccess());
    NXF_CHECK(static_cast<bool>(ok));
    NXF_CHECK(ok.code() == ErrorCode::None);
    const auto bad = Error::invalidArgument("bad value");
    NXF_CHECK(!bad.isSuccess());
    NXF_CHECK(!static_cast<bool>(bad));
    NXF_CHECK(bad.code() == ErrorCode::InvalidArgument);
    NXF_CHECK(bad.message() == "bad value");
    const auto good = Result<int>::success(42);
    NXF_CHECK(good.hasValue());
    NXF_CHECK(good.value() == 42);
    const auto failed = Result<int>::failure(Error::outOfRange("too far"));
    NXF_CHECK(!failed.hasValue());
    NXF_CHECK(failed.error().code() == ErrorCode::OutOfRange);
    const Result<double> moved = Result<double>::success(2.5);
    NXF_CHECK(moved.value() == 2.5);
    return nxf::test::report("CoreErrorTests");
}
