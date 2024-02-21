#pragma once

#include <vector.h>

class Triangle {
public:
    Triangle(const Vector& a, const Vector& b, const Vector& c) : data_({a, b, c}) {
    }

    const Vector& operator[](size_t ind) const {
        return data_.at(ind);
    }

    double Area() const {
        return Length(CrossProduct(data_[1] - data_[0], data_[2] - data_[0])) / 2;
    }

private:
    const std::array<Vector, 3> data_;
};