#pragma once
#include "pch.h"
#include <string>

constexpr float M_PI_2D = 3.14159265358979323846f;

// constructor
// overload +=, -=, *=, /=
// overload binary + operator, -, *, /
// normalise
// length
// square of length
// 2D Distance
// 2D Square Distance
// dot product
// cross product


namespace AG {
	class Vector2D {
	public:

		float x, y;

        Vector2D(float x = 0.0f, float y = 0.0f);

        // accessors
        float getX() const { return x; }
        float getY() const { return y; }
        void setX(float newX) { x = newX; }
        void setY(float newY) { y = newY; }

        // operators
        Vector2D operator+(const Vector2D& v) const;
        Vector2D operator-(const Vector2D& v) const;
        Vector2D operator-() const;
        Vector2D operator*(float scalar) const;
        Vector2D operator/(float scalar) const;
        Vector2D& operator+=(const Vector2D& v);
        Vector2D& operator-=(const Vector2D& v);
        Vector2D& operator*=(float scalar);
        Vector2D& operator/=(float scalar);

        bool operator==(const Vector2D& other) const;
        bool operator!=(const Vector2D& other) const;

        // vector math
        float magnitude() const;
        float magnitudeSquared() const;
        Vector2D normalize() const;
        void normalizeInPlace();
        Vector2D normalVector() const;

        // static helper
        static float dotProduct(const Vector2D& a, const Vector2D& b);
        static float crossProduct(const Vector2D& a, const Vector2D& b);
        static float distance(const Vector2D& a, const Vector2D& b);
        static float distanceSquared(const Vector2D& a, const Vector2D& b);
        static Vector2D formVector(const Vector2D& a, const Vector2D& b);
        static Vector2D midPoint(const Vector2D& a, const Vector2D& b);

        // Projection
        static Vector2D orthogonalProjection(const Vector2D& point, const Vector2D& lineStart, const Vector2D& lineEnd);


        //std::string vector2dTestCases();

	private:

	};
}
