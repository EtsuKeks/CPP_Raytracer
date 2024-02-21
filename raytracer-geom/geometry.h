#pragma once

#include <vector.h>
#include <sphere.h>
#include <intersection.h>
#include <triangle.h>
#include <ray.h>

#include <optional>

std::optional<Intersection> GetIntersection(const Ray& ray, const Sphere& sphere) {
    const double epsilon = 0.000000000001;
    Vector co = ray.GetOrigin() - sphere.GetCenter();

    double d_prod_co = 2 * DotProduct(ray.GetDirection(), co);
    double co_squared_minus_r_squared = std::pow(Length(co), 2) - std::pow(sphere.GetRadius(), 2);

    double determinant_squared = std::pow(d_prod_co, 2) - 4 * co_squared_minus_r_squared;

    if (determinant_squared >= epsilon) {
        double determinant = std::sqrt(determinant_squared);
        double t1 = (-d_prod_co + determinant) / 2;
        double t2 = (-d_prod_co - determinant) / 2;
        if (t2 <= -epsilon && t1 > epsilon) {
            Vector point = ray.GetOrigin() + ray.GetDirection() * t1;
            Vector normal = -(point - sphere.GetCenter());
            return std::make_optional<Intersection>(point, normal, t1);
        } else if (t2 > epsilon && t1 > epsilon) {
            Vector point = ray.GetOrigin() + ray.GetDirection() * t2;
            Vector normal = point - sphere.GetCenter();
            return std::make_optional<Intersection>(point, normal, t2);
        } else {
            // discussible
            return std::nullopt;
        }
    } else if (determinant_squared > -epsilon && determinant_squared < epsilon) {
        double t = -d_prod_co / 2;
        Vector point = ray.GetOrigin() + ray.GetDirection() * t;
        Vector normal = point - sphere.GetCenter();
        return std::make_optional<Intersection>(point, normal, t);
    } else {
        return std::nullopt;
    }
}

std::optional<Intersection> GetIntersection(const Ray& ray, const Triangle& triangle) {
    const double epsilon = 0.000000000001;
    Vector edge_ab = triangle[1] - triangle[0];
    Vector edge_ac = triangle[2] - triangle[0];
    Vector h = CrossProduct(ray.GetDirection(), edge_ac);
    double a = DotProduct(edge_ab, h);

    if (a > -epsilon && a < epsilon) {
        return std::nullopt;
    }

    Vector s = ray.GetOrigin() - triangle[0];
    double u = DotProduct(s, h) / a;
    if (u < -epsilon || u > 1.0 + epsilon) {
        return std::nullopt;
    }

    Vector q = CrossProduct(s, edge_ab);
    double v = DotProduct(ray.GetDirection(), q) / a;
    if (v < -epsilon || u + v > 1.0 + epsilon) {
        return std::nullopt;
    }

    double t = DotProduct(edge_ac, q) / a;

    if (t > epsilon) {
        Vector point = ray.GetOrigin() + ray.GetDirection() * t;
        Vector normal = CrossProduct(triangle[1] - triangle[0], triangle[2] - triangle[0]);
        if (DotProduct(normal, ray.GetDirection()) > 0) {
            -normal;
        }
        return std::make_optional<Intersection>(point, normal, t);
    } else {
        return std::nullopt;
    }
}

Vector Reflect(const Vector& ray, const Vector& normal) {
    Vector perp = -(normal * DotProduct(ray, normal));
    Vector to_add = ray + perp;
    return perp + to_add;
}

std::optional<Vector> Refract(const Vector& ray, const Vector& normal, double eta) {
    Vector perp = -(normal * DotProduct(ray, normal));
    Vector to_add = ray + perp;
    double sin_theta_1 = Length(to_add);
    double sin_theta_2 = sin_theta_1 * eta;
    if (sin_theta_2 > 1) {
        return std::nullopt;
    }
    double cos_theta_2 = std::sqrt(1 - std::pow(sin_theta_2, 2));
    Vector to_add_now = to_add * eta;
    return std::make_optional<Vector>(to_add_now - normal * cos_theta_2);
}

Vector GetBarycentricCoords(const Triangle& triangle, const Vector& point) {
    double area = triangle.Area();
    Vector p = point - triangle[0];
    double v = Length(CrossProduct(triangle[1] - triangle[0], p)) / area / 2;
    double u = Length(CrossProduct(triangle[2] - triangle[0], p)) / area / 2;
    return Vector(1 - u - v, u, v);
}