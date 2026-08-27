#pragma once
#include "Core/FrostMath.h"

namespace Frost
{
	class Camera2D
	{
	public:
		Camera2D() = default;
		Camera2D(int width, int height);

		void SetPosition(const vec2& pos) { m_Pos = pos;   m_Dirty = true; }
		void SetScale(float scale) { m_Scale = scale; m_Dirty = true; }
		void SetSize(int w, int h)
		{
			m_Width = w;
			m_Height = h;
			m_Projection = ortho(0.0f, (float)w, (float)h, 0.0f);
			m_Dirty = true;
		}

		vec2  GetPosition() const { return m_Pos; }
		float GetScale()    const { return m_Scale; }
		int   GetWidth()    const { return m_Width; }
		int   GetHeight()   const { return m_Height; }

		const mat4& GetViewProjection();


		void Follow(const vec2& target, float lerpFactor = 1.0f);

	private:
		void Recalculate();

		int   m_Width = 1280;
		int   m_Height = 720;
		float m_Scale = 1.0f;
		vec2  m_Pos = { 0.0f, 0.0f };

		mat4 m_ViewProjection{ 1.0f };
		mat4 m_Projection{ 1.0f };
		bool m_Dirty = true;
	};
}