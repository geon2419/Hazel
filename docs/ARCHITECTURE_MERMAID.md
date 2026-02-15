# Hazel Game Engine - Architecture (Mermaid Diagrams)

## 1. 전체 프로젝트 구조

```mermaid
graph TB
    subgraph "Hazel Project"
        Sandbox[Sandbox<br/>ConsoleApp]
        Hazel[Hazel Engine<br/>SharedLib DLL]
    end
    
    subgraph "Third Party Libraries"
        GLFW[GLFW<br/>Window Management]
        Glad[Glad<br/>OpenGL Loader]
        ImGui[ImGui<br/>GUI Framework]
        glm[glm<br/>Math Library]
        spdlog[spdlog<br/>Logging]
    end
    
    subgraph "System"
        OpenGL[OpenGL<br/>Graphics API]
        OS[Operating System<br/>Windows]
    end
    
    Sandbox -->|Links to| Hazel
    Hazel -->|Uses| GLFW
    Hazel -->|Uses| Glad
    Hazel -->|Uses| ImGui
    Hazel -->|Uses| glm
    Hazel -->|Uses| spdlog
    Glad -->|Loads| OpenGL
    GLFW -->|Binds to| OpenGL
    GLFW -->|Platform| OS
```

## 2. Hazel 엔진 코어 아키텍처

```mermaid
graph TB
    subgraph "Entry & Core"
        Entry[EntryPoint.h<br/>Main Entry Point]
        App[Application<br/>Core Engine Class]
        Core[Core.h<br/>Macros & Defines]
    end
    
    subgraph "Layer System"
        LayerStack[LayerStack<br/>Layer Manager]
        Layer[Layer<br/>Base Class]
        ImGuiLayer[ImGuiLayer<br/>Debug UI Layer]
    end
    
    subgraph "Window System"
        WindowInt[Window<br/>Interface]
        WinWindow[WindowsWindow<br/>GLFW Implementation]
    end

    subgraph "Renderer"
        ContextInt[GraphicsContext<br/>Interface]
        GLContext[OpenGLContext<br/>Implementation]
    end
    
    subgraph "Event System"
        Event[Event<br/>Base Class]
        AppEvent[ApplicationEvent]
        KeyEvent[KeyEvent]
        MouseEvent[MouseEvent]
        WindowEvent[WindowEvent]
    end
    
    subgraph "Input System"
        Input[Input<br/>Interface]
        WinInput[WindowsInput<br/>Implementation]
        KeyCodes[KeyCodes]
        MouseCodes[MouseButtonCodes]
    end
    
    subgraph "Logging"
        Log[Log<br/>spdlog wrapper]
    end
    
    Entry -->|Creates & Runs| App
    App -->|Manages| LayerStack
    App -->|Owns| WindowInt
    App -->|Uses| Log
    LayerStack -->|Contains| Layer
    ImGuiLayer -.->|Inherits| Layer
    WindowInt <-.->|Implements| WinWindow
    WindowInt -->|Uses| ContextInt
    ContextInt <-.->|Implements| GLContext
    Input <-.->|Implements| WinInput
    WinInput -->|Uses| KeyCodes
    WinInput -->|Uses| MouseCodes
    
    Event <-.->|Inherits| AppEvent
    Event <-.->|Inherits| KeyEvent
    Event <-.->|Inherits| MouseEvent
    Event <-.->|Inherits| WindowEvent
    
    WinWindow -->|Generates| Event
    App -->|Dispatches| Event
    LayerStack -->|Propagates| Event
```

## 3. 이벤트 처리 흐름

```mermaid
sequenceDiagram
    participant OS as Operating System
    participant GLFW as GLFW Library
    participant Win as WindowsWindow
    participant App as Application
    participant Stack as LayerStack
    participant L1 as Layer 1
    participant L2 as Layer N
    
    OS->>GLFW: System Event
    GLFW->>Win: GLFW Callback
    Win->>Win: Create Hazel Event
    Win->>App: OnEvent(event)
    App->>Stack: OnEvent(event)
    
    Note over Stack: Propagate in reverse order
    Stack->>L2: OnEvent(event)
    alt Event Not Handled
        L2-->>Stack: Continue
        Stack->>L1: OnEvent(event)
        L1-->>Stack: Event Handled
    else Event Handled
        L2-->>Stack: Stop Propagation
    end
```

## 4. 애플리케이션 생명주기

