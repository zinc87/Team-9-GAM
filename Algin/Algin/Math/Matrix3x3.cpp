#include "Matrix3x3.h"
#include "math.h"
#include "pch.h"
#include "string"

constexpr float PIOVER180 = 0.0174532925199432f; //rad
constexpr float epsilon = 1e-6f;                 // epsilon

namespace AG {
	Matrix3x3::Matrix3x3() : m{ 0.0f }, m2{ {0.0f} } {}
    Matrix3x3::Matrix3x3(const float* pArr) : m2{ 0.0f } {
        for (int i = 0; i < 9; ++i) {
            m[i] = pArr[i];
        }
    }

    Matrix3x3::Matrix3x3(float _00, float _01, float _02,
        float _10, float _11, float _12,
        float _20, float _21, float _22) : m2{ 0.0f } {
        m[0] = _00; m[1] = _01; m[2] = _02;
        m[3] = _10; m[4] = _11; m[5] = _12;
        m[6] = _20; m[7] = _21; m[8] = _22;
    }

    Matrix3x3& Matrix3x3::operator=(const Matrix3x3& rhs) {
        if (this != &rhs) {
            for (int i = 0; i < 9; ++i) {
                m[i] = rhs.m[i];
            }
        }
        return *this;
    }

    Matrix3x3& Matrix3x3::operator*=(const Matrix3x3& rhs) {
        Matrix3x3 tmp;

        tmp.m[0] = m[0] * rhs.m[0] + m[1] * rhs.m[3] + m[2] * rhs.m[6];
        tmp.m[1] = m[0] * rhs.m[1] + m[1] * rhs.m[4] + m[2] * rhs.m[7];
        tmp.m[2] = m[0] * rhs.m[2] + m[1] * rhs.m[5] + m[2] * rhs.m[8];

        tmp.m[3] = m[3] * rhs.m[0] + m[4] * rhs.m[3] + m[5] * rhs.m[6];
        tmp.m[4] = m[3] * rhs.m[1] + m[4] * rhs.m[4] + m[5] * rhs.m[7];
        tmp.m[5] = m[3] * rhs.m[2] + m[4] * rhs.m[5] + m[5] * rhs.m[8];

        tmp.m[6] = m[6] * rhs.m[0] + m[7] * rhs.m[3] + m[8] * rhs.m[6];
        tmp.m[7] = m[6] * rhs.m[1] + m[7] * rhs.m[4] + m[8] * rhs.m[7];
        tmp.m[8] = m[6] * rhs.m[2] + m[7] * rhs.m[5] + m[8] * rhs.m[8];

        *this = tmp;
        return *this;
    }

    Matrix3x3 operator*(const Matrix3x3& lhs, const Matrix3x3& rhs) {
        Matrix3x3 result;
        result.m[0] = lhs.m[0] * rhs.m[0] + lhs.m[1] * rhs.m[3] + lhs.m[2] * rhs.m[6];
        result.m[1] = lhs.m[0] * rhs.m[1] + lhs.m[1] * rhs.m[4] + lhs.m[2] * rhs.m[7];
        result.m[2] = lhs.m[0] * rhs.m[2] + lhs.m[1] * rhs.m[5] + lhs.m[2] * rhs.m[8];

        result.m[3] = lhs.m[3] * rhs.m[0] + lhs.m[4] * rhs.m[3] + lhs.m[5] * rhs.m[6];
        result.m[4] = lhs.m[3] * rhs.m[1] + lhs.m[4] * rhs.m[4] + lhs.m[5] * rhs.m[7];
        result.m[5] = lhs.m[3] * rhs.m[2] + lhs.m[4] * rhs.m[5] + lhs.m[5] * rhs.m[8];

        result.m[6] = lhs.m[6] * rhs.m[0] + lhs.m[7] * rhs.m[3] + lhs.m[8] * rhs.m[6];
        result.m[7] = lhs.m[6] * rhs.m[1] + lhs.m[7] * rhs.m[4] + lhs.m[8] * rhs.m[7];
        result.m[8] = lhs.m[6] * rhs.m[2] + lhs.m[7] * rhs.m[5] + lhs.m[8] * rhs.m[8];

        return result;
    }

    void Matrix3x3::convertToGLM(const Matrix3x3& customMatrix, glm::mat3& glmMatrix) {
        // GLM is column-major

        glmMatrix[0][0] = customMatrix.m[0];  // First column
        glmMatrix[1][0] = customMatrix.m[3];
        glmMatrix[2][0] = customMatrix.m[6];

        glmMatrix[0][1] = customMatrix.m[1];  // Second column
        glmMatrix[1][1] = customMatrix.m[4];
        glmMatrix[2][1] = customMatrix.m[7];

        glmMatrix[0][2] = customMatrix.m[2];  // Third column
        glmMatrix[1][2] = customMatrix.m[5];
        glmMatrix[2][2] = customMatrix.m[8];
    }

