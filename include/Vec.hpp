#pragma once
#include <cmath>
#include <cassert>
#include <iostream>
#include <cuda_runtime.h>

template<int n,typename T=double> struct vec {
   static_assert(n>0);
   static_assert(std::is_arithmetic<T>::value);
   __host__ __device__ vec() = default;
   __host__ __device__ vec(const vec<n>& v) { for(int i=0;i<n;i++) data[i]=v[i]; }
   __host__ __device__ explicit vec(T v) { for(int i=0;i<n;i++) data[i]=v; }
   __host__ __device__ T& operator[](const int i)       { assert(i>=0 && i<n); return data[i]; }
   __host__ __device__ T  operator[](const int i) const { assert(i>=0 && i<n); return data[i]; }
   __host__ __device__ vec(std::initializer_list<T> list){
      static_assert(n==list.size());
      int i=0;
      for(T v: list)
      data[i++]=v;
   }
   private:
   T data[n] = {0};
};

template<typename T> struct vec<2,T> {
   T x = 0, y = 0;
   __host__ __device__ vec() = default;
   __host__ __device__ vec(T x, T y) : x(x), y(y) {}
   __host__ __device__ vec(const vec<2>& v) : x(v.x), y(v.y) {}
   __host__ __device__ T& operator[](const int i)       { assert(i>=0 && i<2); return i ? y : x; }
   __host__ __device__ T  operator[](const int i) const { assert(i>=0 && i<2); return i ? y : x; }

};

template<typename T> struct vec<3,T> {
   T x = 0, y = 0, z = 0;
   __host__ __device__ vec() = default;
   __host__ __device__ vec(T x, T y, T z) : x(x), y(y), z(z) {}
   __host__ __device__ vec(const vec<3>& v) : x(v.x), y(v.y), z(v.z) {}
   __host__ __device__ T& operator[](const int i)       { assert(i>=0 && i<3); return i ? (1==i ? y : z) : x; }
   __host__ __device__ T  operator[](const int i) const { assert(i>=0 && i<3); return i ? (1==i ? y : z) : x; }
};

