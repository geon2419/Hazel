#include "catch.hpp"

#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Events/KeyEvent.h"
#include "Hazel/Events/MouseEvent.h"

#include <sstream>

namespace Hazel
{

TEST_CASE("EventDispatcher dispatches matching event type", "[Event]")
{
    WindowResizeEvent event(1280, 720);
    Event& baseEvent = event;
    EventDispatcher dispatcher(baseEvent);

    const bool dispatched = dispatcher.Dispatch<WindowResizeEvent>(
        [](WindowResizeEvent& e) { return e.GetWidth() == 1280 && e.GetHeight() == 720; });

    REQUIRE(dispatched);
    REQUIRE(event.IsHandled());
}

TEST_CASE("EventDispatcher keeps handled false when callback returns false", "[Event]")
{
    WindowResizeEvent event(1920, 1080);
    Event& baseEvent = event;
    EventDispatcher dispatcher(baseEvent);

    const bool dispatched = dispatcher.Dispatch<WindowResizeEvent>([](WindowResizeEvent&) { return false; });

    REQUIRE(dispatched);
    REQUIRE_FALSE(event.IsHandled());
}

TEST_CASE("EventDispatcher ignores non-matching event type", "[Event]")
{
    WindowResizeEvent event(800, 600);
    Event& baseEvent = event;
    EventDispatcher dispatcher(baseEvent);
    bool callbackCalled = false;

    const bool dispatched = dispatcher.Dispatch<KeyPressedEvent>([&callbackCalled](KeyPressedEvent&) {
        callbackCalled = true;
        return true;
    });

    REQUIRE_FALSE(dispatched);
    REQUIRE_FALSE(callbackCalled);
    REQUIRE_FALSE(event.IsHandled());
}

TEST_CASE("Event category flags and type metadata are consistent", "[Event]")
{
    KeyPressedEvent keyEvent(65, 2);
    MouseMovedEvent mouseEvent(20.0f, 10.0f);
    WindowCloseEvent closeEvent;

    REQUIRE(keyEvent.GetEventType() == EventType::KeyPressed);
    REQUIRE(std::string(keyEvent.GetName()) == "KeyPressed");
    REQUIRE(keyEvent.IsInCategory(EventCategoryKeyboard));
    REQUIRE(keyEvent.IsInCategory(EventCategoryInput));
    REQUIRE_FALSE(keyEvent.IsInCategory(EventCategoryMouse));

    REQUIRE(mouseEvent.GetEventType() == EventType::MouseMoved);
    REQUIRE(mouseEvent.IsInCategory(EventCategoryMouse));
    REQUIRE(mouseEvent.IsInCategory(EventCategoryInput));
    REQUIRE_FALSE(mouseEvent.IsInCategory(EventCategoryApplication));

    REQUIRE(closeEvent.GetEventType() == EventType::WindowClose);
    REQUIRE(closeEvent.IsInCategory(EventCategoryApplication));
    REQUIRE_FALSE(closeEvent.IsInCategory(EventCategoryInput));
}

TEST_CASE("Event ToString and stream output include payload", "[Event]")
{
    KeyPressedEvent keyEvent(70, 3);
    WindowResizeEvent resizeEvent(1024, 768);
    MouseScrolledEvent scrollEvent(1.5f, -2.0f);

    REQUIRE(keyEvent.ToString() == "KeyPressedEvent: 70 (3 repeats)");
    REQUIRE(resizeEvent.ToString() == "WindowResizeEvent: 1024, 768");
    REQUIRE(scrollEvent.ToString() == "MouseScrolledEvent: 1.5, -2");

    std::stringstream stream;
    stream << resizeEvent;
    REQUIRE(stream.str() == "WindowResizeEvent: 1024, 768");
}
} // namespace Hazel
