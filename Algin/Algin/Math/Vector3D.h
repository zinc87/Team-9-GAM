#pragma once
#include "pch.h"
#include <string>


namespace AG {
	class Vector3D {
	public:
		float x, y, z;

		// constructor
        Vector3D(float x = 0.0f, float y = 0.0f, float z = 0.0f);

        // Operators
        Vector3D operator+(const Vector3D& v) const;
        Vector3D operator-(const Vector3D& v) const;
        Vector3D operator-() const;
        Vector3D operator*(float scalar) const;
        Vector3D operator/(float scalar) const;

        Vector3D& operator+=(const Vector3D& v);
        Vector3D& operator-=(const Vector3D& v);
        Vector3D& operator*=(float scalar);
        Vector3D& operator/=(float scalar);

        bool operator==(const Vector3D& other) const;
        bool operator!=(const Vector3D& other) const;

        // Math
        float magnitude() const;
        float magnitudeSquared() const;
        Vector3D normalize() const;
        void normalizeInPlace();

        // Vector products
        static float dot(const Vector3D& a, const Vector3D& b);
        static Vector3D cross(const Vector3D& a, const Vector3D& b);

        // Distance
        static float distance(const Vector3D& a, const Vector3D& b);
        static float distanceSquared(const Vector3D& a, const Vector3D& b);

        // Midpoint
        static Vector3D midPoint(const Vector3D& a, const Vector3D& b);

        // Debug string
        //std::string toString() const;

	};
}
