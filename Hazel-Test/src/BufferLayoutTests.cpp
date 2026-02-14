#include "catch.hpp"

#include "hzpch.h"

#include "Hazel/Renderer/Buffer.h"

namespace Hazel {

	TEST_CASE("ShaderDataTypeSize maps each type to the expected byte size", "[Buffer]")
	{
		REQUIRE(ShaderDataTypeSize(ShaderDataType::Float) == 4);
		REQUIRE(ShaderDataTypeSize(ShaderDataType::Float2) == 8);
		REQUIRE(ShaderDataTypeSize(ShaderDataType::Float3) == 12);
		REQUIRE(ShaderDataTypeSize(ShaderDataType::Float4) == 16);
		REQUIRE(ShaderDataTypeSize(ShaderDataType::Mat3) == 36);
		REQUIRE(ShaderDataTypeSize(ShaderDataType::Mat4) == 64);
		REQUIRE(ShaderDataTypeSize(ShaderDataType::Int) == 4);
		REQUIRE(ShaderDataTypeSize(ShaderDataType::Int2) == 8);
		REQUIRE(ShaderDataTypeSize(ShaderDataType::Int3) == 12);
		REQUIRE(ShaderDataTypeSize(ShaderDataType::Int4) == 16);
		REQUIRE(ShaderDataTypeSize(ShaderDataType::Bool) == 1);
	}

	TEST_CASE("BufferElement returns expected component counts", "[Buffer]")
	{
		REQUIRE(BufferElement(ShaderDataType::Float, "f").GetComponentCount() == 1);
		REQUIRE(BufferElement(ShaderDataType::Float2, "f2").GetComponentCount() == 2);
		REQUIRE(BufferElement(ShaderDataType::Float3, "f3").GetComponentCount() == 3);
		REQUIRE(BufferElement(ShaderDataType::Float4, "f4").GetComponentCount() == 4);
		REQUIRE(BufferElement(ShaderDataType::Mat3, "m3").GetComponentCount() == 9);
		REQUIRE(BufferElement(ShaderDataType::Mat4, "m4").GetComponentCount() == 16);
		REQUIRE(BufferElement(ShaderDataType::Int, "i").GetComponentCount() == 1);
		REQUIRE(BufferElement(ShaderDataType::Int2, "i2").GetComponentCount() == 2);
		REQUIRE(BufferElement(ShaderDataType::Int3, "i3").GetComponentCount() == 3);
		REQUIRE(BufferElement(ShaderDataType::Int4, "i4").GetComponentCount() == 4);
		REQUIRE(BufferElement(ShaderDataType::Bool, "b").GetComponentCount() == 1);
	}

	TEST_CASE("BufferLayout calculates element offsets and stride", "[Buffer]")
	{
		BufferLayout layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_UV" },
			{ ShaderDataType::Float4, "a_Color" },
			{ ShaderDataType::Int, "a_EntityId" }
		};

		REQUIRE(layout.GetStride() == 40);

		const auto& elements = layout.GetElements();
		REQUIRE(elements.size() == 4);
		REQUIRE(elements[0].Name == "a_Position");
		REQUIRE(elements[0].Offset == 0);
		REQUIRE(elements[0].Size == 12);
		REQUIRE(elements[1].Name == "a_UV");
		REQUIRE(elements[1].Offset == 12);
		REQUIRE(elements[1].Size == 8);
		REQUIRE(elements[2].Name == "a_Color");
		REQUIRE(elements[2].Offset == 20);
		REQUIRE(elements[2].Size == 16);
		REQUIRE(elements[3].Name == "a_EntityId");
		REQUIRE(elements[3].Offset == 36);
		REQUIRE(elements[3].Size == 4);
	}
}
