// Onut
#include <onut/IndexBuffer.h>
#include <onut/Renderer.h>
#include <onut/Settings.h>
#include <onut/Shader.h>
#include <onut/Texture.h>
#include <onut/VertexBuffer.h>
#include <onut/Window.h>

// STL
#include <fstream>
#include <vector>

// Shaders
#include "_2dvs.cso.h"
#include "_2dps.cso.h"
#include "blurvs.cso.h"
#include "blurhps.cso.h"
#include "blurvps.cso.h"
#include "sepia.cso.h"
#include "crt.cso.h"
#include "cartoon.cso.h"
#include "vignette.cso.h"

ORendererRef oRenderer;

namespace onut
{
    RenderStates::RenderStates()
    {
        for (auto& pTexture : textures)
        {
            pTexture = nullptr;
        }
        blendMode = OBlendOpaque;
        sampleFiltering = OFilterLinear;
        sampleAddressMode = OTextureWrap;
        scissor = viewport = {0, 0, 100, 100};
        projection = Matrix::Identity;
        view = Matrix::Identity;
        world = Matrix::Identity;
        depthEnabled = false;
        depthWrite = false;
        backFaceCull = false;
        scissorEnabled = false;
        primitiveMode = OPrimitiveTriangleList;
        vertexShader = nullptr;
        pixelShader = nullptr;
        vertexBuffer = nullptr;
        indexBuffer = nullptr;
        renderTarget = nullptr;
        clearColor = Color::fromHexRGB(0x1d232d);
    }

    RenderStates::RenderStates(const RenderStates& other)
    {
        for (int i = 0; i < MAX_TEXTURES; ++i)
        {
            textures[i] = other.textures[i];
        }
        blendMode = other.blendMode;
        sampleFiltering = other.sampleFiltering;
        sampleAddressMode = other.sampleAddressMode;
        viewport = other.viewport;
        scissor = other.scissor;
        projection = other.projection;
        view = other.view;
        world = other.world;
        depthEnabled = other.depthEnabled;
        depthWrite = other.depthWrite;
        backFaceCull = other.backFaceCull;
        scissorEnabled = other.scissorEnabled;
        primitiveMode = other.primitiveMode;
        vertexShader = other.vertexShader;
        pixelShader = other.pixelShader;
        vertexBuffer = other.vertexBuffer;
        indexBuffer = other.indexBuffer;
        clearColor = other.clearColor;
    }

    RenderStates& RenderStates::operator=(const RenderStates& other)
    {
        for (int i = 0; i < MAX_TEXTURES; ++i)
        {
            textures[i] = other.textures[i];
        }
        blendMode = other.blendMode;
        sampleFiltering = other.sampleFiltering;
        sampleAddressMode = other.sampleAddressMode;
        viewport = other.viewport;
        scissor = other.scissor;
        projection = other.projection;
        view = other.view;
        world = other.world;
        depthEnabled = other.depthEnabled;
        depthWrite = other.depthWrite;
        backFaceCull = other.backFaceCull;
        scissorEnabled = other.scissorEnabled;
        primitiveMode = other.primitiveMode;
        vertexShader = other.vertexShader;
        pixelShader = other.pixelShader;
        vertexBuffer = other.vertexBuffer;
        indexBuffer = other.indexBuffer;
        clearColor = other.clearColor;

        return *this;
    }

    void RenderStates::reset()
    {
        for (auto& pTexture : textures)
        {
            pTexture.reset();
        }
        blendMode.reset();
        sampleFiltering.reset();
        sampleAddressMode.reset();
        viewport.reset();
        scissor.reset();
        projection.reset();
        view.reset();
        world.reset();
        depthEnabled.reset();
        depthWrite.reset();
        backFaceCull.reset();
        scissorEnabled.reset();
        primitiveMode.reset();
        vertexShader.reset();
        pixelShader.reset();
        vertexBuffer.reset();
        indexBuffer.reset();
        renderTarget.reset();
        clearColor.reset();
    }

    Renderer::Renderer()
    {
    }

    Renderer::~Renderer()
    {
    }

    void Renderer::init(const OWindowRef& pWindow)
    {
        loadShaders();
#if !defined(__unix__)
        const float vertices[] = {
            -1, -1,
            -1, 1,
            1, -1,
            1, -1,
            -1, 1,
            1, 1
        };
        m_pEffectsVertexBuffer = OVertexBuffer::createStatic(vertices, sizeof(vertices));
#endif
    }

    void Renderer::setupFor2D()
    {
        setupFor2D(Matrix::Identity);
    }

    void Renderer::setupFor2D(const Matrix& transform)
    {
        set2DCamera(Vector2::Zero);
        renderStates.world = transform;
        renderStates.vertexShader = m_p2DVertexShader;
        renderStates.pixelShader = m_p2DPixelShader;
    }

