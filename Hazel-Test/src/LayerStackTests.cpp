#include "catch.hpp"

#include "Hazel/Layer.h"
#include "Hazel/LayerStack.h"

#include <vector>

namespace Hazel {

	class MockLayer : public Layer
	{
	public:
		explicit MockLayer(const std::string& name)
			: Layer(name)
		{
		}
	};

	class CountingLayer : public Layer
	{
	public:
		static int DestructorCallCount;

		explicit CountingLayer(const std::string& name)
			: Layer(name)
		{
		}

		~CountingLayer() override
		{
			++DestructorCallCount;
		}
	};

	int CountingLayer::DestructorCallCount = 0;

	static std::vector<Layer*> Snapshot(LayerStack& stack)
	{
		return { stack.begin(), stack.end() };
	}

	TEST_CASE("LayerStack keeps layer and overlay insertion order", "[LayerStack]")
	{
		LayerStack stack;

		Layer* layer1 = new MockLayer("Layer1");
		Layer* layer2 = new MockLayer("Layer2");
		Layer* overlay1 = new MockLayer("Overlay1");
		Layer* overlay2 = new MockLayer("Overlay2");

		stack.PushLayer(layer1);
		stack.PushLayer(layer2);
		stack.PushOverlay(overlay1);
		stack.PushOverlay(overlay2);

		const auto items = Snapshot(stack);
		REQUIRE(items.size() == 4);
		REQUIRE(items[0] == layer1);
		REQUIRE(items[1] == layer2);
		REQUIRE(items[2] == overlay1);
		REQUIRE(items[3] == overlay2);
	}

	TEST_CASE("LayerStack pop removes only the target layer or overlay", "[LayerStack]")
	{
		LayerStack stack;

		Layer* layer1 = new MockLayer("Layer1");
		Layer* layer2 = new MockLayer("Layer2");
		Layer* overlay1 = new MockLayer("Overlay1");
		Layer* overlay2 = new MockLayer("Overlay2");

		stack.PushLayer(layer1);
		stack.PushLayer(layer2);
		stack.PushOverlay(overlay1);
		stack.PushOverlay(overlay2);

		stack.PopLayer(layer1);
		auto items = Snapshot(stack);
		REQUIRE(items.size() == 3);
		REQUIRE(items[0] == layer2);
		REQUIRE(items[1] == overlay1);
		REQUIRE(items[2] == overlay2);

		stack.PopOverlay(overlay1);
		items = Snapshot(stack);
		REQUIRE(items.size() == 2);
		REQUIRE(items[0] == layer2);
		REQUIRE(items[1] == overlay2);

		delete layer1;
		delete overlay1;
	}

	TEST_CASE("LayerStack pop with missing pointer leaves stack unchanged", "[LayerStack]")
	{
		LayerStack stack;

		Layer* layer1 = new MockLayer("Layer1");
		Layer* layer2 = new MockLayer("Layer2");
		Layer* overlay1 = new MockLayer("Overlay1");
		Layer* missingLayer = new MockLayer("MissingLayer");
		Layer* missingOverlay = new MockLayer("MissingOverlay");

		stack.PushLayer(layer1);
		stack.PushLayer(layer2);
		stack.PushOverlay(overlay1);

		const auto before = Snapshot(stack);
		stack.PopLayer(missingLayer);
		stack.PopOverlay(missingOverlay);
		const auto after = Snapshot(stack);

		REQUIRE(before.size() == after.size());
		REQUIRE(after.size() == 3);
		REQUIRE(after[0] == layer1);
		REQUIRE(after[1] == layer2);
		REQUIRE(after[2] == overlay1);

		delete missingLayer;
		delete missingOverlay;
	}

	TEST_CASE("LayerStack handles interleaved push and pop sequence", "[LayerStack]")
	{
		LayerStack stack;

		Layer* l1 = new MockLayer("L1");
		Layer* l2 = new MockLayer("L2");
		Layer* l3 = new MockLayer("L3");
		Layer* o1 = new MockLayer("O1");
		Layer* o2 = new MockLayer("O2");

		stack.PushLayer(l1);   // [L1]
		stack.PushOverlay(o1); // [L1, O1]
		stack.PushLayer(l2);   // [L1, L2, O1]
		stack.PushOverlay(o2); // [L1, L2, O1, O2]
		stack.PushLayer(l3);   // [L1, L2, L3, O1, O2]

		auto items = Snapshot(stack);
		REQUIRE(items.size() == 5);
		REQUIRE(items[0] == l1);
		REQUIRE(items[1] == l2);
		REQUIRE(items[2] == l3);
		REQUIRE(items[3] == o1);
		REQUIRE(items[4] == o2);

		stack.PopLayer(l2); // [L1, L3, O1, O2]
		stack.PopOverlay(o1); // [L1, L3, O2]
		items = Snapshot(stack);
		REQUIRE(items.size() == 3);
		REQUIRE(items[0] == l1);
		REQUIRE(items[1] == l3);
		REQUIRE(items[2] == o2);

		delete l2;
		delete o1;
	}

	TEST_CASE("LayerStack owns only entries still stored in the stack", "[LayerStack]")
	{
		CountingLayer::DestructorCallCount = 0;

		CountingLayer* retainedLayer = new CountingLayer("Retained");
		CountingLayer* removedLayer = new CountingLayer("Removed");

		{
			LayerStack stack;
			stack.PushLayer(retainedLayer);
			stack.PushLayer(removedLayer);

			stack.PopLayer(removedLayer);
			REQUIRE(CountingLayer::DestructorCallCount == 0);
		}

		REQUIRE(CountingLayer::DestructorCallCount == 1);

		delete removedLayer;
		REQUIRE(CountingLayer::DestructorCallCount == 2);
	}
}
