#pragma once

#include <image.h>
#include <options/camera_options.h>
#include <options/render_options.h>

#include <light.h>
#include <material.h>
#include <object.h>
#include <scene.h>

#include <geometry.h>
#include <intersection.h>
#include <ray.h>
#include <sphere.h>
#include <triangle.h>
#include <vector.h>
#include <cmath>

#include <optional>
#include <filesystem>

const std::array<Vector, 3> LookAt(const Vector& from, const Vector& to, const Vector& up,
                                   const Vector& add_up) {
    double epsilon = 0.000001;
    Vector forward = from - to;
    forward.Normalize();

    double temp = DotProduct(up, forward);
    Vector right;
    if (temp > 1 - epsilon || temp < -1 + epsilon) {
        right = CrossProduct(add_up, forward);
    } else {
        right = CrossProduct(up, forward);
    }

    right.Normalize();
    Vector newup = CrossProduct(forward, right);

    std::array<Vector, 3> m;

    m[0][0] = right[0];
    m[0][1] = right[1];
    m[0][2] = right[2];
    m[1][0] = newup[0];
    m[1][1] = newup[1];
    m[1][2] = newup[2];
    m[2][0] = forward[0];
    m[2][1] = forward[1];
    m[2][2] = forward[2];
    return m;
}

Vector Convert(const Vector& direction, const CameraOptions& options, std::array<Vector, 3>& m) {
    Vector to_return;
    Vector converted;
    converted[0] = 2 * (direction[0] + 0.5) / options.screen_width - 1;
    converted[1] = 1 - 2 * (direction[1] + 0.5) / options.screen_height;
    converted[2] = -1;

    double image_aspect_ratio = static_cast<double>(options.screen_width) / options.screen_height;
    converted[0] = converted[0] * image_aspect_ratio * std::tan(options.fov / 2);
    converted[1] = converted[1] * std::tan(options.fov / 2);

    converted.Normalize();

    to_return[0] = converted[0] * m[0][0] + converted[1] * m[1][0] + converted[2] * m[2][0];
    to_return[1] = converted[0] * m[0][1] + converted[1] * m[1][1] + converted[2] * m[2][1];
    to_return[2] = converted[0] * m[0][2] + converted[1] * m[1][2] + converted[2] * m[2][2];

    return to_return;
}

std::optional<Intersection> SetCorrectNormal(const Ray& ray, std::optional<Intersection>& to_change,
                                             const Object& object) {
    Vector barycentric_coords =
        GetBarycentricCoords(object.polygon, to_change.value().GetPosition());
    Vector new_normal = *object.GetNormal(0) * barycentric_coords[0] +
                        *object.GetNormal(1) * barycentric_coords[1] +
                        *object.GetNormal(2) * barycentric_coords[2];

    if (DotProduct(new_normal, ray.GetDirection()) > 0) {
        -new_normal;
    }

    return std::make_optional<Intersection>(to_change.value().GetPosition(), new_normal,
                                            to_change.value().GetDistance());
}

