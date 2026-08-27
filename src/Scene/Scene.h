#pragma once
#include <string>
#include <memory>
#include "ECS/Registry.h"
#include "ECS/ScriptSystem.h"
#include "Core/EventBus.h"
#include "Assets/AssetLoader.h"
#include "Platform/Input/Input.h"

namespace Frost
{
    class Scene
    {
    public:
        Scene(const std::string& name) : m_Name(name) {}
        virtual ~Scene() = default;

        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;

        
        virtual void OnInit() {}
        virtual void OnUpdate(float dt) {}
        virtual void OnRender() {}
        virtual void OnShutdown() {}

        
        void Init()
        {
            //Frost::Input::Update();//clear input from old scene
            m_Registry = std::make_unique<Registry>();
            m_ScriptSystem = std::make_unique<ScriptSystem>(*m_Registry);


            OnInit();
        }

        void Update(float dt)
        {
            m_ScriptSystem->Update(dt);
            OnUpdate(dt);
            m_Registry->FlushDestroyQueue();
        }

        void Render()
        {
            OnRender();
        }

        void Shutdown()
        {
            OnShutdown();
            EventBus::ClearAll();
            m_Registry->Reset();
        }

        void ResetRegistry()
        {
            m_Registry->Reset();
        }

        const std::string& GetName()     const { return m_Name; }
        Registry& GetRegistry() { return *m_Registry; }

    protected:
        // helpers 4 entity creation
        Entity CreateEntity()
        {
            return m_Registry->Create();
        }

        template <typename T>
        void AddComponent(Entity e, T component = T{})
        {
            m_Registry->Add<T>(e, std::move(component));
        }

        template <typename T>
        T& GetComponent(Entity e)
        {
            return m_Registry->Get<T>(e);
        }

        template <typename T>
        bool HasComponent(Entity e) const
        {
            return m_Registry->Has<T>(e);
        }

        template <typename T>
        void RemoveComponent(Entity e)
        {
            m_Registry->Remove<T>(e);
        }

        void DestroyEntity(Entity e)
        {
            m_Registry->DestroyDeferred(e);
        }

        template <typename... Components>
        View<Components...> GetView()
        {
            return m_Registry->GetView<Components...>();
        }

        
        template <typename T, typename... Args>
        T* AttachScript(Entity e, Args&&... args)
        {
            auto script = std::make_unique<T>(std::forward<Args>(args)...);
            T* rawPtr = script.get(); // keep raw pointer to return
            script->m_Entity = e;
            script->m_Registry = m_Registry.get();

            // If entity already has ScriptComponent, add to its list
            if (m_Registry->Has<ScriptComponent>(e))
            {
                auto& sc = m_Registry->Get<ScriptComponent>(e);
                sc.scripts.push_back({ std::move(script), false });
            }
            else
            {
                ScriptComponent sc;
                sc.scripts.push_back({ std::move(script), false });
                m_Registry->Add<ScriptComponent>(e, std::move(sc));
            }

            return rawPtr;
        }

        std::unique_ptr<Registry>     m_Registry;
        std::unique_ptr<ScriptSystem> m_ScriptSystem;
        std::string                   m_Name;
    };
}