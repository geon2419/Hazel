#include "catch.hpp"

#include "Hazel/Core/Timestep.h"

namespace Hazel
{

TEST_CASE("Timestep default value is zero", "[Timestep]")
{
    Timestep timestep;

    REQUIRE(timestep.GetSeconds() == Approx(0.0f));
    REQUIRE(timestep.GetMilliseconds() == Approx(0.0f));
    REQUIRE(static_cast<float>(timestep) == Approx(0.0f));
}

TEST_CASE("Timestep conversion stays consistent for frame time", "[Timestep]")
{
    Timestep timestep(0.016f);

    REQUIRE(timestep.GetSeconds() == Approx(0.016f).epsilon(0.0001f));
    REQUIRE(timestep.GetMilliseconds() == Approx(16.0f).epsilon(0.0001f));
    REQUIRE(static_cast<float>(timestep) == Approx(timestep.GetSeconds()));
}

TEST_CASE("Timestep conversion stays consistent for larger values", "[Timestep]")
{
    Timestep timestep(1.5f);

    REQUIRE(timestep.GetSeconds() == Approx(1.5f));
    REQUIRE(timestep.GetMilliseconds() == Approx(1500.0f));
    REQUIRE(static_cast<float>(timestep) == Approx(1.5f));
}
} // namespace Hazel
