#pragma once

#include <triangle.h>
#include <material.h>
#include <sphere.h>
#include <vector.h>
#include <optional>

struct Object {
    const Material* material = nullptr;
    Triangle polygon;
    std::optional<Triangle> normals;

    const Vector* GetNormal(size_t index) const {
        if (!normals.has_value()) {
            return nullptr;
        }
        return &normals.value()[index];
    }
};

struct SphereObject {
    const Material* material = nullptr;
    Sphere sphere;
};
