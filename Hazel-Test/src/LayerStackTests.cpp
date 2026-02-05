#include "catch.hpp"
#include "Hazel/LayerStack.h"
#include "Hazel/Layer.h"

namespace Hazel {

    class MockLayer : public Layer
    {
    public:
        MockLayer(const std::string& name)
            : Layer(name) {}
    };

    TEST_CASE("LayerStack functionality", "[LayerStack]")
    {
        LayerStack stack;

        SECTION("Pushing layers and overlays")
        {
            Layer* layer1 = new MockLayer("Layer1");
            Layer* layer2 = new MockLayer("Layer2");
            Layer* overlay1 = new MockLayer("Overlay1");
            Layer* overlay2 = new MockLayer("Overlay2");

            stack.PushLayer(layer1);
            stack.PushLayer(layer2);
            stack.PushOverlay(overlay1);
            stack.PushOverlay(overlay2);

            // Order should be: Layer1, Layer2, Overlay1, Overlay2
            // Actually, PushLayer inserts at m_LayerInsertIndex.
            // PushLayer(layer1): [layer1], insertIndex = 1
            // PushLayer(layer2): [layer2, layer1], insertIndex = 2? No, wait.
            // Let's re-read LayerStack.cpp
            /*
            void LayerStack::PushLayer(Layer* layer)
            {
                m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
                m_LayerInsertIndex++;
            }
            */
            // PushLayer(L1): insert at 0, index becomes 1. Stack: [L1]
            // PushLayer(L2): insert at 1, index becomes 2. Stack: [L1, L2]
            // PushOverlay(O1): emplace_back. Stack: [L1, L2, O1]
            // PushOverlay(O2): emplace_back. Stack: [L1, L2, O1, O2]

            auto it = stack.begin();
            REQUIRE(*(it++) == layer1);
            REQUIRE(*(it++) == layer2);
            REQUIRE(*(it++) == overlay1);
            REQUIRE(*(it++) == overlay2);
            REQUIRE(it == stack.end());
        }

        SECTION("Popping layers")
        {
            Layer* layer1 = new MockLayer("Layer1");
            Layer* layer2 = new MockLayer("Layer2");

            stack.PushLayer(layer1);
            stack.PushLayer(layer2);
            
            stack.PopLayer(layer1);
            
            auto it = stack.begin();
            REQUIRE(*it == layer2);
            it++;
            REQUIRE(it == stack.end());

            // Note: LayerStack destructor deletes remaining layers. 
            // In these tests, we should be careful about memory if we pop but don't delete.
            // But usually LayerStack owns them.
            delete layer1; // PopLayer doesn't delete, it just removes from vector.
        }

        SECTION("Popping overlays")
        {
            Layer* overlay1 = new MockLayer("Overlay1");
            stack.PushOverlay(overlay1);
            
            stack.PopOverlay(overlay1);
            
            REQUIRE(stack.begin() == stack.end());
            delete overlay1;
        }

        SECTION("Complex push/pop order")
        {
            Layer* l1 = new MockLayer("L1");
            Layer* l2 = new MockLayer("L2");
            Layer* o1 = new MockLayer("O1");
            Layer* o2 = new MockLayer("O2");

            stack.PushLayer(l1);      // [L1]
            stack.PushOverlay(o1);    // [L1, O1]
            stack.PushLayer(l2);      // [L1, L2, O1]
            stack.PushOverlay(o2);    // [L1, L2, O1, O2]

            auto it = stack.begin();
            REQUIRE(*(it++) == l1);
            REQUIRE(*(it++) == l2);
            REQUIRE(*(it++) == o1);
            REQUIRE(*(it++) == o2);

            stack.PopLayer(l1);       // [L2, O1, O2]
            it = stack.begin();
            REQUIRE(*(it++) == l2);
            REQUIRE(*(it++) == o1);
            REQUIRE(*(it++) == o2);
            delete l1;

            stack.PopOverlay(o1);     // [L2, O2]
            it = stack.begin();
            REQUIRE(*(it++) == l2);
            REQUIRE(*(it++) == o2);
            delete o1;
        }
    }
}
