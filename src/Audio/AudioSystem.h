#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "Core/Logger.h"
#include "Core/Assert.h"


// miniaudio imp needs to be defined in AudioSystem.cpp
#include "miniaudio.h"

namespace Frost
{
	class AudioSystem
	{
	public:
		static bool Init()
		{
			FROST_ASSERT_ALWAYS(!s_Initialized, "AudioSystem already initialized!");

			if (ma_engine_init(nullptr, &s_Engine) != MA_SUCCESS)
			{
				FROST_ERROR("Failed to init miniaudio");
				return false;
			}

			s_Initialized = true;
			FROST_LOG("AudioSystem initialized");
			return true;
		}

		static void ShutDown()
		{
			if (!s_Initialized) return;

			for (auto& [path, sound] : s_Sounds)
			{
				ma_sound_stop(sound.get());
				ma_sound_uninit(sound.get());
			}

			s_Sounds.clear();

			ma_engine_uninit(&s_Engine);
			s_Initialized = false;
			FROST_LOG("Audio shutdown");
		}

		static void Play(const std::string& path, bool loop = false)
		{
			if (!s_Initialized) return;

			if (s_Sounds.find(path) == s_Sounds.end())
			{
				if (!Load(path))
					return;
			}

			auto& sound = s_Sounds.at(path);

			ma_sound_seek_to_pcm_frame(sound.get(), 0);
			ma_sound_set_looping(sound.get(), loop ? MA_TRUE : MA_FALSE);
			ma_sound_start(sound.get());
		}

		static void Stop(const std::string& path)
		{
			if (!s_Initialized) return;

			auto it = s_Sounds.find(path);
			if (it != s_Sounds.end())
			{
				ma_sound_stop(it->second.get());
			}
		}

		static void StopAll()
		{
			if (!s_Initialized) return;
			for (auto& [path, sound] : s_Sounds)
			{
				ma_sound_stop(sound.get());
			}
		}

		//volume is from 0.0 to 1.0
		static void SetVolume(const std::string& path, float volume)
		{
			if (!s_Initialized) return;

			auto it = s_Sounds.find(path);

			if (it != s_Sounds.end())
			{
				ma_sound_set_volume(it->second.get(), volume);
			}
		}

		static void SetMasterVolume(float volume)
		{
			if (!s_Initialized) return;
			ma_engine_set_volume(&s_Engine, volume);
		}

		static bool IsPlaying(const std::string& path)
		{
			if (!s_Initialized) return false;
			
			auto it = s_Sounds.find(path);
			if (it == s_Sounds.end()) return false;

			return ma_sound_is_playing(it->second.get()) == MA_TRUE;
		}



		//used on scene load so i done explode from
		//hearing all sounds play at the same time
		static void Preload(const std::string& path)
		{
			if (!s_Initialized) return;

			if (s_Sounds.find(path) == s_Sounds.end())
				Load(path);
		}

		static void Unload(const std::string path)
		{
			if (!s_Initialized) return;

			auto it = s_Sounds.find(path);
			if (it != s_Sounds.end())
			{
				ma_sound_stop(it->second.get());
				ma_sound_uninit(it->second.get());
				s_Sounds.erase(it);
			}
		}

	private:

		static bool Load(const std::string& path)
		{
			auto sound = std::make_unique<ma_sound>();

			ma_result result = ma_sound_init_from_file(
				&s_Engine,
				path.c_str(),
				MA_SOUND_FLAG_DECODE,
				nullptr,
				nullptr,
				sound.get()
			);

			if (result != MA_SUCCESS)
			{
				FROST_ERROR("Failed to load sound %s", path.c_str());
				return false;
			}

			FROST_LOG("Loaded sound %s", path.c_str());
			s_Sounds.emplace(path, std::move(sound));
			return true;

		}

		static inline ma_engine s_Engine{};
		static inline std::unordered_map<std::string, std::unique_ptr<ma_sound>> s_Sounds;
		static inline bool s_Initialized;

	};
}