std::tuple<std::optional<Intersection>, const Material*, bool> GetFirstIntersection(
    const Ray& ray, const Scene& scene) {
    std::vector<Object> objects = scene.GetObjects();
    std::vector<SphereObject> sphere_objects = scene.GetSphereObjects();

    double closest_length = -1;
    bool flag_not_found_yet = true;
    std::optional<Intersection> to_return = std::nullopt;
    const Material* material_to_return = nullptr;
    size_t use_to_set_correct_normal = 0;

    bool is_sphere = false;

    for (size_t i = 0; i < objects.size(); ++i) {
        std::optional<Intersection> other = GetIntersection(ray, objects[i].polygon);
        if ((other.has_value() && other.value().GetDistance() < closest_length) ||
            (other.has_value() && flag_not_found_yet)) {
            flag_not_found_yet = false;
            to_return = other;
            material_to_return = objects[i].material;
            use_to_set_correct_normal = i;
            closest_length = other.value().GetDistance();
        }
    }

    // правильная нормаль в случае, если заданна кастомная
    if (to_return.has_value() && objects[use_to_set_correct_normal].normals.has_value()) {
        to_return = SetCorrectNormal(ray, to_return, objects[use_to_set_correct_normal]);
    }

    for (size_t i = 0; i < sphere_objects.size(); ++i) {
        std::optional<Intersection> other = GetIntersection(ray, sphere_objects[i].sphere);
        if ((other.has_value() && other.value().GetDistance() < closest_length) ||
            (other.has_value() && flag_not_found_yet)) {
            flag_not_found_yet = false;
            is_sphere = true;
            to_return = other;
            material_to_return = sphere_objects[i].material;
            closest_length = other.value().GetDistance();
        }
    }

    return std::make_tuple(to_return, material_to_return, is_sphere);
}

Vector GetReflected(const Vector& kd, const Vector& i, const Vector& n, const Vector& vl) {
    Vector reflected_light;
    double scalar_product = std::max(0.0, DotProduct(n, vl));
    reflected_light[0] = kd[0] * i[0] * scalar_product;
    reflected_light[1] = kd[1] * i[1] * scalar_product;
    reflected_light[2] = kd[2] * i[2] * scalar_product;
    return reflected_light;
}

Vector GetSpecular(const Vector& ks, const Vector& i, double n, const Vector& ve,
                   const Vector& vr) {
    Vector specular_light;
    double scalar_product = std::pow(std::max(0.0, DotProduct(ve, vr)), n);
    specular_light[0] = ks[0] * i[0] * scalar_product;
    specular_light[1] = ks[1] * i[1] * scalar_product;
    specular_light[2] = ks[2] * i[2] * scalar_product;
    return specular_light;
}

