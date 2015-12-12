#include <cassert>
#include <mutex>
#include <sstream>

#include "Audio.h"
#include "InputDevice.h"
#include "onut.h"
#include "Window.h"

using namespace DirectX;

// Our engine services
onut::Window*                       OWindow = nullptr;
onut::Renderer*                     ORenderer = nullptr;
onut::Settings*                     OSettings = new onut::Settings();
onut::SpriteBatch*                  OSpriteBatch = nullptr;
onut::PrimitiveBatch*               OPrimitiveBatch = nullptr;
onut::GamePad*                      g_gamePads[4] = {nullptr};
onut::EventManager*                 OEvent = nullptr;
onut::ContentManager<>*             OContentManager = nullptr;
AudioEngine*                        g_pAudioEngine = nullptr;
onut::TimeInfo<>                    g_timeInfo;
onut::Synchronous<onut::Pool<>>     g_mainSync;
onut::ParticleSystemManager<>*      OParticles = nullptr;
Vector2                             OMousePos;
onut::InputDevice*                  g_inputDevice = nullptr;
onut::Input*                        OInput = nullptr;


// So commonly used stuff
float                               ODT = 0.f;

namespace onut
{
    void createServices()
    {
        // Random
        randomizeSeed();

        // Events
        OEvent = new EventManager();

        // Window
        OWindow = new Window(OSettings->getResolution(), OSettings->getIsResizableWindow());

        // DirectX
        ORenderer = new Renderer(*OWindow);

        // SpriteBatch
        OSB = new SpriteBatch();
        OPB = new PrimitiveBatch();

        // Content
        OContentManager = new ContentManager<>();
        OContentManager->addDefaultSearchPaths();

        // Mouse/Keyboard
        g_inputDevice = new InputDevice(OWindow);
        OInput = new onut::Input(OINPUT_MOUSEZ + 1);

        // Gamepads
        for (int i = 0; i < 4; ++i)
        {
            g_gamePads[i] = new GamePad(i);
        }

        // Audio
#ifdef WIN32
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
#endif
        AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
        eflags = eflags | AudioEngine_Debug;
#endif
        try
        {
            g_pAudioEngine = new AudioEngine(eflags);
        }
        catch (std::exception e)
        {
        }

        // Particles
        OParticles = new ParticleSystemManager<>();

        // Register a bunch of default events
        OEvent->addEvent("NavigateLeft", []
        {
            return OJustPressed(OLeftBtn) || OJustPressed(OLLeftBtn);
        });
        OEvent->addEvent("NavigateRight", []
        {
            return OJustPressed(ORightBtn) || OJustPressed(OLRightBtn);
        });
        OEvent->addEvent("NavigateUp", []
        {
            return OJustPressed(OUpBtn) || OJustPressed(OLUpBtn);
        });
        OEvent->addEvent("NavigateDown", []
        {
            return OJustPressed(ODownBtn) || OJustPressed(OLDownBtn);
        });
        OEvent->addEvent("Accept", []
        {
            return OJustPressed(OABtn) || OJustPressed(OStartBtn);
        });
        OEvent->addEvent("Cancel", []
        {
            return OJustPressed(OBBtn);
        });
        OEvent->addEvent("Start", []
        {
            return OJustPressed(OStartBtn);
        });
        OEvent->addEvent("Back", []
        {
            return OJustPressed(OBackBtn) || OJustPressed(OBBtn);
        });
    }

    void cleanup()
    {
        delete OParticles;
        delete g_pAudioEngine;
        for (int i = 0; i < 4; ++i)
        {
            delete g_gamePads[i];
        }
        delete OInput;
        delete g_inputDevice;
        delete OContentManager;
        delete OPB;
        delete OSB;
        delete ORenderer;
        delete OWindow;
        delete OEvent;
    }

    // Start the engine
    void run(std::function<void()> initCallback, std::function<void()> updateCallback, std::function<void()> renderCallback)
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
        MSG msg = {0};
        while (true)
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

            // Sync to main callbacks
            g_mainSync.processQueue();

            // Update
            if (g_pAudioEngine) g_pAudioEngine->Update();
            auto framesToUpdate = g_timeInfo.update(OSettings->getIsFixedStep());
            ODT = onut::getTimeInfo().getDeltaTime<float>();
            while (framesToUpdate--)
            {
                g_inputDevice->update();
                OInput->update();
                POINT cur;
                GetCursorPos(&cur);
                ScreenToClient(OWindow->getHandle(), &cur);
                OInput->mousePos.x = cur.x;
                OInput->mousePos.y = cur.y;
                OInput->mousePosf.x = static_cast<float>(cur.x);
                OInput->mousePosf.y = static_cast<float>(cur.y);
                OMousePos = OInput->mousePosf;
                for (auto& gamePad : g_gamePads)
                {
                    gamePad->update();
                }
                AnimManager::getGlobalManager()->update();
                OEvent->processEvents();
                OParticles->update();
                if (updateCallback)
                {
                    updateCallback();
                }
            }

            // Render
            g_timeInfo.render();
            ORenderer->beginFrame();
            if (renderCallback)
            {
                renderCallback();
            }
            OParticles->render();
            ORenderer->endFrame();
        }

        cleanup();
    }

    GamePad* getGamePad(int index)
    {
        assert(index >= 0 && index <= 3);
        return g_gamePads[index];
    }

    const TimeInfo<>& getTimeInfo()
    {
        return g_timeInfo;
    }

    void drawPal(const OPal& pal, OFont* pFont)
    {
        static const float H = 32.f;
        float i = 0;
        OSB->begin();
        for (auto& color : pal)
        {
            OSB->drawRect(nullptr, {0, i, H * GOLDEN_RATIO, H}, color);
            i += H;
        }
        if (pFont)
        {
            i = 0;
            int index = 0;
            for (auto& color : pal)
            {
                std::stringstream ss;
                ss << index;
                pFont->draw<OCenter>(ss.str(), Rect{0, i, H * GOLDEN_RATIO, H}.Center(), Color::Black);
                i += H;
                ++index;
            }
        }
        OSB->end();
    }
}
