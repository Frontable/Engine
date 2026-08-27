#include "BatchRenderer.h"
#include "Core/Logger.h"
#include "Core/Assert.h"
#include <cmath>

namespace Frost
{
    BatchRenderer::BatchRenderer()
    {
        Init();
    }

    BatchRenderer::~BatchRenderer()
    {
        if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
        if (m_VBO) glDeleteBuffers(1, &m_VBO);
        if (m_EBO) glDeleteBuffers(1, &m_EBO);
    }

    void BatchRenderer::Init()
    {
        // pre-build index buffer same pattern repeats for each quad
        unsigned int pattern[] = { 0, 1, 2, 2, 3, 0 };
        unsigned int indices[MAX_INDICES]{};
        int offset = 0;

        for (int i = 0; i < MAX_INDICES; i += INDICES_PER_QUAD)
        {
            for (int j = 0; j < INDICES_PER_QUAD; j++)
                indices[i + j] = pattern[j] + offset;
            offset += VERTS_PER_QUAD;
        }

        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);

        glGenBuffers(1, &m_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(Vertex) * MAX_VERTICES, nullptr, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &m_EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            sizeof(unsigned int) * MAX_INDICES, indices, GL_STATIC_DRAW);

        // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), (void*)offsetof(Vertex, position));
        glEnableVertexAttribArray(0);

        // UV
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), (void*)offsetof(Vertex, uv));
        glEnableVertexAttribArray(1);

        // color
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE,
            sizeof(Vertex), (void*)offsetof(Vertex, color));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
        FROST_LOG("BatchRenderer initialized — max %d sprites", MAX_SPRITES);
    }

    void BatchRenderer::Begin(Shader* shader, const mat4& viewProjection)
    {
        FROST_ASSERT(shader, "Shader is null");
        m_Shader = shader;
        m_ViewProj = viewProjection;
        m_DrawCalls = 0;

        m_Shader->Bind();
        m_Shader->SetMat4("uProjection", m_ViewProj);
    }

    void BatchRenderer::DrawSprite(const vec3& position,
        const vec2& size,
        float             rotation,
        const SpriteInfo& sprite,
        const Texture* texture,
        float             atlasW,
        float             atlasH,
        const vec4& color)
    {
        FROST_ASSERT(texture, "Texture is null");

        // flush if texture changes or batch is full
        if (texture->ID() != m_CurrentTexID && m_IndicesCount > 0)
            Flush();
        if (m_VertexCount + VERTS_PER_QUAD > MAX_VERTICES)
            Flush();

        m_CurrentTexID = texture->ID();

        // UV — top-left origin flipped for OpenGL
        vec2 uvMin = {
            sprite.uv.x / atlasW,
            1.0f - (sprite.uv.y + sprite.size.y) / atlasH
        };
        vec2 uvMax = {
            (sprite.uv.x + sprite.size.x) / atlasW,
            1.0f - sprite.uv.y / atlasH
        };

        vec2 uvs[4] = {
            { uvMin.x, uvMin.y },
            { uvMax.x, uvMin.y },
            { uvMax.x, uvMax.y },
            { uvMin.x, uvMax.y }
        };

        // local quad corners
        vec2 half = size * 0.5f;
        vec2 corners[4] = {
            { -half.x,  half.y },
            {  half.x,  half.y },
            {  half.x, -half.y },
            { -half.x, -half.y }
        };

        // rotate and translate
        float c = std::cos(rotation);
        float s = std::sin(rotation);

        for (int i = 0; i < VERTS_PER_QUAD; i++)
        {
            float rx = corners[i].x * c - corners[i].y * s;
            float ry = corners[i].x * s + corners[i].y * c;

            m_Vertices[m_VertexCount++] = {
                { position.x + rx, position.y + ry, position.z },
                uvs[i],
                color
            };
        }

        m_IndicesCount += INDICES_PER_QUAD;
    }

    void BatchRenderer::End()
    {
        Flush();
    }

    void BatchRenderer::Flush()
    {
        if (m_IndicesCount == 0) return;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_CurrentTexID);
        m_Shader->SetInt("uTexture", 0);

        UploadVertices();

        glBindVertexArray(m_VAO);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glDrawElements(GL_TRIANGLES, m_IndicesCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        m_DrawCalls++;
        m_VertexCount = 0;
        m_IndicesCount = 0;
        m_CurrentTexID = 0;
    }

    void BatchRenderer::UploadVertices()
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            sizeof(Vertex) * m_VertexCount,
            m_Vertices.data());
    }
}