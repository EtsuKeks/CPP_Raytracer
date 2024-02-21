#pragma once

#include <array>
#include <cstddef>
#include <cmath>

class Vector {
public:
    Vector() : data_({0, 0, 0}) {
    }

    Vector(double x, double y, double z) : data_({x, y, z}) {
    }

    double& operator[](size_t ind) {
        return data_.at(ind);
    }

    double operator[](size_t ind) const {
        return data_.at(ind);
    }

    void Normalize() {
        double norm = std::sqrt(std::pow(data_.at(0), 2) + std::pow(data_.at(1), 2) +
                                std::pow(data_.at(2), 2));
        for (double& num : data_) {
            num /= norm;
        }
    }

    friend Vector operator-(const Vector& lhs, const Vector& rhs) {
        return Vector(lhs[0] - rhs[0], lhs[1] - rhs[1], lhs[2] - rhs[2]);
    }

    friend Vector operator+(const Vector& lhs, const Vector& rhs) {
        return Vector(lhs[0] + rhs[0], lhs[1] + rhs[1], lhs[2] + rhs[2]);
    }

    friend Vector operator*(const Vector& lhs, const double& rhs) {
        return Vector(lhs[0] * rhs, lhs[1] * rhs, lhs[2] * rhs);
    }

    Vector& operator-() {
        data_[0] = -data_[0];
        data_[1] = -data_[1];
        data_[2] = -data_[2];
        return *this;
    }

private:
    std::array<double, 3> data_;
};

double DotProduct(const Vector& a, const Vector& b) {
    return (a[0] * b[0] + a[1] * b[1] + a[2] * b[2]);
}

Vector CrossProduct(const Vector& a, const Vector& b) {
    return Vector((a[1] * b[2] - b[1] * a[2]), (b[0] * a[2] - a[0] * b[2]),
                  (a[0] * b[1] - b[0] * a[1]));
}

double Length(const Vector& v) {
    return std::sqrt(std::pow(v[0], 2) + std::pow(v[1], 2) + std::pow(v[2], 2));
}