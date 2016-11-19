#pragma once

#include <xo-math/xo-math.h>
using Rect = xo::Vector4;

struct iRect
{
    int left;
    int top;
    int right;
    int bottom;

    bool operator==(const iRect& other) const
    {
        return
            other.left == left &&
            other.right == right &&
            other.top == top &&
            other.bottom == bottom;
    }

    bool operator!=(const iRect& other) const
    {
        return
            other.left != left ||
            other.right != right ||
            other.top != top ||
            other.bottom != bottom;
    }
};
