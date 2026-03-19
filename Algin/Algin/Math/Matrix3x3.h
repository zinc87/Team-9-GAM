#pragma once
#include "pch.h"

namespace AG {
	class Matrix3x3 {
	public:
		float m[9];     // 1D array
		float m2[3][3]; // 2D array

		Matrix3x3();
		Matrix3x3(const float* pArr);
		Matrix3x3(float _00, float _01, float _02,
			float _10, float _11, float _12,
			float _20, float _21, float _22);

		Matrix3x3& operator=(const Matrix3x3& rhs);
		Matrix3x3& operator*=(const Matrix3x3& rhs);
		friend Matrix3x3 operator*(const Matrix3x3& lhs, const Matrix3x3& rhs);

		static void convertToGLM(const Matrix3x3& customMatrix, glm::mat3& glmMatrix);
		static void Mtx33Identity(Matrix3x3& pResult);
		static void Mtx33Translate(Matrix3x3& pResult, float x, float y);
		static void Mtx33Scale(Matrix3x3& pResult, float x, float y);
		static void Mtx33RotRad(Matrix3x3& pResult, float angle);
		static void Mtx33RotDeg(Matrix3x3& pResult, float angle);
		static void Mtx33Transpose(Matrix3x3& pResult, const Matrix3x3& pMtx);
		static void Mtx33Inverse(Matrix3x3* pResult, float* determinant, const Matrix3x3& pMtx);

	private:

	};
}
