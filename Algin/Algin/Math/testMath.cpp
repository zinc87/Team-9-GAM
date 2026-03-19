


#include "Vector2D.h"
#include "Vector3D.h"
#include "Matrix3x3.h"
#include "Matrix4x4.h"
#include "pch.h"

/*
#include <iomanip>
#include <iostream>


using namespace AG;

void testVector2D() {
    std::cout << "----- Vector2D Tests -----\n";

    Vector2D a(3, 4);
    Vector2D b(1, 2);

    // Magnitude
    std::cout << "Magnitude of a (3,4): " << a.magnitude() << " (expected 5)\n";

    // Addition
    Vector2D add = a + b;
    std::cout << "a + b = (" << add.x << "," << add.y << ") (expected 4,6)\n";

    // Subtraction
    Vector2D sub = a - b;
    std::cout << "a - b = (" << sub.x << "," << sub.y << ") (expected 2,2)\n";

    // Scalar multiplication
    Vector2D mul = a * 2.0f;
    std::cout << "a * 2 = (" << mul.x << "," << mul.y << ") (expected 6,8)\n";

    // Normalization
    Vector2D norm = a.normalize();
    std::cout << "a normalized = (" << std::fixed << std::setprecision(2)
        << norm.x << "," << norm.y << ") (expected 0.60,0.80)\n";

    // Dot product
    float dot = Vector2D::dotProduct(a, b);
    std::cout << "dot(a,b) = " << dot << " (expected 11)\n";

    // Cross product
    float cross = Vector2D::crossProduct(a, b);
    std::cout << "cross(a,b) = " << cross << " (expected 2)\n";

    // Distance
    float dist = Vector2D::distance(a, b);
    std::cout << "distance(a,b) = " << dist << " (expected ~2.828)\n";

    // Midpoint
    Vector2D mid = Vector2D::midPoint(a, b);
    std::cout << "midpoint(a,b) = (" << mid.x << "," << mid.y << ") (expected 2,3)\n";

    // Orthogonal projection
    Vector2D point(2, 3);
    Vector2D lineStart(0, 0);
    Vector2D lineEnd(4, 0);
    Vector2D proj = Vector2D::orthogonalProjection(point, lineStart, lineEnd);
    std::cout << "projection of (2,3) onto line (0,0)-(4,0) = ("
        << proj.x << "," << proj.y << ") (expected 2,0)\n";
}

void testVector3D() {
    std::cout << "----- Vector3D Tests -----\n";

    Vector3D a(1, 2, 3);
    Vector3D b(4, 5, 6);

    // Magnitude
    std::cout << "Magnitude of a (1,2,3): " << a.magnitude()
        << " (expected ~3.742)\n";

    // Addition
    Vector3D add = a + b;
    std::cout << "a + b = (" << add.x << "," << add.y << "," << add.z
        << ") (expected 5,7,9)\n";

    // Subtraction
    Vector3D sub = a - b;
    std::cout << "a - b = (" << sub.x << "," << sub.y << "," << sub.z
        << ") (expected -3,-3,-3)\n";

    // Scalar multiplication
    Vector3D mul = a * 2.0f;
    std::cout << "a * 2 = (" << mul.x << "," << mul.y << "," << mul.z
        << ") (expected 2,4,6)\n";

    // Normalization
    Vector3D norm = a.normalize();
    std::cout << "a normalized = (" << std::fixed << std::setprecision(2)
        << norm.x << "," << norm.y << "," << norm.z
        << ") (expected 0.27,0.53,0.80)\n";

    // Dot product
    float dot = Vector3D::dot(a, b);
    std::cout << "dot(a,b) = " << dot << " (expected 32)\n";

    // Cross product
    Vector3D cross = Vector3D::cross(a, b);
    std::cout << "cross(a,b) = (" << cross.x << "," << cross.y << ","
        << cross.z << ") (expected -3,6,-3)\n";

    // Distance
    float dist = Vector3D::distance(a, b);
    std::cout << "distance(a,b) = " << dist << " (expected ~5.196)\n";

    // Midpoint
    Vector3D mid = Vector3D::midPoint(a, b);
    std::cout << "midpoint(a,b) = (" << mid.x << "," << mid.y << "," << mid.z
        << ") (expected 2.5,3.5,4.5)\n";
}


void testMatrix3x3() {
    using namespace AG;

    std::cout << "=== Matrix3x3 Tests ===\n";

    // Identity
    Matrix3x3 I;
    Matrix3x3::Mtx33Identity(I);
    std::cout << "Identity:\n";
    I.debugPrint();
    // Expected:
    // [1 0 0]
    // [0 1 0]
    // [0 0 1]

    // Translation
    Matrix3x3 T;
    Matrix3x3::Mtx33Translate(T, 3.0f, 4.0f);
    std::cout << "Translation (3,4):\n";
    T.debugPrint();
    // Expected:
    // [1 0 3]
    // [0 1 4]
    // [0 0 1]

    // Scaling
    Matrix3x3 S;
    Matrix3x3::Mtx33Scale(S, 2.0f, 3.0f);
    std::cout << "Scaling (2,3):\n";
    S.debugPrint();
    // Expected:
    // [2 0 0]
    // [0 3 0]
    // [0 0 1]

    // Rotation 90 degrees
    Matrix3x3 R;
    Matrix3x3::Mtx33RotDeg(R, 90.0f);
    std::cout << "Rotation 90 deg:\n";
    R.debugPrint();
    // Expected:
    // [0 -1 0]
    // [1 0 0]
    // [0 0 1]

    // Multiplication: T * S
    Matrix3x3 TS = T * S;
    std::cout << "Translation * Scaling:\n";
    TS.debugPrint();
    // Expected:
    // [2 0 3]
    // [0 3 4]
    // [0 0 1]
}

void testMatrix4x4() {
    using namespace AG;

    std::cout << "=== Matrix4x4 Tests ===\n";

    // Identity
    Matrix4x4 I;
    Matrix4x4::Mtx44Identity(I);
    std::cout << "Identity:\n";
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            std::cout << I.m[i * 4 + j] << " ";
        }
        std::cout << "\n";
    }
    // Expected:
    // [1 0 0 0]
    // [0 1 0 0]
    // [0 0 1 0]
    // [0 0 0 1]

    // Translation
    Matrix4x4 T;
    Matrix4x4::Mtx44Translate(T, 1.0f, 2.0f, 3.0f);
    std::cout << "Translation (1,2,3):\n";
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            std::cout << T.m[i * 4 + j] << " ";
        }
        std::cout << "\n";
    }
    // Expected:
    // [1 0 0 1]
    // [0 1 0 2]
    // [0 0 1 3]
    // [0 0 0 1]

    // Scaling
    Matrix4x4 S;
    Matrix4x4::Mtx44Scale(S, 2.0f, 3.0f, 4.0f);
    std::cout << "Scaling (2,3,4):\n";
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            std::cout << S.m[i * 4 + j] << " ";
        }
        std::cout << "\n";
    }
    // Expected:
    // [2 0 0 0]
    // [0 3 0 0]
    // [0 0 4 0]
    // [0 0 0 1]

    // Rotation 90 degrees around Z
    Matrix4x4 R;
    Matrix4x4::Mtx44RotDeg(R, 90.0f);
    std::cout << "Rotation 90 deg (Z):\n";
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            std::cout << R.m[i * 4 + j] << " ";
        }
        std::cout << "\n";
    }
    // Expected:
    // [0 -1 0 0]
    // [1 0 0 0]
    // [0 0 1 0]
    // [0 0 0 1]

    // Multiplication: T * S
    Matrix4x4 TS = T * S;
    std::cout << "Translation * Scaling:\n";
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            std::cout << TS.m[i * 4 + j] << " ";
        }
        std::cout << "\n";
    }
    // Expected:
    // [2 0 0 1]
    // [0 3 0 2]
    // [0 0 4 3]
    // [0 0 0 1]
}

int main() {
    testVector2D();
    std::cout << "\n";
    testVector3D();
    std::cout << "\n";
    testMatrix3x3();
    std::cout << "\n";
    testMatrix4x4();
    return 0;
}

*/
