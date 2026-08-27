#pragma once
#include <memory>
#include <string>
#include "Platform/Window/IWindow.h"
#include "Rendering/BatchRenderer.h"
#include "Rendering/Camera2D.h"
#include "Rendering/Texture.h"
#include "Scene/SceneManager.h"
#include "Platform/Input/Input.h"
#include "Core/EventBus.h"
#include "Core/Events.h"
#include "Core/Logger.h"
#include "Core/Threadpool.h"

namespace Frost
{
    struct AppConfig
    {
        std::string title = "FrostEngine";
        int         width = 1280;
        int         height = 720;
        bool        vsync = true;
    };

    class Application
    {
    public:
        Application(const AppConfig& config = AppConfig{});
        virtual ~Application() = default;

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;
        
        virtual void OnInit() = 0;
        
        void Run();
        
        IWindow& GetWindow() { return *m_Window; }
        BatchRenderer& GetRenderer() { return *m_Renderer; }
        Camera2D& GetCamera() { return *m_Camera; }
        Texture* GetWhiteTex() { return m_WhiteTex.get(); }

        int Width()  const { return m_Config.width; }
        int Height() const { return m_Config.height; }

        static Application& Get()
        {
            FROST_ASSERT_ALWAYS(s_Instance, "No application instance");
            return *s_Instance;
        }

        Frost::ThreadPool& GetThreadPool() { return *m_ThreadPool; }

    protected:
        AppConfig m_Config;

    private:
        void Init();
        void Shutdown();

        std::unique_ptr<IWindow>       m_Window;
        std::unique_ptr<BatchRenderer> m_Renderer;
        std::unique_ptr<Camera2D>      m_Camera;
        std::shared_ptr<Texture>       m_WhiteTex;
        std::unique_ptr<Frost::ThreadPool> m_ThreadPool;

        bool m_Running = true;

        static Application* s_Instance;
    };
}