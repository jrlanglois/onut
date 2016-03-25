#include "onut/UIContext.h"
#include "onut/UIPanel.h"

#include "UIJson.h"

namespace onut
{
    OUIPanelRef UIPanel::create()
    {
        return OMake<UIPanel>();
    }

    UIPanel::UIPanel(const UIPanel& other) :
        UIControl(other)
    {
        color = other.color;
    }

    void UIPanel::load(const rapidjson::Value& jsonNode)
    {
        UIControl::load(jsonNode);
        color = getJsonColor(jsonNode["color"]);
    }

    void UIPanel::save(rapidjson::Value& jsonNode, rapidjson::Allocator& allocator) const
    {
        UIControl::save(jsonNode, allocator);
        setJsonColor(jsonNode, "color", color, allocator);
    }

    void UIPanel::renderControl(const OUIContextRef& context, const Rect& rect)
    {
        const auto& callback = context->getStyle<UIPanel>(getStyle());
        if (callback)
        {
            callback->render(OThis, rect);
        }
        else
        {
            context->drawRect(OThis, rect, color);
        }
    }
};