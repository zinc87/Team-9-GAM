
#include "Matrix4x4.h"
#include "math.h"
#include "pch.h"
#include "string"

constexpr float PIOVER180 = 0.0174532925199432f; //rad
constexpr float epsilon = 1e-6f;                 // epsilon

namespace AG {
    Matrix4x4::Matrix4x4() : m{ 0.0f } {}
   
    Matrix4x4::Matrix4x4(const float* pArr) {
        for (int i = 0; i < 16; ++i) {
            m[i] = pArr[i];
        }
    }
   
    Matrix4x4::Matrix4x4(float _00, float _01, float _02, float _03,
        float _10, float _11, float _12, float _13,
        float _20, float _21, float _22, float _23,
        float _30, float _31, float _32, float _33) {
        m[0] = _00; m[1] = _01; m[2] = _02; m[3] = _03;
        m[4] = _10; m[5] = _11; m[6] = _12; m[7] = _13;
        m[8] = _20; m[9] = _21; m[10] = _22; m[11] = _23;
        m[12] = _30; m[13] = _31; m[14] = _32; m[15] = _33;
    }

  
    Matrix4x4& Matrix4x4::operator=(const Matrix4x4& rhs) {
        if (this != &rhs) {
            for (int i = 0; i < 16; ++i) {
                m[i] = rhs.m[i];
            }
        }
        return *this;
    }

   
    Matrix4x4& Matrix4x4::operator*=(const Matrix4x4& rhs) {
        Matrix4x4 tmp;

        tmp.m[0] = m[0] * rhs.m[0] + m[1] * rhs.m[4] + m[2] * rhs.m[8] + m[3] * rhs.m[12];
        tmp.m[1] = m[0] * rhs.m[1] + m[1] * rhs.m[5] + m[2] * rhs.m[9] + m[3] * rhs.m[13];
        tmp.m[2] = m[0] * rhs.m[2] + m[1] * rhs.m[6] + m[2] * rhs.m[10] + m[3] * rhs.m[14];
        tmp.m[3] = m[0] * rhs.m[3] + m[1] * rhs.m[7] + m[2] * rhs.m[11] + m[3] * rhs.m[15];

        tmp.m[4] = m[4] * rhs.m[0] + m[5] * rhs.m[4] + m[6] * rhs.m[8] + m[7] * rhs.m[12];
        tmp.m[5] = m[4] * rhs.m[1] + m[5] * rhs.m[5] + m[6] * rhs.m[9] + m[7] * rhs.m[13];
        tmp.m[6] = m[4] * rhs.m[2] + m[5] * rhs.m[6] + m[6] * rhs.m[10] + m[7] * rhs.m[14];
        tmp.m[7] = m[4] * rhs.m[3] + m[5] * rhs.m[7] + m[6] * rhs.m[11] + m[7] * rhs.m[15];

        tmp.m[8] = m[8] * rhs.m[0] + m[9] * rhs.m[4] + m[10] * rhs.m[8] + m[11] * rhs.m[12];
        tmp.m[9] = m[8] * rhs.m[1] + m[9] * rhs.m[5] + m[10] * rhs.m[9] + m[11] * rhs.m[13];
        tmp.m[10] = m[8] * rhs.m[2] + m[9] * rhs.m[6] + m[10] * rhs.m[10] + m[11] * rhs.m[14];
        tmp.m[11] = m[8] * rhs.m[3] + m[9] * rhs.m[7] + m[10] * rhs.m[11] + m[11] * rhs.m[15];

        tmp.m[12] = m[12] * rhs.m[0] + m[13] * rhs.m[4] + m[14] * rhs.m[8] + m[15] * rhs.m[12];
        tmp.m[13] = m[12] * rhs.m[1] + m[13] * rhs.m[5] + m[14] * rhs.m[9] + m[15] * rhs.m[13];
        tmp.m[14] = m[12] * rhs.m[2] + m[13] * rhs.m[6] + m[14] * rhs.m[10] + m[15] * rhs.m[14];
        tmp.m[15] = m[12] * rhs.m[3] + m[13] * rhs.m[7] + m[14] * rhs.m[11] + m[15] * rhs.m[15];

        *this = tmp;
        return *this;
    }

