#include "DebugRenderer.h"
#include <cmath>
#include "Core/FrostMath.h"

namespace Frost
{
    DebugRenderer::~DebugRenderer()
    {
        if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
        if (m_VBO) glDeleteBuffers(1, &m_VBO);
    }

    bool DebugRenderer::Init(const std::string& vertPath,
        const std::string& fragPath)
    {
        m_Shader = ShaderLoader::Create(vertPath, fragPath);
        if (!m_Shader)
        {
            FROST_ERROR("DebugRenderer failed to load shaders");
            return false;
        }

        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

        
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(float) * 2 * SEGMENTS, nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
            2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);

        m_Initialized = true;
        FROST_LOG("DebugRenderer initialized");
        return true;
    }

    void DebugRenderer::DrawCircle(vec2 center, float radius, vec4 color)
    {
        m_Circles.push_back({ center, radius, color });
    }

    void DebugRenderer::Flush(const mat4& viewProjection)
    {
        if (m_Circles.empty() || !m_Initialized) return;

        // Save state
        GLboolean depthTestWasEnabled;
        glGetBooleanv(GL_DEPTH_TEST, &depthTestWasEnabled);

        m_Shader->Bind();
        m_Shader->SetMat4("uProjection", viewProjection);

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);

        for (auto& circle : m_Circles)
        {
            float verts[SEGMENTS * 2];
            for (int i = 0; i < SEGMENTS; i++)
            {
                float angle = (float)i / (float)SEGMENTS * Frost::TWO_PI;
                verts[i * 2 + 0] = circle.center.x + std::cos(angle) * circle.radius;
                verts[i * 2 + 1] = circle.center.y + std::sin(angle) * circle.radius;
            }

            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
            glUniform4f(
                glGetUniformLocation(m_Shader->ID(), "uColor"),
                circle.color.r, circle.color.g,
                circle.color.b, circle.color.a
            );
            glDrawArrays(GL_LINE_LOOP, 0, SEGMENTS);
        }

        // Restore state exactly as we found it
        if (depthTestWasEnabled) glEnable(GL_DEPTH_TEST);
        else                     glDisable(GL_DEPTH_TEST);

        glUseProgram(0);
        glBindVertexArray(0);

        m_Circles.clear();
    }
}