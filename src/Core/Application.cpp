#include "Application.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Frost
{
    Application* Application::s_Instance = nullptr;

    Application::Application(const AppConfig& config)
        : m_Config(config)
    {
        FROST_ASSERT_ALWAYS(!s_Instance, "Application already exists");
        s_Instance = this;
        Init();
    }

    void Application::Init()
    {
        // logger first
        FROST_INIT_LOG(true, true);
        FROST_LOG("FrostEngine v2 initializing");

        // window
        m_Window = IWindow::Create({
            m_Config.title,
            m_Config.width,
            m_Config.height,
            m_Config.vsync
            });

        // GLAD
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            FROST_ERROR("Failed to initialize GLAD");
            return;
        }

        glViewport(0, 0, m_Config.width, m_Config.height);

        // input must be after window so EventBus callbacks work
        Input::Init();

        // renderer
        m_Renderer = std::make_unique<BatchRenderer>();

        m_ThreadPool = std::make_unique<Frost::ThreadPool>();
        FROST_LOG("ThreadPool ready — %zu threads", m_ThreadPool->ThreadCount());

        // camera
        m_Camera = std::make_unique<Camera2D>(
            m_Config.width, m_Config.height);

        // white texture for particles and colored quads
        m_WhiteTex = TextureLoader::CreateWhite();

        // window close stop the loop
        EventBus::On<WindowClosedEvent>([this](const WindowClosedEvent&)
            {
                m_Running = false;
            });

        // window resize update camera
        EventBus::On<WindowResizedEvent>([this](const WindowResizedEvent& e)
            {
                m_Config.width = e.width;
                m_Config.height = e.height;
                m_Camera->SetSize(e.width, e.height);
            });

        FROST_LOG("FrostEngine v2 initialized (%dx%d)",
            m_Config.width, m_Config.height);
    }

    void Application::Run()
    {
        OnInit();

        double lastTime = glfwGetTime();

        while (m_Running && !m_Window->ShouldClose())
        {
            // delta time
            double currentTime = glfwGetTime();
            float  dt = (float)(currentTime - lastTime);
            lastTime = currentTime;

            // clamp
            if (dt > 0.05f) dt = 0.05f;

            // frame start
            Input::Update();
            m_Window->Update();

            // update
            SceneManager::Get().Update(dt);

            // render
            glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            SceneManager::Get().Render();

            m_Window->SwapBuffers();
        }

        Shutdown();
    }

    void Application::Shutdown()
    {
        FROST_LOG("FrostEngine shutting down");
        EventBus::ClearAll();
    }
}