    Matrix4x4 operator*(const Matrix4x4& lhs, const Matrix4x4& rhs) {
        Matrix4x4 result;

        result.m[0] = lhs.m[0] * rhs.m[0] + lhs.m[1] * rhs.m[4] + lhs.m[2] * rhs.m[8] + lhs.m[3] * rhs.m[12];
        result.m[1] = lhs.m[0] * rhs.m[1] + lhs.m[1] * rhs.m[5] + lhs.m[2] * rhs.m[9] + lhs.m[3] * rhs.m[13];
        result.m[2] = lhs.m[0] * rhs.m[2] + lhs.m[1] * rhs.m[6] + lhs.m[2] * rhs.m[10] + lhs.m[3] * rhs.m[14];
        result.m[3] = lhs.m[0] * rhs.m[3] + lhs.m[1] * rhs.m[7] + lhs.m[2] * rhs.m[11] + lhs.m[3] * rhs.m[15];

        result.m[4] = lhs.m[4] * rhs.m[0] + lhs.m[5] * rhs.m[4] + lhs.m[6] * rhs.m[8] + lhs.m[7] * rhs.m[12];
        result.m[5] = lhs.m[4] * rhs.m[1] + lhs.m[5] * rhs.m[5] + lhs.m[6] * rhs.m[9] + lhs.m[7] * rhs.m[13];
        result.m[6] = lhs.m[4] * rhs.m[2] + lhs.m[5] * rhs.m[6] + lhs.m[6] * rhs.m[10] + lhs.m[7] * rhs.m[14];
        result.m[7] = lhs.m[4] * rhs.m[3] + lhs.m[5] * rhs.m[7] + lhs.m[6] * rhs.m[11] + lhs.m[7] * rhs.m[15];

        result.m[8] = lhs.m[8] * rhs.m[0] + lhs.m[9] * rhs.m[4] + lhs.m[10] * rhs.m[8] + lhs.m[11] * rhs.m[12];
        result.m[9] = lhs.m[8] * rhs.m[1] + lhs.m[9] * rhs.m[5] + lhs.m[10] * rhs.m[9] + lhs.m[11] * rhs.m[13];
        result.m[10] = lhs.m[8] * rhs.m[2] + lhs.m[9] * rhs.m[6] + lhs.m[10] * rhs.m[10] + lhs.m[11] * rhs.m[14];
        result.m[11] = lhs.m[8] * rhs.m[3] + lhs.m[9] * rhs.m[7] + lhs.m[10] * rhs.m[11] + lhs.m[11] * rhs.m[15];

        result.m[12] = lhs.m[12] * rhs.m[0] + lhs.m[13] * rhs.m[4] + lhs.m[14] * rhs.m[8] + lhs.m[15] * rhs.m[12];
        result.m[13] = lhs.m[12] * rhs.m[1] + lhs.m[13] * rhs.m[5] + lhs.m[14] * rhs.m[9] + lhs.m[15] * rhs.m[13];
        result.m[14] = lhs.m[12] * rhs.m[2] + lhs.m[13] * rhs.m[6] + lhs.m[14] * rhs.m[10] + lhs.m[15] * rhs.m[14];
        result.m[15] = lhs.m[12] * rhs.m[3] + lhs.m[13] * rhs.m[7] + lhs.m[14] * rhs.m[11] + lhs.m[15] * rhs.m[15];

        return result;
    }

    void Matrix4x4::convertToGLM(const Matrix4x4& customMatrix, glm::mat4& glmMatrix) {
       

        glmMatrix[0][0] = customMatrix.m[0];   // First column
        glmMatrix[1][0] = customMatrix.m[4];
        glmMatrix[2][0] = customMatrix.m[8];
        glmMatrix[3][0] = customMatrix.m[12];

        glmMatrix[0][1] = customMatrix.m[1];   // Second column
        glmMatrix[1][1] = customMatrix.m[5];
        glmMatrix[2][1] = customMatrix.m[9];
        glmMatrix[3][1] = customMatrix.m[13];

        glmMatrix[0][2] = customMatrix.m[2];   // Third column
        glmMatrix[1][2] = customMatrix.m[6];
        glmMatrix[2][2] = customMatrix.m[10];
        glmMatrix[3][2] = customMatrix.m[14];

        glmMatrix[0][3] = customMatrix.m[3];   // Fourth column (translation)
        glmMatrix[1][3] = customMatrix.m[7];
        glmMatrix[2][3] = customMatrix.m[11];
        glmMatrix[3][3] = customMatrix.m[15];
    }