```mermaid
stateDiagram-v2
    [*] --> Initialize
    Initialize --> CreateWindow
    CreateWindow --> SetupLayers
    SetupLayers --> GameLoop
    
    state GameLoop {
        [*] --> UpdateLayers
        UpdateLayers --> RenderImGui
        RenderImGui --> SwapBuffers
        SwapBuffers --> PollEvents
        PollEvents --> CheckClose
        CheckClose --> UpdateLayers: Continue
        CheckClose --> [*]: Should Close
    }
    
    GameLoop --> Shutdown
    Shutdown --> [*]
```

## 5. 렌더링 파이프라인

```mermaid
flowchart TD
    Start([Application Run Loop]) --> A{Window Should Close?}
    A -->|No| B[Update All Layers]
    A -->|Yes| End([Shutdown])
    
    B --> C[Begin ImGui Frame]
    C --> D[Render ImGui for Each Layer]
    D --> E[End ImGui Frame]
    E --> F[Render ImGui Draw Data]
    F --> G[Swap Window Buffers]
    G --> H[Poll Input Events]
    H --> A
```

## 6. 빌드 시스템

```mermaid
graph LR
    subgraph "Build Configuration"
        Premake[premake5.lua]
    end
    
    subgraph "Generated Files"
        Sln[Hazel.sln]
        HazelProj[Hazel.vcxproj]
        SandboxProj[Sandbox.vcxproj]
    end
    
    subgraph "Compilation"
        HazelBuild[Hazel Build]
        SandboxBuild[Sandbox Build]
    end
    
    subgraph "Output"
        HazelDll[Hazel.dll]
        SandboxExe[Sandbox.exe]
    end
    
    Premake -->|Generates| Sln
    Premake -->|Generates| HazelProj
    Premake -->|Generates| SandboxProj
    HazelProj -->|Compile| HazelBuild
    SandboxProj -->|Compile| SandboxBuild
    HazelBuild -->|Output| HazelDll
    SandboxBuild -->|Output| SandboxExe
    SandboxBuild -->|Links| HazelDll
```

## 7. 의존성 그래프

```mermaid
graph TD
    Sandbox[Sandbox Application]
    Hazel[Hazel Engine Library]
    
    subgraph "Rendering"
        GLFW[GLFW]
        Glad[Glad]
        OpenGL[OpenGL]
    end
    
    subgraph "UI"
        ImGui[ImGui]
    end
    
    subgraph "Utilities"
        glm[glm - Math]
        spdlog[spdlog - Logging]
    end
    
    Sandbox -->|links| Hazel
    Hazel -->|uses| GLFW
    Hazel -->|uses| Glad
    Hazel -->|uses| ImGui
    Hazel -->|uses| glm
    Hazel -->|uses| spdlog
    
    GLFW -->|binds| OpenGL
    Glad -->|loads| OpenGL
```

## 8. 클래스 다이어그램 - Core

```mermaid
classDiagram
    class Application {
        -m_Window: unique_ptr~Window~
        -m_Running: bool
        -m_LayerStack: LayerStack
        -m_ImGuiLayer: ImGuiLayer*
        +Run() void
        +OnEvent(Event&) void
        +PushLayer(Layer*) void
        +PushOverlay(Layer*) void
        +GetWindow() Window&
        +GetInstance() Application&
    }
    
    class Window {
        <<interface>>
        +OnUpdate() void
        +GetWidth() unsigned int
        +GetHeight() unsigned int
        +SetEventCallback(EventCallbackFn) void
        +SetVSync(bool) void
        +IsVSync() bool
        +GetNativeWindow() void*
        +Create() Window*
    }
    
    class WindowsWindow {
        -m_Window: GLFWwindow*
        -m_Data: WindowData
        -m_Context: GraphicsContext*
        +OnUpdate() void
        +GetWidth() unsigned int
        +GetHeight() unsigned int
        +SetEventCallback(EventCallbackFn) void
        +SetVSync(bool) void
        -Init(WindowProps&) void
        -Shutdown() void
    }

    class GraphicsContext {
        <<interface>>
        +Init() void
        +SwapBuffers() void
    }

    class OpenGLContext {
        -m_WindowHandle: GLFWwindow*
        +Init() void
        +SwapBuffers() void
    }
    
    class Layer {
        <<abstract>>
        #m_DebugName: string
        +OnAttach() void
        +OnDetach() void
        +OnUpdate() void
        +OnImGuiRender() void
        +OnEvent(Event&) void
    }
    
    class LayerStack {
        -m_Layers: vector~Layer*~
        -m_LayerInsertIndex: unsigned int
        +PushLayer(Layer*) void
        +PushOverlay(Layer*) void
        +PopLayer(Layer*) void
        +PopOverlay(Layer*) void
        +begin() iterator
        +end() iterator
    }
    
    class ImGuiLayer {
        +OnAttach() void
        +OnDetach() void
        +OnImGuiRender() void
        +Begin() void
        +End() void
    }
    
    Application --> Window : owns
    Application --> LayerStack : manages
    Window <|.. WindowsWindow : implements
    WindowsWindow --> GraphicsContext : uses
    GraphicsContext <|.. OpenGLContext : implements
    Layer <|-- ImGuiLayer : extends
    LayerStack --> Layer : contains
```

