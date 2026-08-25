#pragma once

// Core
#include "Core/FrostMath.h"
#include "Core/Logger.h"
#include "Core/Assert.h"
#include "Core/EventBus.h"
#include "Core/Events.h"
#include "Core/Application.h"
#include "Core/ThreadPool.h"

// Platform
#include "Platform/Window/IWindow.h"
#include "Platform/Input/Input.h"

// ECS
#include "ECS/Registry.h"
#include "ECS/View.h"
#include "ECS/ScriptBase.h"
#include "ECS/ScriptComponent.h"
#include "ECS/ScriptSystem.h"

// Rendering
#include "Rendering/Shader.h"
#include "Rendering/ShaderLoader.h"
#include "Rendering/Texture.h"
#include "Rendering/TextureLoader.h"
#include "Rendering/BatchRenderer.h"
#include "Rendering/Camera2D.h"
#include "Rendering/DebugRenderer.h"
#include "Rendering/TextRenderer.h"

// Assets
#include "Assets/AssetLoader.h"

// Scene
#include "Scene/Scene.h"
#include "Scene/SceneManager.h"