    void Matrix4x4::Mtx44Identity(Matrix4x4& pResult) {
        pResult.m[0] = 1.0f; pResult.m[1] = 0.0f; pResult.m[2] = 0.0f; pResult.m[3] = 0.0f;
        pResult.m[4] = 0.0f; pResult.m[5] = 1.0f; pResult.m[6] = 0.0f; pResult.m[7] = 0.0f;
        pResult.m[8] = 0.0f; pResult.m[9] = 0.0f; pResult.m[10] = 1.0f; pResult.m[11] = 0.0f;
        pResult.m[12] = 0.0f; pResult.m[13] = 0.0f; pResult.m[14] = 0.0f; pResult.m[15] = 1.0f;
    }

    void Matrix4x4::Mtx44Translate(Matrix4x4& pResult, float x, float y, float z) {
        Mtx44Identity(pResult);
        pResult.m[3] = x;
        pResult.m[7] = y;
        pResult.m[11] = z;
    }

    void Matrix4x4::Mtx44Scale(Matrix4x4& pResult, float x, float y, float z) {
        Mtx44Identity(pResult);
        pResult.m[0] = x;
        pResult.m[5] = y;
        pResult.m[10] = z;
    }

    void Matrix4x4::Mtx44RotRad(Matrix4x4& pResult, float angle) {
        float cosA = cos(angle);
        float sinA = sin(angle);

        if (fabs(cosA) < epsilon) {
            cosA = 0.0;
        }
        if (fabs(sinA) < epsilon) {
            sinA = 0.0;
        }

        pResult.m[0] = cosA;  pResult.m[1] = -sinA; pResult.m[2] = 0.0f; pResult.m[3] = 0.0f;
        pResult.m[4] = sinA;  pResult.m[5] = cosA;  pResult.m[6] = 0.0f; pResult.m[7] = 0.0f;
        pResult.m[8] = 0.0f;  pResult.m[9] = 0.0f;  pResult.m[10] = 1.0f; pResult.m[11] = 0.0f;
        pResult.m[12] = 0.0f;  pResult.m[13] = 0.0f;  pResult.m[14] = 0.0f; pResult.m[15] = 1.0f;
    }

    void Matrix4x4::Mtx44RotDeg(Matrix4x4& pResult, float angle) {
        Mtx44RotRad(pResult, angle * PIOVER180);
    }

    void Matrix4x4::Mtx44Transpose(Matrix4x4& pResult, const Matrix4x4& pMtx) {
        pResult.m[0] = pMtx.m[0];  pResult.m[1] = pMtx.m[4];  pResult.m[2] = pMtx.m[8];  pResult.m[3] = pMtx.m[12];
        pResult.m[4] = pMtx.m[1];  pResult.m[5] = pMtx.m[5];  pResult.m[6] = pMtx.m[9];  pResult.m[7] = pMtx.m[13];
        pResult.m[8] = pMtx.m[2];  pResult.m[9] = pMtx.m[6];  pResult.m[10] = pMtx.m[10]; pResult.m[11] = pMtx.m[14];
        pResult.m[12] = pMtx.m[3];  pResult.m[13] = pMtx.m[7];  pResult.m[14] = pMtx.m[11]; pResult.m[15] = pMtx.m[15];
    }

