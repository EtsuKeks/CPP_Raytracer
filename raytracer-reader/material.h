#pragma once

#include <vector.h>
#include <string>

struct Material {
    std::string name;
    Vector ambient_color = Vector(0, 0, 0);
    Vector diffuse_color = Vector(0, 0, 0);
    Vector specular_color = Vector(0, 0, 0);
    Vector intensity = Vector(0, 0, 0);
    double specular_exponent = 1.0;
    double refraction_index = 1.0;
    Vector albedo = Vector(1, 0, 0);
};