template<typename T> struct vec<4,T> {
   T x = 0, y = 0, z = 0, w = 0;
   __host__ __device__ vec() = default;
   __host__ __device__ vec(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
   __host__ __device__ vec(const vec<4>& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
   __host__ __device__ T& operator[](const int i)       { assert(i>=0 && i<4); return i<2 ? (i ? y : x) : (2==i ? z : w); }
   __host__ __device__ T  operator[](const int i) const { assert(i>=0 && i<4); return i<2 ? (i ? y : x) : (2==i ? z : w); }
};

// Aliases

typedef vec<2> vec2;
typedef vec<3> vec3;
typedef vec<4> vec4;
typedef vec<2,int> vec2i;
typedef vec<3,int> vec3i;
typedef vec<4,int> vec4i;

// Operators

template<int n,typename T> 
__host__ __device__ double operator*(const vec<n,T>& lhs, const vec<n,T>& rhs) {
   T res = 0;
   for(int i=0; i<n; i++) res+=lhs[i]*rhs[i];
   return res;
}
template<int n,typename T> 
__host__ __device__ T dot(const vec<n,T>& lhs, const vec<n,T>& rhs) {
   return lhs*rhs;
}

template<int n,typename T> 
__host__ __device__ vec<n,T> operator+(const vec<n,T>& lhs, const vec<n,T>& rhs) {
   vec<n,T> res = lhs;
   for(int i=0;i<n;i++) res[i]+=rhs[i];
   return res;
}

template<int n,typename T> 
__host__ __device__ vec<n,T> operator-(const vec<n,T>& lhs, const vec<n,T>& rhs) {
   vec<n,T> res = lhs;
   for(int i=0;i<n;i++) res[i]-=rhs[i];
   return res;
}
template<int n,typename T,typename U> vec<n,T> 
__host__ __device__ operator*(const vec<n,T>& lhs, const U& rhs) {
   vec<n,T> res = lhs;
   for(int i=0;i<n;i++) res[i]*=rhs;
   return res;
}

template<int n,typename T,typename U> 
__host__ __device__ vec<n,T> operator*(const U& lhs, const vec<n,T> &rhs) {
   return rhs * lhs;
}

template<int n,typename T> vec<n,T> 
__host__ __device__ operator/(const vec<n,T>& lhs, const T& rhs) {
   vec<n,T> res = lhs;
   for (int i=n; i--; res[i]/=rhs);
   return res;
}

template<int n> std::ostream& operator<<(std::ostream& out, const vec<n>& v) {
   out<<"[";
   for (int i=0; i<n; i++) out << v[i] << (i<n-1 ? ", " : "");
   out<<"]";
   return out;
}

// Operations

template<int n> 
__host__ __device__ double norm(const vec<n>& v) {
   return std::sqrt(v*v);
}

template<int n> vec<n> 
__host__ __device__ normalized(const vec<n>& v) {
   double nm=norm(v);
   if(nm<1e-8) return vec<n>();
   return v / nm;
}

__host__ __device__ inline vec3 cross(const vec3 &v1, const vec3 &v2) {
   return {v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x};
}

__host__ __device__ inline vec3 face_normal(const vec3 &v1, const vec3 &v2, const vec3 &v3) {
   return normalized(cross(v2-v1, v3-v1));
}

/// @brief Converts a vector of size old_size to a vector of size new_size
/// it fills the rest of the vector with fill parameter and return vector of size new_size 
template<int new_size,int old_size,typename T>
__host__ __device__  vec<new_size,T> convert_to_size(const vec<old_size,T>& v,T fill=T{0}){
   vec<new_size,T> res;
   int common_size=(new_size<old_size?new_size:old_size);
   for(int i=0;i<common_size;i++)
      res[i]=v[i];

   for(int i=common_size;i<new_size;i++)
      res[i]=fill;
   return res;
}
// Matrices

template<int n> struct dt;

template<int nrows,int ncols> struct mat {
   vec<ncols> rows[nrows] = {{}};

   /// @brief operator[] returns a row
   __host__ __device__ vec<ncols>& operator[] (const int idx)       { assert(idx>=0 && idx<nrows); return rows[idx]; }
   /// @brief operator[] returns a row
   __host__ __device__ const vec<ncols>& operator[] (const int idx) const { assert(idx>=0 && idx<nrows); return rows[idx]; }

   __host__ __device__ static mat<nrows,ncols> identity() {
      mat<nrows,ncols> res;
      for (int i=nrows; i--; res[i][i]=1);
      return res;
   }

   __host__ __device__ mat<nrows,ncols> invert() const {
      if constexpr(nrows==ncols && nrows==4){
         mat<nrows, ncols> inv;
         float det;
         float m[16];
   
         // Copy matrix data to a flat array for easier indexing
         for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
               m[r * 4 + c] = rows[r][c];
            }
         }
   
         // Calculate the cofactor matrix
         inv[0][0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
         inv[0][1] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
         inv[0][2] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
         inv[0][3] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
         inv[1][0] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
         inv[1][1] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
         inv[1][2] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
         inv[1][3] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
         inv[2][0] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
         inv[2][1] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
         inv[2][2] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
         inv[2][3] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
         inv[3][0] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
         inv[3][1] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
         inv[3][2] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
         inv[3][3] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];
      
         // Calculate determinant
         det = m[0] * inv[0][0] + m[1] * inv[0][1] + m[2] * inv[0][2] + m[3] * inv[0][3];
   
         // If determinant is 0, matrix is not invertible
         if (det == 0) {
            // Return identity matrix or another identity
            return mat<nrows, ncols>::identity();
         }
      
         det = 1.0f / det;
      
         mat<nrows, ncols> result;
         for (int r = 0; r < 4; ++r) {
         for (int c = 0; c < 4; ++c) {
            result[r][c] = inv[r][c] * det;
         }
      }
         return result.transpose();
      }
      else if constexpr(nrows==3) {
         const mat<3,3>& A = *this;
         mat<3,3> inv;
         
         // Obliczanie wyznacznika
         float det = A[0][0]*(A[1][1]*A[2][2] - A[2][1]*A[1][2]) -
                     A[0][1]*(A[1][0]*A[2][2] - A[1][2]*A[2][0]) +
                     A[0][2]*(A[1][0]*A[2][1] - A[1][1]*A[2][0]);
         
         const float EPSILON = 1e-6f;
         if (abs(det) < EPSILON) {
            // Zwracamy macierz jednostkową, jeśli macierz jest osobliwa
            return identity();
         }

         float inv_det = 1.0f / det;
         
         inv[0][0] = (A[1][1] * A[2][2] - A[2][1] * A[1][2]) * inv_det;
         inv[0][1] = (A[0][2] * A[2][1] - A[0][1] * A[2][2]) * inv_det;
         inv[0][2] = (A[0][1] * A[1][2] - A[0][2] * A[1][1]) * inv_det;
         inv[1][0] = (A[1][2] * A[2][0] - A[1][0] * A[2][2]) * inv_det;
         inv[1][1] = (A[0][0] * A[2][2] - A[0][2] * A[2][0]) * inv_det;
         inv[1][2] = (A[0][2] * A[1][0] - A[0][0] * A[1][2]) * inv_det;
         inv[2][0] = (A[1][0] * A[2][1] - A[2][0] * A[1][1]) * inv_det;
         inv[2][1] = (A[0][1] * A[2][0] - A[0][0] * A[2][1]) * inv_det;
         inv[2][2] = (A[0][0] * A[1][1] - A[0][1] * A[1][0]) * inv_det;
         
         return inv;
      }
   }


   __host__ __device__ mat<ncols,nrows> transpose() const {
      mat<ncols,nrows> res;
      for (int i=ncols; i--; )
         for (int j=nrows; j--; res[i][j]=rows[j][i]);
      return res;
   }
};