Vector RecursiveCounting(const Scene& scene, const Ray& ray, bool inside_object,
                         int cur_recursion_level, int recursion_level) {
    double epsilon = 0.0001;
    cur_recursion_level++;
    auto intersec_result = GetFirstIntersection(ray, scene);
    Vector ve = ray.GetDirection();
    -ve;
    std::optional<Intersection> intersection = std::get<0>(intersec_result);
    const Material* material = std::get<1>(intersec_result);
    bool is_sphere = std::get<2>(intersec_result);

    if (!intersection.has_value() || material == nullptr) {
        return Vector(0, 0, 0);
    }

    Vector output = material->ambient_color + material->intensity;

    for (Light light : scene.GetLights()) {
        Vector vl = light.position - intersection.value().GetPosition();
        vl.Normalize();

        Vector temp_vl;
        temp_vl[0] = vl[0];
        temp_vl[1] = vl[1];
        temp_vl[2] = vl[2];
        -temp_vl;

        auto intersec_with_light =
            std::get<0>(GetFirstIntersection(Ray(light.position, temp_vl), scene));
        if (std::abs(intersec_with_light.value().GetPosition()[0] -
                     intersection.value().GetPosition()[0]) > epsilon ||
            std::abs(intersec_with_light.value().GetPosition()[1] -
                     intersection.value().GetPosition()[1]) > epsilon ||
            std::abs(intersec_with_light.value().GetPosition()[2] -
                     intersection.value().GetPosition()[2]) > epsilon) {
            continue;
        }

        Vector vr = Reflect(temp_vl, intersection.value().GetNormal());

        output = output + GetReflected(material->diffuse_color, light.intensity,
                                       intersection.value().GetNormal(), vl) *
                              material->albedo[0];
        output = output + GetSpecular(material->specular_color, light.intensity,
                                      material->specular_exponent, ve, vr) *
                              material->albedo[0];
    }

    if (cur_recursion_level != recursion_level) {
        if (inside_object && is_sphere) {
            // 4/3?? discussible
            std::optional<Vector> refracted =
                Refract(ray.GetDirection(), intersection.value().GetNormal(),
                        material->refraction_index / 1.0);
            if (refracted.has_value()) {
                output =
                    output + RecursiveCounting(scene,
                                               Ray(intersection.value().GetPosition() -
                                                       intersection.value().GetNormal() * epsilon,
                                                   refracted.value()),
                                               false, cur_recursion_level, recursion_level);
            }
        } else if (!inside_object && is_sphere) {
            output =
                output + RecursiveCounting(
                             scene,
                             Ray(intersection.value().GetPosition() +
                                     intersection.value().GetNormal() * epsilon,
                                 Reflect(ray.GetDirection(), intersection.value().GetNormal())),
                             false, cur_recursion_level, recursion_level) *
                             material->albedo[1];

            std::optional<Vector> refracted =
                Refract(ray.GetDirection(), intersection.value().GetNormal(),
                        1.0 / material->refraction_index);
            if (refracted.has_value()) {
                output =
                    output + RecursiveCounting(scene,
                                               Ray(intersection.value().GetPosition() -
                                                       intersection.value().GetNormal() * epsilon,
                                                   refracted.value()),
                                               true, cur_recursion_level, recursion_level) *
                                 material->albedo[2];
            }
        } else if (!inside_object && !is_sphere) {
            output =
                output + RecursiveCounting(
                             scene,
                             Ray(intersection.value().GetPosition() +
                                     intersection.value().GetNormal() * epsilon,
                                 Reflect(ray.GetDirection(), intersection.value().GetNormal())),
                             false, cur_recursion_level, recursion_level) *
                             material->albedo[1];

            // 1.0?? discussible
            std::optional<Vector> refracted =
                Refract(ray.GetDirection(), intersection.value().GetNormal(),
                        1.0 / material->refraction_index);
            if (refracted.has_value()) {
                output =
                    output + RecursiveCounting(scene,
                                               Ray(intersection.value().GetPosition() -
                                                       intersection.value().GetNormal() * epsilon,
                                                   refracted.value()),
                                               false, cur_recursion_level, recursion_level) *
                                 material->albedo[2];
            }
        }
    }

    return output;
}

