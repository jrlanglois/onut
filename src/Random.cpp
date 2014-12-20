#include <stdlib.h>
#include <time.h>

#include "Random.h"

namespace onut {
    void randomizeSeed() {
        srand(static_cast<unsigned int>(time(nullptr)));
    }

    int randi() {
        return rand();
    }

    int randi(int max) {
        return rand() % (max + 1);
    }

    int randi(int min, int max) {
        auto range = max - min + 1;
        return rand() % range + min;
    }

    float randf(float max) {
        auto rnd = rand();
        auto rndf = static_cast<double>(rnd) / static_cast<double>(RAND_MAX - 1);
        rndf *= static_cast<double>(max);
        return static_cast<float>(rndf);
    }

    float randf(float min, float max) {
        auto rnd = rand();
        auto rndf = static_cast<double>(rnd) / static_cast<double>(RAND_MAX - 1);
        rndf *= static_cast<double>(max - min);
        return static_cast<float>(rndf) + min;
    }

    Vector2 rand2f(const Vector2& max) {
        return{ randf(max.x), randf(max.y) };
    }

    Vector2 rand2f(const Vector2& min, const Vector2& max) {
        return{ randf(min.x, max.x), randf(min.y, max.y) };
    }

    Vector3 rand3f(const Vector3& max) {
        return{ randf(max.x), randf(max.y), randf(max.z) };
    }

    Vector3 rand3f(const Vector3& min, const Vector3& max) {
        return{ randf(min.x, max.x), randf(min.y, max.y), randf(min.z, max.z) };
    }

    Vector4 rand4f(const Vector4& max) {
        return{ randf(max.x), randf(max.y), randf(max.z), randf(max.w) };
    }

    Vector4 rand4f(const Vector4& min, const Vector4& max) {
        return{ randf(min.x, max.x), randf(min.y, max.y), randf(min.z, max.z), randf(min.w, max.w) };
    }

    Color randc(float alpha) {
        return{ randf(1.f), randf(1.f), randf(1.f), alpha };
    }

    Color randc(const Color& max) {
        return{ randf(max.x), randf(max.y), randf(max.z), randf(max.w) };
    }

    Color randc(const Color& min, const Color& max) {
        return{ randf(min.x, max.x), randf(min.y, max.y), randf(min.z, max.z), randf(min.w, max.w) };
    }

    Color randc(const Color& color, float variation) {
        return color;
    }

    Color randc(const std::vector<Color>& palette, float variation) {
        return randc(randv(palette), variation);
    }
}
