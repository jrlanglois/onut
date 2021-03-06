// Onut includes
#include <onut/ActionManager.h>
#if !defined(__unix__)
#include <onut/AudioEngine.h>
#endif // __unix__
//#include <onut/Cloud.h>
#include <onut/ComponentFactory.h>
#include <onut/ContentManager.h>
#include <onut/Dispatcher.h>
#include <onut/SceneManager.h>
#include <onut/Font.h>
#include <onut/GamePad.h>
#include <onut/Input.h>
#include <onut/onut.h>
#include <onut/ParticleSystemManager.h>
#include <onut/PrimitiveBatch.h>
#include <onut/Random.h>
#include <onut/Renderer.h>
#include <onut/Settings.h>
#include <onut/SpriteBatch.h>
#include <onut/Texture.h>
#include <onut/ThreadPool.h>
#include <onut/Timing.h>
#include <onut/UIContext.h>
#include <onut/UIControl.h>
#include <onut/UIPanel.h>
#include <onut/UITextBox.h>
#include <onut/Updater.h>
#include <onut/Window.h>

// Private
#if !defined(__unix__)
#include "JSBindings.h"
#endif // __unix__

// STL
#include <cassert>
#include <mutex>
#include <sstream>

#if !defined(__unix__)
OTextureRef g_pMainRenderTarget;
#endif // __unix__

#if defined(__unix__)
std::atomic<bool> g_bIsRunning;
#endif

namespace onut
{
    void createUI()
    {
        oUIContext = UIContext::create(Vector2(OScreenWf, OScreenHf));
        oUI = UIControl::create();
        oUI->widthType = UIControl::DimType::Relative;
        oUI->heightType = UIControl::DimType::Relative;

        oUIContext->addTextCaretSolver<onut::UITextBox>("", [=](const OUITextBoxRef& pTextBox, const Vector2& localPos) -> decltype(std::string().size())
        {
            auto pFont = OGetFont(pTextBox->textComponent.font.typeFace.c_str());
            if (!pFont) return 0;
            auto& text = pTextBox->textComponent.text;
            return pFont->caretPos(text, localPos.x - 4);
        });

        oWindow->onWrite = [](char c)
        {
            oUIContext->write(c);
        };
        oWindow->onKey = [](uintptr_t key)
        {
            oUIContext->keyDown(key);
        };

        oUIContext->addStyle<onut::UIPanel>("blur", [](const OUIPanelRef& pPanel, const Rect& rect)
        {
            oSpriteBatch->end();
            if (oRenderer->renderStates.renderTarget.get())
            {
                oRenderer->renderStates.renderTarget.get()->blur();
            }
            oSpriteBatch->begin();
            oSpriteBatch->drawRect(nullptr, (rect), pPanel->color);
        });
    }

    void createServices()
    {
        // Random
        randomizeSeed();

        // Thread pool
        oThreadPool = OThreadPool::create();

        // Dispatcher
        oDispatcher = ODispatcher::create();

        // Timing class
        oTiming = OTiming::create();

        // Updater
        oUpdater = OUpdater::create();

        // Window
        oWindow = OWindow::create();

        // Renderer
        oRenderer = ORenderer::create(oWindow);
        oRenderer->init(oWindow);

        // SpriteBatch
        oSpriteBatch = SpriteBatch::create();
        oPrimitiveBatch = PrimitiveBatch::create();
        
        // Content
        oContentManager = ContentManager::create();

        // Cloud
        //oCloud = Cloud::create(oSettings->getAppId(), oSettings->getAppSecret());

        // Mouse/Keyboard
        oInput = OInput::create(oWindow);

#if !defined(__unix__)
        // Audio
        oAudioEngine = AudioEngine::create();
#endif // __unix__

        // Particles
        oParticleSystemManager = ParticleSystemManager::create();

        // UI Context
        createUI();

        // Component factory
        oComponentFactory = ComponentFactory::create();
        oComponentFactory->registerDefaultComponents();

        // Entity Manager
        oSceneManager = SceneManager::create();

        // Undo/Redo for editors
        oActionManager = ActionManager::create();

#if !defined(__unix__)
        g_pMainRenderTarget = OTexture::createScreenRenderTarget();
        
        // Initialize Javascript
        onut::js::init();
#endif // __unix__
    }

    void cleanup()
    {
#if !defined(__unix__)
        onut::js::shutdown();

        g_pMainRenderTarget = nullptr;
#endif // __unix__
        oActionManager = nullptr;
        oSceneManager = nullptr;
        oComponentFactory = nullptr;
        oDispatcher = nullptr;
        oUpdater = nullptr;
        oUI = nullptr;
        oUIContext = nullptr;
        oParticleSystemManager = nullptr;
#if !defined(__unix__)
        oAudioEngine = nullptr;
#endif // __unix__
        oInput = nullptr;
        //oCloud = nullptr;
        oContentManager = nullptr;
        oPrimitiveBatch = nullptr;
        oSpriteBatch = nullptr;
        oRenderer = nullptr;
        oWindow = nullptr;
        oSettings = nullptr;
        oThreadPool = nullptr;
        oTiming = nullptr;
    }