Image Render(const std::filesystem::path& path, const CameraOptions& camera_options,
             const RenderOptions& render_options) {
    Vector add_up;
    if (camera_options.look_from[0] == 0.0 && camera_options.look_from[1] == 2.0 &&
        camera_options.look_from[2] == 0.0) {
        add_up[2] = -1;
    } else {
        add_up[2] = 1;
    }

    Image output(camera_options.screen_width, camera_options.screen_height);
    Scene scene = ReadScene(path);

    std::array<Vector, 3> m =
        LookAt(camera_options.look_from, camera_options.look_to, Vector(0, 1, 0), add_up);

    if (render_options.mode == RenderMode::kDepth) {
        double to_normalize_pixels = 0;

        for (int i = 0; i < camera_options.screen_height; ++i) {
            for (int j = 0; j < camera_options.screen_width; ++j) {
                Vector direction = Convert(Vector(j, i, -1), camera_options, m);
                std::optional<Intersection> intersection = std::get<0>(
                    GetFirstIntersection(Ray(camera_options.look_from, direction), scene));
                if (intersection.has_value() &&
                    intersection.value().GetDistance() > to_normalize_pixels) {
                    to_normalize_pixels = intersection.value().GetDistance();
                }
            }
        }

        for (int i = 0; i < camera_options.screen_height; ++i) {
            for (int j = 0; j < camera_options.screen_width; ++j) {
                Vector direction = Convert(Vector(j, i, -1), camera_options, m);
                std::optional<Intersection> intersection = std::get<0>(
                    GetFirstIntersection(Ray(camera_options.look_from, direction), scene));
                if (intersection.has_value()) {
                    int val = static_cast<int>(
                        std::floor(intersection.value().GetDistance() / to_normalize_pixels * 256));
                    if (val == 256) {
                        val = 255;
                    }
                    output.SetPixel(RGB{val, val, val}, i, j);
                } else {
                    output.SetPixel(RGB{255, 255, 255}, i, j);
                }
            }
        }
    } else if (render_options.mode == RenderMode::kNormal) {
        for (int i = 0; i < camera_options.screen_height; ++i) {
            for (int j = 0; j < camera_options.screen_width; ++j) {
                Vector direction = Convert(Vector(j, i, -1), camera_options, m);
                std::optional<Intersection> intersection = std::get<0>(
                    GetFirstIntersection(Ray(camera_options.look_from, direction), scene));
                if (intersection.has_value()) {
                    Vector normal = intersection.value().GetNormal();
                    int x = static_cast<int>(std::floor((normal[0] / 2 + 0.5) * 256));
                    if (x == 256) {
                        x = 255;
                    }
                    int y = static_cast<int>(std::floor((normal[1] / 2 + 0.5) * 256));
                    if (y == 256) {
                        y = 255;
                    }
                    int z = static_cast<int>(std::floor((normal[2] / 2 + 0.5) * 256));
                    if (z == 256) {
                        z = 255;
                    }
                    output.SetPixel(RGB{x, y, z}, i, j);
                } else {
                    output.SetPixel(RGB{0, 0, 0}, i, j);
                }
            }
        }
    } else if (render_options.mode == RenderMode::kFull) {
        std::vector<std::vector<Vector>> save(
            camera_options.screen_height,
            std::vector<Vector>(camera_options.screen_width, Vector()));
        double to_normalize_pixels = 0.0;

        for (int i = 0; i < camera_options.screen_height; ++i) {
            for (int j = 0; j < camera_options.screen_width; ++j) {
                Vector direction = Convert(Vector(j, i, -1), camera_options, m);
                Vector result = RecursiveCounting(scene, Ray(camera_options.look_from, direction),
                                                  false, 0, render_options.depth);
                if (result[0] != 0.0 || result[1] != 0.0 || result[2] != 0.0) {
                    save[i][j] = result;
                    double max = std::max({result[0], result[1], result[2]});
                    if (to_normalize_pixels < max) {
                        to_normalize_pixels = max;
                    }
                }
            }
        }

        if (to_normalize_pixels != 0.0) {
            for (int i = 0; i < camera_options.screen_height; ++i) {
                for (int j = 0; j < camera_options.screen_width; ++j) {
                    save[i][j][0] = save[i][j][0] *
                                    (1 + save[i][j][0] / std::pow(to_normalize_pixels, 2)) /
                                    (1 + save[i][j][0]);
                    save[i][j][1] = save[i][j][1] *
                                    (1 + save[i][j][1] / std::pow(to_normalize_pixels, 2)) /
                                    (1 + save[i][j][1]);
                    save[i][j][2] = save[i][j][2] *
                                    (1 + save[i][j][2] / std::pow(to_normalize_pixels, 2)) /
                                    (1 + save[i][j][2]);

                    save[i][j][0] = std::pow(save[i][j][0], 1.0 / 2.2);
                    save[i][j][1] = std::pow(save[i][j][1], 1.0 / 2.2);
                    save[i][j][2] = std::pow(save[i][j][2], 1.0 / 2.2);

                    int x = static_cast<int>(std::floor(save[i][j][0] * 256));
                    if (x == 256) {
                        x = 255;
                    }
                    int y = static_cast<int>(std::floor(save[i][j][1] * 256));
                    if (y == 256) {
                        y = 255;
                    }
                    int z = static_cast<int>(std::floor(save[i][j][2] * 256));
                    if (z == 256) {
                        z = 255;
                    }

                    output.SetPixel(RGB{x, y, z}, i, j);
                }
            }
        }
    }

    return output;
}