## 9. 클래스 다이어그램 - Event System

```mermaid
classDiagram
    class Event {
        <<abstract>>
        #m_Handled: bool
        +GetEventType() EventType
        +GetName() string
        +GetCategoryFlags() int
        +IsInCategory(EventCategory) bool
        +ToString() string
    }
    
    class WindowCloseEvent {
        +GetEventType() EventType
        +GetName() string
    }
    
    class WindowResizeEvent {
        -m_Width: unsigned int
        -m_Height: unsigned int
        +GetWidth() unsigned int
        +GetHeight() unsigned int
    }
    
    class KeyEvent {
        <<abstract>>
        #m_KeyCode: int
        +GetKeyCode() int
    }
    
    class KeyPressedEvent {
        -m_RepeatCount: int
        +GetRepeatCount() int
    }
    
    class KeyReleasedEvent {
    }
    
    class MouseMovedEvent {
        -m_MouseX: float
        -m_MouseY: float
        +GetX() float
        +GetY() float
    }
    
    class MouseButtonEvent {
        <<abstract>>
        #m_Button: int
        +GetMouseButton() int
    }
    
    class MouseButtonPressedEvent {
    }
    
    class MouseButtonReleasedEvent {
    }
    
    Event <|-- WindowCloseEvent
    Event <|-- WindowResizeEvent
    Event <|-- KeyEvent
    Event <|-- MouseMovedEvent
    Event <|-- MouseButtonEvent
    KeyEvent <|-- KeyPressedEvent
    KeyEvent <|-- KeyReleasedEvent
    MouseButtonEvent <|-- MouseButtonPressedEvent
    MouseButtonEvent <|-- MouseButtonReleasedEvent
```

## 10. 디렉토리 구조

```mermaid
graph TB
    Root[Hazel/]
    
    HazelDir[Hazel/]
    SandboxDir[Sandbox/]
    VendorDir[vendor/]
    BinDir[bin/]
    BinIntDir[bin-int/]
    
    HazelSrc[src/]
    HazelVendor[vendor/]
    
    HazelCore[Hazel/]
    Platform[Platform/]
    
    Windows[Windows/]
    Events[Events/]
    ImGuiDir[ImGui/]
    
    Root --> HazelDir
    Root --> SandboxDir
    Root --> VendorDir
    Root --> BinDir
    Root --> BinIntDir
    
    HazelDir --> HazelSrc
    HazelDir --> HazelVendor
    
    HazelSrc --> HazelCore
    HazelSrc --> Platform
    
    HazelCore --> Events
    HazelCore --> ImGuiDir
    Platform --> Windows
    
    style Root fill:#f9f,stroke:#333,stroke-width:4px
    style HazelDir fill:#bbf,stroke:#333,stroke-width:2px
    style SandboxDir fill:#bfb,stroke:#333,stroke-width:2px
```

## 주요 컴포넌트 상세 설명

### Application (엔진 핵심)
- 엔진의 메인 클래스로 게임 루프를 관리합니다
- 싱글톤 패턴으로 전역 액세스를 제공합니다
- Window, LayerStack, Event System을 통합 관리합니다

### Layer System (계층 시스템)
- 게임 로직을 논리적 계층으로 분리합니다
- 업데이트와 렌더링을 계층별로 관리합니다
- Overlay는 일반 Layer보다 항상 위에 렌더링됩니다

### Event System (이벤트 시스템)
- 이벤트 기반 아키텍처를 구현합니다
- 다양한 입력 및 시스템 이벤트를 처리합니다
- 이벤트는 LayerStack을 통해 역순으로 전파됩니다

### Platform Layer (플랫폼 레이어)
- 플랫폼별 구현을 추상화합니다
- 현재는 Windows + macOS(dev scope) GLFW 구현을 제공합니다
- 다른 플랫폼 지원 시 쉽게 확장 가능합니다

### Build System (빌드 시스템)
- Premake5를 사용하여 빌드 설정을 관리합니다
- Debug, Release, Dist 세 가지 빌드 구성을 지원합니다
- Hazel은 DLL로, Sandbox는 EXE로 빌드됩니다
