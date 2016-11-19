#pragma once

#include <xo-math/xo-math.h>
using Matrix = xo::Matrix4x4;

Matrix operator*(const Matrix& m, float S);
