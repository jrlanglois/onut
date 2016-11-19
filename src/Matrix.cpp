#include <onut/Matrix.h>

Matrix operator*(const Matrix& m, float S)
{
    Matrix ret;
    ret.m00 = m.m00 * S;
    ret.m01 = m.m01 * S;
    ret.m02 = m.m02 * S;
    ret.m03 = m.m03 * S;
    ret.m10 = m.m10 * S;
    ret.m11 = m.m11 * S;
    ret.m12 = m.m12 * S;
    ret.m13 = m.m13 * S;
    ret.m20 = m.m20 * S;
    ret.m21 = m.m21 * S;
    ret.m22 = m.m22 * S;
    ret.m23 = m.m23 * S;
    ret.m30 = m.m30 * S;
    ret.m31 = m.m31 * S;
    ret.m32 = m.m32 * S;
    ret.m33 = m.m33 * S;
    return std::move(ret);
}
