#pragma once

#include <material.h>
#include <vector.h>
#include <object.h>
#include <light.h>

#include <optional>
#include <iterator>
#include <regex>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <filesystem>

class Scene {
public:
    Scene(std::vector<Object>& objects, std::vector<SphereObject>& sphere_objects,
          std::vector<Light>& lights, std::unordered_map<std::string, Material>& materials)
        : objects_(std::move(objects)),
          sphere_objects_(std::move(sphere_objects)),
          lights_(std::move(lights)),
          materials_(std::move(materials)) {
    }

    const std::vector<Object>& GetObjects() const {
        return objects_;
    }

    const std::vector<SphereObject>& GetSphereObjects() const {
        return sphere_objects_;
    }

    const std::vector<Light>& GetLights() const {
        return lights_;
    }

    const std::unordered_map<std::string, Material>& GetMaterials() const {
        return materials_;
    }

private:
    std::vector<Object> objects_;
    std::vector<SphereObject> sphere_objects_;
    std::vector<Light> lights_;
    std::unordered_map<std::string, Material> materials_;
};

std::unordered_map<std::string, Material> ReadMaterials(const std::filesystem::path& path) {
    std::unordered_map<std::string, Material> materials;

    std::ifstream mtl_file(path.string());

    std::string line;
    Material cur_material;
    while (std::getline(mtl_file, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token.empty() || token[0] == '#') {
            continue;
        }

        if (token == "newmtl") {
            if (!cur_material.name.empty()) {
                materials.emplace(cur_material.name, cur_material);
            }
            cur_material = Material();
            iss >> cur_material.name;
        } else if (token == "Ka") {
            iss >> cur_material.ambient_color[0] >> cur_material.ambient_color[1] >>
                cur_material.ambient_color[2];
        } else if (token == "Kd") {
            iss >> cur_material.diffuse_color[0] >> cur_material.diffuse_color[1] >>
                cur_material.diffuse_color[2];
        } else if (token == "Ks") {
            iss >> cur_material.specular_color[0] >> cur_material.specular_color[1] >>
                cur_material.specular_color[2];
        } else if (token == "Ke") {
            iss >> cur_material.intensity[0] >> cur_material.intensity[1] >>
                cur_material.intensity[2];
        } else if (token == "Ns") {
            iss >> cur_material.specular_exponent;
        } else if (token == "Ni") {
            iss >> cur_material.refraction_index;
        } else if (token == "al") {
            iss >> cur_material.albedo[0] >> cur_material.albedo[1] >> cur_material.albedo[2];
        }
    }

    if (!cur_material.name.empty()) {
        materials.emplace(cur_material.name, cur_material);
    }
    mtl_file.close();

    return materials;
}

Scene ReadScene(const std::filesystem::path& path) {
    std::ifstream obj_file(path.string());

    std::vector<Vector> vertices;
    std::vector<Vector> normals;

    std::vector<Object> objects;
    std::vector<SphereObject> sphere_objects;
    std::vector<Light> lights;
    std::unordered_map<std::string, Material> materials;

    Material* cur_material = nullptr;

    while (!obj_file.eof()) {
        std::string line;
        std::getline(obj_file, line);

        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream iss(line);
        std::vector<std::string> tokens(std::istream_iterator<std::string>{iss},
                                        std::istream_iterator<std::string>());

        if (tokens[0] == "v" && tokens.size() >= 4) {
            Vector vertex;
            vertex[0] = std::stod(tokens[1]);
            vertex[1] = std::stod(tokens[2]);
            vertex[2] = std::stod(tokens[3]);
            vertices.push_back(vertex);
        } else if (tokens[0] == "vn" && tokens.size() >= 4) {
            Vector normal;
            normal[0] = std::stod(tokens[1]);
            normal[1] = std::stod(tokens[2]);
            normal[2] = std::stod(tokens[3]);
            normals.push_back(normal);
        } else if (tokens[0] == "f" && tokens.size() >= 4) {
            std::regex pattern("(-?\\d+)(/)?(-?\\d+)?(/)?(-?\\d+)?");
            std::smatch match;
            if (std::regex_match(tokens[1], match, pattern)) {
                if (match[5].matched) {
                    std::vector<Vector> vertices_temp;
                    std::vector<Vector> normals_temp;

                    for (size_t i = 1; i < tokens.size(); ++i) {
                        std::regex pattern("(-?\\d+)(/)?(-?\\d+)?(/)?(-?\\d+)?");
                        std::smatch match;
                        std::regex_match(tokens[i], match, pattern);
                        int num1 = std::stoi(match[1]);
                        int num3 = std::stoi(match[5]);
                        vertices_temp.push_back(num1 > 0 ? *(vertices.begin() + num1 - 1)
                                                         : *(vertices.end() + num1));
                        normals_temp.push_back(num3 > 0 ? *(normals.begin() + num3 - 1)
                                                        : *(normals.end() + num3));
                    }

                    for (size_t i = 0; i < tokens.size() - 3; ++i) {
                        objects.push_back(Object{
                            cur_material,
                            Triangle(vertices_temp[0], vertices_temp[i + 1], vertices_temp[i + 2]),
                            std::make_optional<Triangle>(normals_temp[0], normals_temp[i + 1],
                                                         normals_temp[i + 2])});
                    }
                } else {
                    std::vector<Vector> vertices_temp;

                    for (size_t i = 1; i < tokens.size(); ++i) {
                        std::regex pattern("(-?\\d+)(/)?(-?\\d+)?(/)?(-?\\d+)?");
                        std::smatch match;
                        std::regex_match(tokens[i], match, pattern);
                        int num1 = std::stoi(match[1]);
                        vertices_temp.push_back(num1 > 0 ? *(vertices.begin() + num1 - 1)
                                                         : *(vertices.end() + num1));
                    }

                    for (size_t i = 0; i < tokens.size() - 3; ++i) {
                        objects.push_back(Object{
                            cur_material,
                            Triangle(vertices_temp[0], vertices_temp[i + 1], vertices_temp[i + 2]),
                            std::nullopt});
                    }
                }
            }
        } else if (tokens[0] == "P" && tokens.size() >= 7) {
            Vector position;
            Vector intensity;
            lights.push_back(
                Light{Vector(std::stod(tokens[1]), std::stod(tokens[2]), std::stod(tokens[3])),
                      Vector(std::stod(tokens[4]), std::stod(tokens[5]), std::stod(tokens[6]))});
        } else if (tokens[0] == "S" && tokens.size() >= 5) {
            sphere_objects.push_back(SphereObject{
                cur_material,
                Sphere(Vector(std::stod(tokens[1]), std::stod(tokens[2]), std::stod(tokens[3])),
                       std::stod(tokens[4]))});
        } else if (tokens[0] == "mtllib" && tokens.size() >= 2) {
            std::string mtl_file_name = tokens[1];
            std::filesystem::path mtl_path = path.parent_path() / mtl_file_name;
            std::unordered_map<std::string, Material> cur_materials = ReadMaterials(mtl_path);
            materials.insert(cur_materials.begin(), cur_materials.end());
        } else if (tokens[0] == "usemtl" && tokens.size() >= 2) {
            cur_material = &materials.at(tokens[1]);
        }
    }
    obj_file.close();

    return Scene(objects, sphere_objects, lights, materials);
}