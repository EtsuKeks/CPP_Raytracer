#include <options/camera_options.h>
#include <options/render_options.h>
#include <tests/commons.h>
#include <raytracer.h>
#include <util.h>
#include <image.h>

#include <cmath>
#include <string_view>
#include <optional>
#include <numbers>

#include <catch2/catch_test_macros.hpp>

void CheckImage(std::string_view obj_filename, std::string_view result_filename,
                const CameraOptions& camera_options, const RenderOptions& render_options,
                const std::optional<std::filesystem::path>& output_path) {
    static const auto kTestsDir = GetFileDir(__FILE__);
    auto image = Render(kTestsDir / obj_filename, camera_options, render_options);
    if (output_path) {
        image.Write(*output_path);
    }
    Compare(image, Image{kTestsDir / result_filename});
}

TEST_CASE("Shading parts") {
    CameraOptions camera_opts{640, 480};
    CheckImage("shading_parts/scene.obj", "shading_parts/scene.png", camera_opts, {1}, GetFileDir(__FILE__) / "shading_parts/temp.png");
}

TEST_CASE("Triangle") {
    CameraOptions camera_opts{.screen_width = 640,
                              .screen_height = 480,
                              .look_from = {0., 2., 0.},
                              .look_to = {0., 0., 0.}};
    CheckImage("triangle/scene.obj", "triangle/scene.png", camera_opts, {1}, GetFileDir(__FILE__) / "triangle/temp.png");
}

TEST_CASE("Triangle2") {
    CameraOptions camera_opts{.screen_width = 640,
                              .screen_height = 480,
                              .look_from = {0., -2., 0.},
                              .look_to = {0., 0., 0.}};
    CheckImage("triangle/scene.obj", "triangle/black.png", camera_opts, {1}, GetFileDir(__FILE__) / "triangle/temp2.png");
}

TEST_CASE("Classic box", "[no_asan]") {
    CameraOptions camera_opts{.screen_width = 500,
                              .screen_height = 500,
                              .look_from = {-.5, 1.5, .98},
                              .look_to = {0., 1., 0.}};
    CheckImage("classic_box/CornellBox.obj", "classic_box/first.png", camera_opts, {4}, GetFileDir(__FILE__) / "classic_box/temp.png");
    camera_opts.look_from = {-.9, 1.9, -1};
    camera_opts.look_to = {0., 0., 0.};
    CheckImage("classic_box/CornellBox.obj", "classic_box/second.png", camera_opts, {4}, GetFileDir(__FILE__) / "classic_box/temp2.png");
}

TEST_CASE("Mirrors", "[no_asan]") {
    CameraOptions camera_opts{.screen_width = 800,
                              .screen_height = 600,
                              .look_from = {2., 1.5, -.1},
                              .look_to = {1., 1.2, -2.8}};
    CheckImage("mirrors/scene.obj", "mirrors/result.png", camera_opts, {9}, GetFileDir(__FILE__) / "mirrors/temp.png");
}

TEST_CASE("Box with spheres") {
    CameraOptions camera_opts{.screen_width = 640,
                              .screen_height = 480,
                              .fov = std::numbers::pi / 3,
                              .look_from = {0., .7, 1.75},
                              .look_to = {0., .7, 0.}};
    CheckImage("box/cube.obj", "box/cube.png", camera_opts, {4}, GetFileDir(__FILE__) / "box/temp.png");
}

TEST_CASE("Distorted box", "[no_asan]") {
    CameraOptions camera_opts{.screen_width = 500,
                              .screen_height = 500,
                              .look_from = {-0.5, 1.5, 1.98},
                              .look_to = {0., 1., 0.}};
    CheckImage("distorted_box/CornellBox.obj", "distorted_box/result.png", camera_opts, {4}, GetFileDir(__FILE__) / "distorted_box/temp.png");
}

TEST_CASE("Deer", "[no_asan]") {
    CameraOptions camera_opts{.screen_width = 500,
                              .screen_height = 500,
                              .look_from = {100., 200., 150.},
                              .look_to = {0., 100., 0.}};
    CheckImage("deer/CERF_Free.obj", "deer/result.png", camera_opts, {1}, GetFileDir(__FILE__) / "deer/temp.png");
}
