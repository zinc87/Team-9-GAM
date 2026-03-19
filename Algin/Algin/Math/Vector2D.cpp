#include "pch.h"
#include "Vector2D.h"
#include "cmath"


namespace AG {

    // Constructor
    Vector2D::Vector2D(float x, float y) : x(x), y(y) {}

    // ---------------- Operators ----------------

    // Addition
    Vector2D Vector2D::operator+(const Vector2D& v) const {
        return Vector2D(x + v.x, y + v.y);
    }

    // Subtraction
    Vector2D Vector2D::operator-(const Vector2D& v) const {
        return Vector2D(x - v.x, y - v.y);
    }

    // Negation
    Vector2D Vector2D::operator-() const {
        return Vector2D(-x, -y);
    }

    // Scalar multiplication
    Vector2D Vector2D::operator*(float scalar) const {
        return Vector2D(x * scalar, y * scalar);
    }

    // Scalar division
    Vector2D Vector2D::operator/(float scalar) const {
        
        return Vector2D(x / scalar, y / scalar);
    }

    // Compound operators
    Vector2D& Vector2D::operator+=(const Vector2D& v) {
        x += v.x; y += v.y;
        return *this;
    }

    Vector2D& Vector2D::operator-=(const Vector2D& v) {
        x -= v.x; y -= v.y;
        return *this;
    }

    Vector2D& Vector2D::operator*=(float scalar) {
        x *= scalar; y *= scalar;
        return *this;
    }

    Vector2D& Vector2D::operator/=(float scalar) {
        
        x /= scalar; y /= scalar;
        return *this;
    }

    // Equality
    bool Vector2D::operator==(const Vector2D& other) const {
        return x == other.x && y == other.y;
    }

    bool Vector2D::operator!=(const Vector2D& other) const {
        return !(*this == other);
    }

    // ---------------- Math ----------------

    // Length
    float Vector2D::magnitude() const {
        return std::sqrt(x * x + y * y);
    }

    float Vector2D::magnitudeSquared() const {
        return x * x + y * y;
    }

    // Normalize
    Vector2D Vector2D::normalize() const {
        float len = magnitude();
        if (len == 0.0f) {
            return Vector2D(0.0f, 0.0f);
        }
        return Vector2D(x / len, y / len);
    }

    void Vector2D::normalizeInPlace() {
        float len = magnitude();
        if (len == 0.0f) return;
        x /= len;
        y /= len;
    }

    // Get a perpendicular vector
    Vector2D Vector2D::normalVector() const {
        return Vector2D(-y, x);
    }

    // ---------------- Static helpers ----------------

    // Dot product
    float Vector2D::dotProduct(const Vector2D& a, const Vector2D& b) {
        return a.x * b.x + a.y * b.y;
    }

    // Cross product
    float Vector2D::crossProduct(const Vector2D& a, const Vector2D& b) {
        return a.x * b.y - a.y * b.x;
    }

    // Distance
    float Vector2D::distance(const Vector2D& a, const Vector2D& b) {
        return (a - b).magnitude();
    }

    float Vector2D::distanceSquared(const Vector2D& a, const Vector2D& b) {
        return (a - b).magnitudeSquared();
    }

    // Form vector from two points
    Vector2D Vector2D::formVector(const Vector2D& a, const Vector2D& b) {
        return Vector2D(b.x - a.x, b.y - a.y);
    }

    // Midpoint
    Vector2D Vector2D::midPoint(const Vector2D& a, const Vector2D& b) {
        return Vector2D((a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f);
    }

    // Orthogonal projection of point onto line
    Vector2D Vector2D::orthogonalProjection(
        const Vector2D& point,
        const Vector2D& lineStart,
        const Vector2D& lineEnd
    ) {
        Vector2D lineVec = formVector(lineStart, lineEnd);
        Vector2D pointVec = formVector(lineStart, point);

        float lineLenSq = lineVec.magnitudeSquared();
        if (lineLenSq == 0.0f) return lineStart;

        float t = dotProduct(pointVec, lineVec) / lineLenSq;
        return Vector2D(lineStart.x + t * lineVec.x,
            lineStart.y + t * lineVec.y);
    }
}