using mat3 = mat<3,3>;
using mat4 = mat<4,4>;

template<int n> mat<n,n> once(){
   mat<n,n> res;
   for(int i=0;i<n;i++) res[i][i]=1;
   return res;
}

template<int nrows,int ncols>
__host__ __device__ vec<ncols> operator*(const vec<nrows>& lhs, const mat<nrows,ncols>& rhs) {
   return (mat<1,nrows>{{lhs}}*rhs)[0];
}

template<int nrows,int ncols>
__host__ __device__ vec<nrows> operator*(const mat<nrows,ncols>& lhs, const vec<ncols>& rhs) {
   vec<nrows> res;
   for (int i=nrows; i--; res[i]=lhs[i]*rhs);
   return res;
}

template<int R1,int C1,int C2>
__host__ __device__ mat<R1,C2>operator*(const mat<R1,C1>& lhs, const mat<C1,C2>& rhs) {
   mat<R1,C2> result;
   for (int i=R1; i--; )
      for (int j=C2; j--; )
         for (int k=C1; k--; result[i][j]+=lhs[i][k]*rhs[k][j]);
   return result;
}

template<int nrows,int ncols>
__host__ __device__ mat<nrows,ncols> operator*(const mat<nrows,ncols>& lhs, const double& val) {
   mat<nrows,ncols> result;
   for (int i=nrows; i--; result[i] = lhs[i]*val);
   return result;
}

template<int nrows,int ncols>
__host__ __device__ mat<nrows,ncols> operator/(const mat<nrows,ncols>& lhs, const double& val) {
   mat<nrows,ncols> result;
   for (int i=nrows; i--; result[i] = lhs[i]/val);
   return result;
}

template<int nrows,int ncols>
__host__ __device__ mat<nrows,ncols> operator+(const mat<nrows,ncols>& lhs, const mat<nrows,ncols>& rhs) {
   mat<nrows,ncols> result;
   for (int i=nrows; i--; )
      for (int j=ncols; j--; result[i][j]=lhs[i][j]+rhs[i][j]);
   return result;
}

template<int nrows,int ncols>mat<nrows,ncols>
__host__ __device__ operator-(const mat<nrows,ncols>& lhs, const mat<nrows,ncols>& rhs) {
   mat<nrows,ncols> result;
   for (int i=nrows; i--; )
      for (int j=ncols; j--; result[i][j]=lhs[i][j]-rhs[i][j]);
   return result;
}

template<int nrows,int ncols> std::ostream& operator<<(std::ostream& out, const mat<nrows,ncols>& m) {
   for (int i=0; i<nrows; i++) out << m[i] << std::endl;
   return out;
}

template<int n> struct dt {
   __host__ __device__ static double det(const mat<n,n>& src) {
      double res = 0;
      for (int i=n; i--; res += src[0][i] * src.cofactor(0,i));
      return res;
   }
};

template<> struct dt<1> {
   __host__ __device__ static double det(const mat<1,1>& src) {
      return src[0][0];
   }
};
