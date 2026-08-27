#pragma once
#include "Scene.h"
#include "Core/Logger.h"
#include "Core/Assert.h"
#include <memory>
#include <stack>
#include <functional>

namespace Frost
{
	class SceneManager
	{
	public:
		static SceneManager& Get()
		{
			static SceneManager instance;
			return instance;
		}

		SceneManager(const SceneManager&) = delete;
		SceneManager& operator=(const SceneManager&) = delete;

		template <typename T, typename... Args>
		void Load(Args&&... args)
		{
			m_PendingLoad = [args = std::make_tuple(
				std::forward<Args>(args)...)]() mutable
				{
					return std::apply(
						[](auto&&... a) {
							return std::make_unique<T>(
								std::forward<decltype(a)>(a)...);
						},
						std::move(args)
					);
				};
		}

		template <typename T, typename... Args>
		void Push(Args&&... args)
		{
			m_PendingPush = [args = std::make_tuple(
				std::forward<Args>(args)...)]() mutable
				{
					return std::apply(
						[](auto&&... a) {
							return std::make_unique<T>(
								std::forward<decltype(a)>(a)...);
						},
						std::move(args)
					);
				};
		}


		void Pop()
		{
			m_PendingPop = true;
		}


		void Update(float dt)
		{
			ProcessPending();
			if (!m_Scenes.empty())
				m_Scenes.top()->Update(dt);
		}

		void Render()
		{
			if (!m_Scenes.empty())
				m_Scenes.top()->Render();
		}

		bool   IsEmpty() const { return m_Scenes.empty(); }
		size_t Size()    const { return m_Scenes.size(); }

		Scene* Current()
		{
			if (m_Scenes.empty()) return nullptr;
			return m_Scenes.top().get();
		}

	private:
		SceneManager() = default;
		~SceneManager() = default;

		void ProcessPending()
		{

			if (m_PendingLoad)
			{
				while (!m_Scenes.empty())
				{
					FROST_LOG("Unloading scene: %s",
						m_Scenes.top()->GetName().c_str());
					m_Scenes.top()->Shutdown();
					m_Scenes.pop();
				}

				auto scene = m_PendingLoad();
				FROST_LOG("Loading scene: %s", scene->GetName().c_str());
				scene->Init();
				m_Scenes.push(std::move(scene));
				m_PendingLoad = nullptr;
			}


			if (m_PendingPush)
			{
				auto scene = m_PendingPush();
				FROST_LOG("Pushing scene: %s", scene->GetName().c_str());
				scene->Init();
				m_Scenes.push(std::move(scene));
				m_PendingPush = nullptr;
			}


			if (m_PendingPop)
			{
				if (!m_Scenes.empty())
				{
					FROST_LOG("Popping scene: %s",
						m_Scenes.top()->GetName().c_str());
					m_Scenes.top()->Shutdown();
					m_Scenes.pop();
				}
				m_PendingPop = false;
			}
		}

		std::stack<std::unique_ptr<Scene>> m_Scenes;

		std::function<std::unique_ptr<Scene>()> m_PendingLoad;
		std::function<std::unique_ptr<Scene>()> m_PendingPush;
		bool m_PendingPop = false;
	};
}