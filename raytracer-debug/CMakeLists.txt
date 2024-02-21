add_catch(test_raytracer_debug tests/test.cpp)

if (TEST_SOLUTION)
    target_include_directories(test_raytracer_debug PRIVATE ../tests/raytracer-geom)
    target_include_directories(test_raytracer_debug PRIVATE ../tests/raytracer-reader)
    target_include_directories(test_raytracer_debug PRIVATE ../tests/raytracer)
else()
    target_include_directories(test_raytracer_debug PRIVATE ../raytracer-geom)
    target_include_directories(test_raytracer_debug PRIVATE ../raytracer-reader)
endif()
target_include_directories(test_raytracer_debug PRIVATE ../raytracer)

target_link_libraries(test_raytracer_debug PRIVATE ${PNG_LIBRARY} ${JPEG_LIBRARIES})
target_include_directories(test_raytracer_debug PRIVATE ${PNG_INCLUDE_DIRS} ${JPEG_INCLUDE_DIRS})