    Renderer::CameraMatrices Renderer::build2DCamera(const Vector2& position, float zoom)
    {
        CameraMatrices ret;
        ret.projection = Matrix::CreateOrthographicOffCenter(0, static_cast<float>(getResolution().x), static_cast<float>(getResolution().y), 0, -999, 999);
        ret.view = Matrix::CreateTranslation(-position) * Matrix::CreateScale(zoom);
        ret.view.Invert();
        return std::move(ret);
    }

    Renderer::CameraMatrices Renderer::build2DCameraOffCenter(const Vector2& position, float zoom)
    {
        CameraMatrices ret;
        ret.projection = Matrix::CreateOrthographicOffCenter(0, static_cast<float>(getResolution().x), static_cast<float>(getResolution().y), 0, -999, 999);
        ret.view = Matrix::CreateTranslation(-position) *
            Matrix::CreateScale(zoom) *
            Matrix::CreateTranslation({static_cast<float>(getResolution().x) * .5f, static_cast<float>(getResolution().y) * .5f, 0.f});
        ret.view.Invert();
        return std::move(ret);
    }

    Renderer::CameraMatrices Renderer::set2DCamera(const Vector2& position, float zoom)
    {
        auto ret = build2DCamera(position, zoom);
        set2DCamera(ret);
        return std::move(ret);
    }

    Renderer::CameraMatrices Renderer::set2DCameraOffCenter(const Vector2& position, float zoom)
    {
        auto ret = build2DCameraOffCenter(position, zoom);
        set2DCamera(ret);
        return std::move(ret);
    }

    void Renderer::set2DCamera(const CameraMatrices& camera, const Matrix& transform)
    {
        renderStates.projection = camera.projection;
        renderStates.view = camera.view;
        renderStates.world = transform;
    }

    void Renderer::loadShaders()
    {
#if !defined(__unix__)
        // Create 2D shaders
        {
            m_p2DVertexShader = OShader::createFromBinaryData(_2dvs_cso, sizeof(_2dvs_cso), OVertexShader, {{2, "POSITION"}, {2, "TEXCOORD"}, {4, "COLOR"}});
            m_p2DPixelShader = OShader::createFromBinaryData(_2dps_cso, sizeof(_2dps_cso), OPixelShader);
        }

        // Effects
        {
            m_pEffectsVertexShader = OShader::createFromBinaryData(blurvs_cso, sizeof(blurvs_cso), OVertexShader, {{2, "POSITION"}});
            m_pBlurHPixelShader = OShader::createFromBinaryData(blurhps_cso, sizeof(blurhps_cso), OPixelShader);
            m_pBlurVPixelShader = OShader::createFromBinaryData(blurvps_cso, sizeof(blurvps_cso), OPixelShader);
            m_pSepiaPixelShader = OShader::createFromBinaryData(sepia_cso, sizeof(sepia_cso), OPixelShader);
            m_pCRTPixelShader = OShader::createFromBinaryData(crt_cso, sizeof(crt_cso), OPixelShader);
            m_pCartoonPixelShader = OShader::createFromBinaryData(cartoon_cso, sizeof(cartoon_cso), OPixelShader);
            m_pVignettePixelShader = OShader::createFromBinaryData(vignette_cso, sizeof(vignette_cso), OPixelShader);
        }
#endif
    }

    void Renderer::setupEffectRenderStates()
    {
        renderStates.depthEnabled = false;
        renderStates.scissorEnabled = false;
        renderStates.blendMode = BlendMode::Opaque;
        renderStates.sampleFiltering = OFilterLinear;
        renderStates.sampleAddressMode = OTextureClamp;
        renderStates.vertexShader = m_pEffectsVertexShader;
        renderStates.vertexBuffer = m_pEffectsVertexBuffer;
    }

    void Renderer::drawBlurH()
    {
        setupEffectRenderStates();
        renderStates.pixelShader = m_pBlurHPixelShader;
        draw(6);
    }

    void Renderer::drawBlurV()
    {
        setupEffectRenderStates();
        renderStates.pixelShader = m_pBlurVPixelShader;
        draw(6);
    }

    void Renderer::drawSepia()
    {
        setupEffectRenderStates();
        renderStates.pixelShader = m_pSepiaPixelShader;
        draw(6);
    }

    void Renderer::drawCRT()
    {
        setupEffectRenderStates();
        renderStates.pixelShader = m_pCRTPixelShader;
        draw(6);
    }

    void Renderer::drawCartoon()
    {
        setupEffectRenderStates();
        renderStates.pixelShader = m_pCartoonPixelShader;
        draw(6);
    }

    void Renderer::drawVignette()
    {
        setupEffectRenderStates();
        renderStates.pixelShader = m_pVignettePixelShader;
        draw(6);
    }

    Point Renderer::getResolution() const
    {
        auto& pRenderTarget = renderStates.renderTarget.get();
        if (pRenderTarget)
        {
            return pRenderTarget->getSize();
        }
        return getTrueResolution();
    }
}