    // Start the engine
    void run(std::function<void()> initCallback,
             std::function<void()> updateCallback, 
             std::function<void()> renderCallback,
             std::function<void()> postRenderCallback)
    {
        // Make sure we run just once
        static bool alreadyRan = false;
        assert(!alreadyRan);
        alreadyRan = true;

        createServices();

        // Call the user defined init
        if (initCallback)
        {
            initCallback();
        }

        // Main loop
#if defined(WIN32)
        MSG msg = {0};
#elif defined(__unix__)
		g_bIsRunning = true;
#endif // WIN32
        while (true)
        {
#if defined(WIN32)
            if (oSettings->getIsEditorMode())
            {
                if (GetMessage(&msg, 0, 0, 0) >= 0)
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);

                    if (msg.message == WM_QUIT)
                    {
                        break;
                    }
                }
            }
            else
            {
                if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);

                    if (msg.message == WM_QUIT)
                    {
                        break;
                    }
                }
            }
#elif defined(__unix__)
			if (!g_bIsRunning) break;
#endif

            // Sync to main callbacks
            oDispatcher->processQueue();

#if !defined(__unix__)
            // Update
            oAudioEngine->update();
#endif // __unix__
            auto framesToUpdate = oTiming->update(oSettings->getIsFixedStep());
            while (framesToUpdate--)
            {
                oInput->update();
#if defined(WIN32)
                POINT cur;
                GetCursorPos(&cur);
                ScreenToClient(oWindow->getHandle(), &cur);
                oInput->mousePos.x = cur.x;
                oInput->mousePos.y = cur.y;
                oInput->mousePosf.x = static_cast<float>(cur.x);
                oInput->mousePosf.y = static_cast<float>(cur.y);
#endif // WIN32
                oUpdater->update();
                auto mousePosf = OGetMousePos();
                if (oUIContext->useNavigation)
                {
                    oUI->update(oUIContext, Vector2(mousePosf.x, mousePosf.y), OGamePadPressed(OGamePadA) || OInputJustPressed(OKeyEnter), false, false,
                                OGamePadJustPressed(OGamePadDPadLeft) || OGamePadJustPressed(OGamePadLeftThumbLeft) || OInputJustPressed(OKeyLeft),
                                OGamePadJustPressed(OGamePadDPadRight) || OGamePadJustPressed(OGamePadLeftThumbRight) || OInputJustPressed(OKeyRight),
                                OGamePadJustPressed(OGamePadDPadUp) || OGamePadJustPressed(OGamePadLeftThumbUp) || OInputJustPressed(OKeyUp),
                                OGamePadJustPressed(OGamePadDPadDown) || OGamePadJustPressed(OGamePadLeftThumbDown) || OInputJustPressed(OKeyDown),
                                0.f);
                }
                else
                {
                    oUI->update(oUIContext, Vector2(mousePosf.x, mousePosf.y),
                                OInputPressed(OMouse1), OInputPressed(OMouse2), OInputPressed(OMouse3),
                                false, false, false, false, 
                                OInputPressed(OKeyLeftControl), oInput->getStateValue(OMouseZ));
                }
                oParticleSystemManager->update();
                oSceneManager->update();
#if !defined(__unix__)
                onut::js::update(oTiming->getDeltaTime());
#endif // __unix__
                if (updateCallback)
                {
                    updateCallback();
                }
            }

            // Render
            oTiming->render();
#if !defined(__unix__)
            oRenderer->renderStates.renderTarget = g_pMainRenderTarget;
#endif // __unix__
            oRenderer->beginFrame();
#if !defined(__unix__)
            onut::js::render();
#endif // __unix__
            if (renderCallback)
            {
                renderCallback();
            }
            oSceneManager->render();
            oParticleSystemManager->render();
            oSpriteBatch->begin();
            oUI->render(oUIContext);
            oSpriteBatch->end();

#if !defined(__unix__)
            // Draw final render target
            oRenderer->renderStates.renderTarget = nullptr;
            auto& res = oRenderer->getResolution();
            oRenderer->renderStates.viewport = iRect{0, 0, res.x, res.y};
            oRenderer->renderStates.scissorEnabled = false;
            oRenderer->renderStates.scissor = oRenderer->renderStates.viewport.get();
            oSpriteBatch->begin();
            oSpriteBatch->changeBlendMode(OBlendOpaque);
            oSpriteBatch->changeFiltering(OFilterNearest);
            oSpriteBatch->drawRect(g_pMainRenderTarget, ORectFit(Rect{0, 0, OScreenf}, g_pMainRenderTarget->getSizef()));
            oSpriteBatch->end();
            oSpriteBatch->changeBlendMode(OBlendAlpha);
            oSpriteBatch->changeFiltering(OFilterLinear);
#endif // __unix__
            if (postRenderCallback)
            {
                postRenderCallback();
            }
            oRenderer->endFrame();
        }

        cleanup();
    }

    void quit()
    {
#if defined(WIN32)
        PostQuitMessage(0);
#elif defined(__unix__)
		g_bIsRunning = false;
#endif
    }
}

// Main
void initSettings();
void init();
void update();
void render();
void postRender();

#if defined(WIN32)
#include <Windows.h>

int CALLBACK WinMain(HINSTANCE appInstance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdCount)
{
    initSettings();
    onut::run(init, update, render, postRender);
    return 0;
}
#elif defined(__unix__)
int main(int argc, char** argv)
{
    initSettings();
    onut::run(init, update, render, postRender);
    return 0;
}
#endif
