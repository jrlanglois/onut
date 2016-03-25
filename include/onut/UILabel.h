#pragma once
#include "onut/UIComponents.h"
#include "onut/UIControl.h"

#include "onut/ForwardDeclaration.h"
OForwardDeclare(UILabel)

namespace onut
{
    class UILabel final : public UIControl
    {
    public:
        static OUILabelRef create();

        UILabel();
        UILabel(const UILabel& other);

        Type getType() const override { return Type::Label; }

        UITextComponent textComponent;

    protected:
        void load(const rapidjson::Value& jsonNode) override;
        void save(rapidjson::Value& jsonNode, rapidjson::Allocator& allocator) const override;
        void renderControl(const OUIContextRef& context, const Rect& rect) override;
    };
};