    void Matrix4x4::Mtx44Inverse(Matrix4x4* pResult, float* determinant, const Matrix4x4& pMtx) {
        // Calculate determinant
        *determinant = pMtx.m[0] * (pMtx.m[5] * (pMtx.m[10] * pMtx.m[15] - pMtx.m[14] * pMtx.m[11]) -
            pMtx.m[6] * (pMtx.m[9] * pMtx.m[15] - pMtx.m[13] * pMtx.m[11]) +
            pMtx.m[7] * (pMtx.m[9] * pMtx.m[14] - pMtx.m[13] * pMtx.m[10])) -

            pMtx.m[1] * (pMtx.m[4] * (pMtx.m[10] * pMtx.m[15] - pMtx.m[14] * pMtx.m[11]) -
                pMtx.m[6] * (pMtx.m[8] * pMtx.m[15] - pMtx.m[12] * pMtx.m[11]) +
                pMtx.m[7] * (pMtx.m[8] * pMtx.m[14] - pMtx.m[12] * pMtx.m[10])) +

            pMtx.m[2] * (pMtx.m[4] * (pMtx.m[9] * pMtx.m[15] - pMtx.m[13] * pMtx.m[11]) -
                pMtx.m[5] * (pMtx.m[8] * pMtx.m[15] - pMtx.m[12] * pMtx.m[11]) +
                pMtx.m[7] * (pMtx.m[8] * pMtx.m[13] - pMtx.m[12] * pMtx.m[9])) -

            pMtx.m[3] * (pMtx.m[4] * (pMtx.m[9] * pMtx.m[14] - pMtx.m[13] * pMtx.m[10]) -
                pMtx.m[5] * (pMtx.m[8] * pMtx.m[14] - pMtx.m[12] * pMtx.m[10]) +
                pMtx.m[6] * (pMtx.m[8] * pMtx.m[13] - pMtx.m[12] * pMtx.m[9]));

        if (*determinant == 0.0f) {
            // error message
            AG_CORE_ERROR("matrix nt invertible");
            return;
        }

        float invDet = 1.0f / *determinant;

        // Calculate cofactors and transpose of cofactors
        pResult->m[0] = invDet * (pMtx.m[5] * (pMtx.m[10] * pMtx.m[15] - pMtx.m[14] * pMtx.m[11]) -
            pMtx.m[6] * (pMtx.m[9] * pMtx.m[15] - pMtx.m[13] * pMtx.m[11]) +
            pMtx.m[7] * (pMtx.m[9] * pMtx.m[14] - pMtx.m[13] * pMtx.m[10]));

        pResult->m[1] = -invDet * (pMtx.m[1] * (pMtx.m[10] * pMtx.m[15] - pMtx.m[14] * pMtx.m[11]) -
            pMtx.m[2] * (pMtx.m[9] * pMtx.m[15] - pMtx.m[13] * pMtx.m[11]) +
            pMtx.m[3] * (pMtx.m[9] * pMtx.m[14] - pMtx.m[13] * pMtx.m[10]));

        pResult->m[2] = invDet * (pMtx.m[1] * (pMtx.m[6] * pMtx.m[15] - pMtx.m[14] * pMtx.m[7]) -
            pMtx.m[2] * (pMtx.m[5] * pMtx.m[15] - pMtx.m[13] * pMtx.m[7]) +
            pMtx.m[3] * (pMtx.m[5] * pMtx.m[14] - pMtx.m[13] * pMtx.m[6]));

        pResult->m[3] = -invDet * (pMtx.m[1] * (pMtx.m[6] * pMtx.m[11] - pMtx.m[10] * pMtx.m[7]) -
            pMtx.m[2] * (pMtx.m[5] * pMtx.m[11] - pMtx.m[9] * pMtx.m[7]) +
            pMtx.m[3] * (pMtx.m[5] * pMtx.m[10] - pMtx.m[9] * pMtx.m[6]));

        pResult->m[4] = -invDet * (pMtx.m[4] * (pMtx.m[10] * pMtx.m[15] - pMtx.m[14] * pMtx.m[11]) -
            pMtx.m[6] * (pMtx.m[8] * pMtx.m[15] - pMtx.m[12] * pMtx.m[11]) +
            pMtx.m[7] * (pMtx.m[8] * pMtx.m[14] - pMtx.m[12] * pMtx.m[10]));

        pResult->m[5] = invDet * (pMtx.m[0] * (pMtx.m[10] * pMtx.m[15] - pMtx.m[14] * pMtx.m[11]) -
            pMtx.m[2] * (pMtx.m[8] * pMtx.m[15] - pMtx.m[12] * pMtx.m[11]) +
            pMtx.m[3] * (pMtx.m[8] * pMtx.m[14] - pMtx.m[12] * pMtx.m[10]));

        pResult->m[6] = -invDet * (pMtx.m[0] * (pMtx.m[6] * pMtx.m[15] - pMtx.m[14] * pMtx.m[7]) -
            pMtx.m[2] * (pMtx.m[4] * pMtx.m[15] - pMtx.m[12] * pMtx.m[7]) +
            pMtx.m[3] * (pMtx.m[4] * pMtx.m[14] - pMtx.m[12] * pMtx.m[6]));

        pResult->m[7] = invDet * (pMtx.m[0] * (pMtx.m[6] * pMtx.m[11] - pMtx.m[10] * pMtx.m[7]) -
            pMtx.m[2] * (pMtx.m[4] * pMtx.m[11] - pMtx.m[8] * pMtx.m[7]) +
            pMtx.m[3] * (pMtx.m[4] * pMtx.m[10] - pMtx.m[8] * pMtx.m[6]));

        pResult->m[8] = invDet * (pMtx.m[4] * (pMtx.m[9] * pMtx.m[15] - pMtx.m[13] * pMtx.m[11]) -
            pMtx.m[5] * (pMtx.m[8] * pMtx.m[15] - pMtx.m[12] * pMtx.m[11]) +
            pMtx.m[7] * (pMtx.m[8] * pMtx.m[13] - pMtx.m[12] * pMtx.m[9]));

        pResult->m[9] = -invDet * (pMtx.m[0] * (pMtx.m[9] * pMtx.m[15] - pMtx.m[13] * pMtx.m[11]) -
            pMtx.m[1] * (pMtx.m[8] * pMtx.m[15] - pMtx.m[12] * pMtx.m[11]) +
            pMtx.m[3] * (pMtx.m[8] * pMtx.m[13] - pMtx.m[12] * pMtx.m[9]));

        pResult->m[10] = invDet * (pMtx.m[0] * (pMtx.m[5] * pMtx.m[15] - pMtx.m[13] * pMtx.m[7]) -
            pMtx.m[1] * (pMtx.m[4] * pMtx.m[15] - pMtx.m[12] * pMtx.m[7]) +
            pMtx.m[3] * (pMtx.m[4] * pMtx.m[13] - pMtx.m[12] * pMtx.m[5]));

        pResult->m[11] = -invDet * (pMtx.m[0] * (pMtx.m[5] * pMtx.m[11] - pMtx.m[9] * pMtx.m[7]) -
            pMtx.m[1] * (pMtx.m[4] * pMtx.m[11] - pMtx.m[8] * pMtx.m[7]) +
            pMtx.m[3] * (pMtx.m[4] * pMtx.m[9] - pMtx.m[8] * pMtx.m[5]));

        pResult->m[12] = -invDet * (pMtx.m[4] * (pMtx.m[9] * pMtx.m[14] - pMtx.m[13] * pMtx.m[10]) -
            pMtx.m[5] * (pMtx.m[8] * pMtx.m[14] - pMtx.m[12] * pMtx.m[10]) +
            pMtx.m[6] * (pMtx.m[8] * pMtx.m[13] - pMtx.m[12] * pMtx.m[9]));

        pResult->m[13] = invDet * (pMtx.m[0] * (pMtx.m[9] * pMtx.m[14] - pMtx.m[13] * pMtx.m[10]) -
            pMtx.m[1] * (pMtx.m[8] * pMtx.m[14] - pMtx.m[12] * pMtx.m[10]) +
            pMtx.m[2] * (pMtx.m[8] * pMtx.m[13] - pMtx.m[12] * pMtx.m[9]));

        pResult->m[14] = -invDet * (pMtx.m[0] * (pMtx.m[5] * pMtx.m[14] - pMtx.m[13] * pMtx.m[6]) -
            pMtx.m[1] * (pMtx.m[4] * pMtx.m[14] - pMtx.m[12] * pMtx.m[6]) +
            pMtx.m[2] * (pMtx.m[4] * pMtx.m[13] - pMtx.m[12] * pMtx.m[5]));

        pResult->m[15] = invDet * (pMtx.m[0] * (pMtx.m[5] * pMtx.m[10] - pMtx.m[9] * pMtx.m[6]) -
            pMtx.m[1] * (pMtx.m[4] * pMtx.m[10] - pMtx.m[8] * pMtx.m[6]) +
            pMtx.m[2] * (pMtx.m[4] * pMtx.m[9] - pMtx.m[8] * pMtx.m[5]));

        // to remove '-' from zero
        for (int i = 0; i < 16; ++i) {
            if (pResult->m[i] == -0.0f) {
                pResult->m[i] = 0.0f;
            }
        }
    }
}