    void Matrix3x3::Mtx33Identity(Matrix3x3& pResult) {
        pResult.m[0] = 1.0f; pResult.m[1] = 0.0f; pResult.m[2] = 0.0f;
        pResult.m[3] = 0.0f; pResult.m[4] = 1.0f; pResult.m[5] = 0.0f;
        pResult.m[6] = 0.0f; pResult.m[7] = 0.0f; pResult.m[8] = 1.0f;
    }

    void Matrix3x3::Mtx33Translate(Matrix3x3& pResult, float x, float y) {
        Mtx33Identity(pResult);
        pResult.m[2] = x;
        pResult.m[5] = y;
    }

    void Matrix3x3::Mtx33Scale(Matrix3x3& pResult, float x, float y) {
        Mtx33Identity(pResult);
        pResult.m[0] = x;
        pResult.m[4] = y;
    }

    void Matrix3x3::Mtx33RotRad(Matrix3x3& pResult, float angle) {
        float cosA = cos(angle);
        float sinA = sin(angle);

        if (fabs(cosA) < epsilon) {
            cosA = 0.0;
        }
        if (fabs(sinA) < epsilon) {
            sinA = 0.0;
        }

        pResult.m[0] = cosA;  pResult.m[1] = -sinA; pResult.m[2] = 0.0f;
        pResult.m[3] = sinA;  pResult.m[4] = cosA;  pResult.m[5] = 0.0f;
        pResult.m[6] = 0.0f;  pResult.m[7] = 0.0f;  pResult.m[8] = 1.0f;
    }

    void Matrix3x3::Mtx33RotDeg(Matrix3x3& pResult, float angle) {
        Mtx33RotRad(pResult, angle * PIOVER180);
    }

    void Matrix3x3::Mtx33Transpose(Matrix3x3& pResult, const Matrix3x3& pMtx) {
        pResult.m[0] = pMtx.m[0]; pResult.m[1] = pMtx.m[3]; pResult.m[2] = pMtx.m[6];
        pResult.m[3] = pMtx.m[1]; pResult.m[4] = pMtx.m[4]; pResult.m[5] = pMtx.m[7];
        pResult.m[6] = pMtx.m[2]; pResult.m[7] = pMtx.m[5]; pResult.m[8] = pMtx.m[8];
    }

    
    void Matrix3x3::Mtx33Inverse(Matrix3x3* pResult, float* determinant, const Matrix3x3& pMtx) {
        *determinant = pMtx.m[0] * (pMtx.m[4] * pMtx.m[8] - pMtx.m[7] * pMtx.m[5]) -
            pMtx.m[1] * (pMtx.m[3] * pMtx.m[8] - pMtx.m[6] * pMtx.m[5]) +
            pMtx.m[2] * (pMtx.m[3] * pMtx.m[7] - pMtx.m[6] * pMtx.m[4]);

        if (*determinant == 0.0f) {
            std::cerr << "Matrix is not invertible\n";
            return;
        }

        float invDet = 1.0f / *determinant;

        pResult->m[0] = invDet * (pMtx.m[4] * pMtx.m[8] - pMtx.m[7] * pMtx.m[5]);
        pResult->m[1] = -invDet * (pMtx.m[1] * pMtx.m[8] - pMtx.m[7] * pMtx.m[2]);
        pResult->m[2] = invDet * (pMtx.m[1] * pMtx.m[5] - pMtx.m[4] * pMtx.m[2]);

        pResult->m[3] = -invDet * (pMtx.m[3] * pMtx.m[8] - pMtx.m[6] * pMtx.m[5]);
        pResult->m[4] = invDet * (pMtx.m[0] * pMtx.m[8] - pMtx.m[6] * pMtx.m[2]);
        pResult->m[5] = -invDet * (pMtx.m[0] * pMtx.m[5] - pMtx.m[3] * pMtx.m[2]);

        pResult->m[6] = invDet * (pMtx.m[3] * pMtx.m[7] - pMtx.m[6] * pMtx.m[4]);
        pResult->m[7] = -invDet * (pMtx.m[0] * pMtx.m[7] - pMtx.m[6] * pMtx.m[1]);
        pResult->m[8] = invDet * (pMtx.m[0] * pMtx.m[4] - pMtx.m[3] * pMtx.m[1]);

        // to remove '-' from zero when printing
        for (int i = 0; i < 9; ++i) {
            if (pResult->m[i] == 0.0f) {
                pResult->m[i] = 0.0f;
            }
        }
    }
}
