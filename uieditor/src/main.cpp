#include <Windows.h>
#include "ActionManager.h"
#include "DocumentView.h"
#include "events.h"
#include "menu.h"
#include "styles.h"

void init();
void update();
void render();

DocumentView*       g_pDocument = nullptr;
onut::UIContext*    g_pUIContext = nullptr;
onut::UIControl*    g_pUIScreen = nullptr;
onut::ActionManager g_actionManager;

int CALLBACK WinMain(HINSTANCE appInstance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdCount)
{
    OSettings->setResolution({1280, 720});
    OSettings->setGameName("Oak Nut UI Editor");
    OSettings->setIsResizableWindow(true);

    onut::run(init, update, render);

    return 0;
}

void init()
{
    OContentManager->addSearchPath("../../assets/textures/icons");

    g_pUIContext = new onut::UIContext(onut::sUIVector2{OScreenWf, OScreenHf});
    g_pUIContext->onClipping = [](bool enabled, const onut::sUIRect& rect)
    {
        if (enabled)
        {
            egEnable(EG_SCISSOR);
            egScissor((uint32_t)rect.position.x, (uint32_t)rect.position.y, (uint32_t)rect.size.x, (uint32_t)rect.size.y);
        }
        else
        {
            egDisable(EG_SCISSOR);
        }
    };
    createUIStyles(g_pUIContext);

    g_pUIContext->addStyle<onut::UIPanel>("sizableRegion", [](const onut::UIPanel* pPanel, const onut::sUIRect& rect)
    {
        OSB->drawRect(nullptr, onut::UI2Onut(rect), Color::Black);
        OSB->end();

        // Render edited UIs
        //  ORenderer->set2DCamera({std::roundf(-rect.position.x), std::roundf(-rect.position.y)});
        ORenderer->set2DCamera({-rect.position.x, -rect.position.y});
        OSB->begin();
        g_pDocument->render();
        OSB->end();

        ORenderer->set2DCamera({0, 0});
        OSB->begin();
    });

    g_pUIScreen = new onut::UIControl("../../assets/ui/editor.json");
    g_pUIScreen->retain();
    hookUIEvents(g_pUIScreen);

    g_pDocument = new DocumentView("");

    buildMenu();

    OWindow->onMenu = onMenu;
    OWindow->onWrite = [](char c){g_pUIContext->write(c); };
    OWindow->onKey = [](uintptr_t key)
    {
        if (!g_pDocument->isBusy())
        {
            g_pUIContext->keyDown(key);
            if (!dynamic_cast<onut::UITextBox*>(g_pUIContext->getFocusControl()))
            {
                checkShortCut(key);
                g_pDocument->onKeyDown(key);
            }
        }
    };
}

void update()
{
    // Adjust size
    g_pUIContext->resize({OScreenWf, OScreenHf});

    // Update.
    g_pUIScreen->update(*g_pUIContext, {OMousePos.x, OMousePos.y}, OInput->isStateDown(DIK_MOUSEB1));
    g_pDocument->update();
}

void render()
{
    ORenderer->clear(OColorHex(1e1e1e));

    OSB->begin();
    g_pUIScreen->render(*g_pUIContext);
    //OSB->drawRect(nullptr, {OMousePos, {10, 10}});
    OSB->end();
}
