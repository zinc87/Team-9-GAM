#include "pch.h"
#include "Vector3D.h"


namespace AG {
    // Constructor
    Vector3D::Vector3D(float x, float y, float z) : x(x), y(y), z(z) {}

    // Operators
    Vector3D Vector3D::operator+(const Vector3D& v) const {
        return Vector3D(x + v.x, y + v.y, z + v.z);
    }

    Vector3D Vector3D::operator-(const Vector3D& v) const {
        return Vector3D(x - v.x, y - v.y, z - v.z);
    }

    Vector3D Vector3D::operator-() const {
        return Vector3D(-x, -y, -z);
    }

    Vector3D Vector3D::operator*(float scalar) const {
        return Vector3D(x * scalar, y * scalar, z * scalar);
    }

    Vector3D Vector3D::operator/(float scalar) const {
        return Vector3D(x / scalar, y / scalar, z / scalar);
    }

    Vector3D& Vector3D::operator+=(const Vector3D& v) {
        x += v.x; y += v.y; z += v.z;
        return *this;
    }

    Vector3D& Vector3D::operator-=(const Vector3D& v) {
        x -= v.x; y -= v.y; z -= v.z;
        return *this;
    }

    Vector3D& Vector3D::operator*=(float scalar) {
        x *= scalar; y *= scalar; z *= scalar;
        return *this;
    }

    Vector3D& Vector3D::operator/=(float scalar) {
        x /= scalar; y /= scalar; z /= scalar;
        return *this;
    }

    bool Vector3D::operator==(const Vector3D& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    bool Vector3D::operator!=(const Vector3D& other) const {
        return !(*this == other);
    }

    // Math
    float Vector3D::magnitude() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    float Vector3D::magnitudeSquared() const {
        return x * x + y * y + z * z;
    }

    Vector3D Vector3D::normalize() const {
        float len = magnitude();
        if (len == 0.0f) return Vector3D(0.0f, 0.0f, 0.0f);
        return Vector3D(x / len, y / len, z / len);
    }

    void Vector3D::normalizeInPlace() {
        float len = magnitude();
        if (len == 0.0f) return;
        x /= len; y /= len; z /= len;
    }

    // Vector products
    float Vector3D::dot(const Vector3D& a, const Vector3D& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    Vector3D Vector3D::cross(const Vector3D& a, const Vector3D& b) {
        return Vector3D(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }

    // Distance
    float Vector3D::distance(const Vector3D& a, const Vector3D& b) {
        return (a - b).magnitude();
    }

    float Vector3D::distanceSquared(const Vector3D& a, const Vector3D& b) {
        return (a - b).magnitudeSquared();
    }

    // Midpoint
    Vector3D Vector3D::midPoint(const Vector3D& a, const Vector3D& b) {
        return Vector3D((a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f, (a.z + b.z) * 0.5f);
    }

    // Debug string
    /*std::string Vector3D::toString() const {
        std::ostringstream oss;
        oss << "(" << x << ", " << y << ", " << z << ")";
        return oss.str();
    }*/
}
