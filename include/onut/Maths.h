#pragma once
#include <onut/Color.h>
#include <onut/Matrix.h>
#include <onut/Quaternion.h>
#include <onut/Rect.h>
#include <onut/Vector2.h>
#include <onut/Vector3.h>
#include <onut/Vector4.h>

// Alignment
#define OTopLeft onut::Align::TopLeft
#define OTop onut::Align::Top
#define OTopRight onut::Align::TopRight
#define OLeft onut::Align::Left
#define OCenter onut::Align::Center
#define ORight onut::Align::Right
#define OBottomLeft onut::Align::BottomLeft
#define OBottom onut::Align::Bottom
#define OBottomRight onut::Align::BottomRight

// Constants
#define OPI xo::PI
#define OPIx2 xo::PIx2
#define OInversePIx2 xo::InversePIx2
#define OTAU xo::TAU
#define OHalfPI xo::HalfPI
#define OHalfPIx3 xo::HalfPIx3
#define OQuarterPI xo::QuarterPI

#define OFloatEpsilon xo::FloatEpsilon

#define ORad2Deg xo::Rad2Deg
#define ODeg2Rad xo::Deg2Rad


//--- Rect helpers for drawing
// Centered origin sprite
// cx,cy = Sprite center position
#define ORectCenteredOrigin(cx, cy, w, h)       Rect{(cx) - (w) * .5f, (cy) - (h) * .5f, w, h}

// Rectangle helpers
#define ORectFullScreen                         Rect{0, 0, OScreenWf, OScreenHf}
#define ORectGrid(cols, rows, ix, iy, px, py)   Rect{OScreenWf/(cols)*(ix)+((ix>0)?px*.5f:px),OScreenHf/(rows)*(iy)+((iy>0)?py*.5f:py),OScreenWf/(cols)-((ix<=0||ix>=cols-1)?px*1.5f:px),OScreenHf/(rows)-((iy<=0||iy>=rows-1)?py*1.5f:py)}
#define ORectLayout(left, top, right, bottom)   Rect{(left), (top), (right) - (left), (bottom) - (top)}
#define ORectAlign                              onut::alignedRect
template<typename TparentRect, typename Tsize>
inline Rect ORectFit(const TparentRect& parentRect, const Tsize& size)
{
    Rect ret;

    float scale = std::min<>((float)parentRect.z / (float)size.x, (float)parentRect.w / (float)size.y);

    ret.x = (float)parentRect.x + (float)parentRect.z * .5f - (float)size.x * scale * .5f;
    ret.y = (float)parentRect.y + (float)parentRect.w * .5f - (float)size.y * scale * .5f;
    ret.z = (float)size.x * scale;
    ret.w = (float)size.y * scale;

    return std::move(ret);
}

// Color stuff
#define OColorHex(hex)                          Color::fromHexRGB(0x ## hex)
#define OColorRGB(r, g, b)                      Color(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f)
#define OColorRGBA(r, g, b, a)                  Color(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f, static_cast<float>(a) / 255.0f)
#define OColorBGR(b, g, r)                      Color(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f)
#define OColorBGRA(b, g, r, a)                  Color(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f, static_cast<float>(a) / 255.0f)

// Gradient
#define OGradientH(left, right)                 {left, left, right, right}
#define OGradientV(top, bottom)                 {top, bottom, bottom, top}

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

namespace onut
{
    template<typename Tsize>
    Tsize max(Tsize a, Tsize b)
    {
        return std::max<Tsize>(a, b);
    }

    template<typename Tsize, typename ... Targs>
    Tsize max(Tsize a, Tsize b, Targs ... args)
    {
        return std::max<Tsize>(a, max(b, args...));
    }

    template<typename Tsize>
    Tsize min(Tsize a, Tsize b)
    {
        return std::min<Tsize>(a, b);
    }

    template<typename Tsize, typename ... Targs>
    Tsize min(Tsize a, Tsize b, Targs ... args)
    {
        return std::min<Tsize>(a, min(b, args...));
    }

    Vector2 tilesCollision(const Vector2& from, const Vector2& to, const Vector2& size, 
                           bool* pPassableTiles, int width, int height, int tileSize);
}

inline Vector4 ORectLocalToWorld(const Vector4& local, const Vector4& parent)
{
    auto ret = local;
    ret.x *= parent.z;
    ret.y *= parent.w;
    ret.x += parent.x;
    ret.y += parent.y;
    ret.z *= parent.z;
    ret.w *= parent.w;
    return std::move(ret);
}

inline Vector4 ORectWorldToLocal(const Vector4& world, const Vector4& parent)
{
    auto ret = world;
    ret.x -= parent.x;
    ret.y -= parent.y;
    ret.x /= parent.z;
    ret.y /= parent.w;
    ret.z /= parent.z;
    ret.w /= parent.w;
    return std::move(ret);
}
