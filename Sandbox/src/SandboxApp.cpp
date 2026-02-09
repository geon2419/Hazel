#include <Hazel.h>
#include <Hazel/Core.h>

class ExampleLayer : public Hazel::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_CameraPosition(0.0f, 0.0f, 0.0f)
	{
		m_VertexArray.reset(Hazel::VertexArray::Create());

		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f, // Pinkish
			 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f, // Bluish
			 0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f  // Yellowish
		};

		std::shared_ptr<Hazel::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(Hazel::VertexBuffer::Create(vertices, sizeof(vertices)));

		vertexBuffer->SetLayout({
			{ Hazel::ShaderDataType::Float3, "a_Position" },
			{ Hazel::ShaderDataType::Float4, "a_Color"  }
			});
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		unsigned int indices[3] = { 0, 1, 2 };
		const uint32_t indexCount = sizeof(indices) / sizeof(uint32_t);

		std::shared_ptr<Hazel::IndexBuffer> indexBuffer;
		indexBuffer.reset(Hazel::IndexBuffer::Create(indices, indexCount));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		m_SquareVA.reset(Hazel::VertexArray::Create());

		float squareVertices[3 * 4] = {
			-0.75f, -0.75f, 0.0f,
			 0.75f, -0.75f, 0.0f,
			 0.75f,  0.75f, 0.0f,
			-0.75f,  0.75f, 0.0f,
		};

		std::shared_ptr<Hazel::VertexBuffer> squareVB;
		squareVB.reset(Hazel::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squareVB->SetLayout({
			{ Hazel::ShaderDataType::Float3, "a_Position" },
			});
		m_SquareVA->AddVertexBuffer(squareVB);

		unsigned int squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		const uint32_t squareIndexCount = sizeof(squareIndices) / sizeof(uint32_t);
		std::shared_ptr<Hazel::IndexBuffer> squareIB;
		squareIB.reset(Hazel::IndexBuffer::Create(squareIndices, squareIndexCount));
		m_SquareVA->SetIndexBuffer(squareIB);

		std::string vertexSource = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;

			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
			}
		)";

		std::string fragmentSource = R"(
			#version 330 core

			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec4 v_Color;
			
			void main()
			{
				color = v_Color;
			}
		)";

		m_Shader.reset(new Hazel::Shader(vertexSource, fragmentSource));

		std::string blueShaderVertexSource = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
			}
		)";

		std::string blueShaderFragmentSource = R"(
			#version 330 core

			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			
			void main()
			{
				color = vec4(0.2, 0.3, 0.8, 1.0);
			}
		)";

		m_BlueShader.reset(new Hazel::Shader(blueShaderVertexSource, blueShaderFragmentSource));
	}

	void OnUpdate() override
	{
		if (Hazel::Input::IsKeyPressed(HZ_KEY_LEFT))
		{
			m_CameraPosition.x -= m_CameraMoveSpeed;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_RIGHT))
		{
			m_CameraPosition.x += m_CameraMoveSpeed;
		}

		if (Hazel::Input::IsKeyPressed(HZ_KEY_UP))
		{
			m_CameraPosition.y += m_CameraMoveSpeed;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_DOWN))
		{
			m_CameraPosition.y -= m_CameraMoveSpeed;
		}

		if (Hazel::Input::IsKeyPressed(HZ_KEY_A))
		{
			m_CameraRotation += m_CameraRotationSpeed * 10.0f;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_D))
		{
			m_CameraRotation -= m_CameraRotationSpeed * 10.0f;
		}

		Hazel::RenderCommand::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
		Hazel::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		Hazel::Renderer::BeginScene(m_Camera);
		Hazel::Renderer::Submit(m_BlueShader, m_SquareVA);
		Hazel::Renderer::Submit(m_Shader, m_VertexArray);
		Hazel::Renderer::EndScene();
	}

	void OnImGuiRender() override
	{
	}

	void OnEvent(Hazel::Event& event) override
	{
	}

private:
	std::shared_ptr<Hazel::Shader> m_Shader;
	std::shared_ptr<Hazel::VertexArray> m_VertexArray;

	std::shared_ptr<Hazel::Shader> m_BlueShader;
	std::shared_ptr<Hazel::VertexArray> m_SquareVA;

	Hazel::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
	float m_CameraMoveSpeed = 0.1f;

	float m_CameraRotation = 0.0f;
	float m_CameraRotationSpeed = 1.0f;
};

class Sandbox : public Hazel::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox()
	{
	}
};

Hazel::Application* Hazel::CreateApplication()
{
	return new Sandbox();
}