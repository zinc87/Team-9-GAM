#pragma once
#include "pch.h"


//AG_CORE_ERROR("apple");

namespace AG {
	class Matrix4x4 {
	public:
		float m[16]; // array of 4x4 matrix

		Matrix4x4(); // default constructor
		Matrix4x4(const float* pArr);
		Matrix4x4(float _00, float _01, float _02, float _03,
			float _10, float _11, float _12, float _13,
			float _20, float _21, float _22, float _23,
			float _30, float _31, float _32, float _33);

		Matrix4x4& operator=(const Matrix4x4& rhs);
		Matrix4x4& operator*=(const Matrix4x4& rhs);
		friend Matrix4x4 operator*(const Matrix4x4& lhs, const Matrix4x4& rhs);

		static void convertToGLM(const Matrix4x4& customMatrix, glm::mat4& glmMatrix);
		static void Mtx44Identity(Matrix4x4& pResult);
		static void Mtx44Translate(Matrix4x4& pResult, float x, float y, float z);
		static void Mtx44Scale(Matrix4x4& pResult, float x, float y, float z);
		static void Mtx44RotRad(Matrix4x4& pResult, float angle);
		static void Mtx44RotDeg(Matrix4x4& pResult, float angle);
		static void Mtx44Transpose(Matrix4x4& pResult, const Matrix4x4& pMtx);
		static void Mtx44Inverse(Matrix4x4* pResult, float* determinant, const Matrix4x4& pMtx);

	private:

	};
}
