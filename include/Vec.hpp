#pragma once
#include <cmath>
#include <cassert>
#include <iostream>

template<int n,typename T=double> struct vec {
   static_assert(n>0);
   static_assert(std::is_arithmetic<T>::value);
   vec() = default;
   explicit vec(T v) { for(int i=0;i<n;i++) data[i]=v; }
   T& operator[](const int i)       { assert(i>=0 && i<n); return data[i]; }
   T  operator[](const int i) const { assert(i>=0 && i<n); return data[i]; }
   vec(std::initializer_list<T> list){
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
   vec() = default;
   vec(T x, T y) : x(x), y(y) {}
   vec(const vec<2>& v) : x(v.x), y(v.y) {}
   T& operator[](const int i)       { assert(i>=0 && i<2); return i ? y : x; }
   T  operator[](const int i) const { assert(i>=0 && i<2); return i ? y : x; }

};

template<typename T> struct vec<3,T> {
   T x = 0, y = 0, z = 0;
   vec() = default;
   vec(T x, T y, T z) : x(x), y(y), z(z) {}
   vec(const vec<3>& v) : x(v.x), y(v.y), z(v.z) {}
   T& operator[](const int i)       { assert(i>=0 && i<3); return i ? (1==i ? y : z) : x; }
   T  operator[](const int i) const { assert(i>=0 && i<3); return i ? (1==i ? y : z) : x; }
};

template<typename T> struct vec<4,T> {
   T x = 0, y = 0, z = 0, w = 0;
   vec() = default;
   vec(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
   vec(const vec<4>& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
   T& operator[](const int i)       { assert(i>=0 && i<4); return i<2 ? (i ? y : x) : (2==i ? z : w); }
   T  operator[](const int i) const { assert(i>=0 && i<4); return i<2 ? (i ? y : x) : (2==i ? z : w); }
   vec<2> xy()  const { return {x, y};    }
   vec<3> xyz() const { return {x, y, z}; }
};

// Aliases

typedef vec<2> vec2;
typedef vec<3> vec3;
typedef vec<4> vec4;
typedef vec<2,int> vec2i;
typedef vec<3,int> vec3i;
typedef vec<4,int> vec4i;

// Operators

template<int n,typename T> double operator*(const vec<n,T>& lhs, const vec<n,T>& rhs) {
   T res = 0;
   for(int i=0; i<n; i++) res+=lhs[i]*rhs[i];
   return res;
}

template<int n,typename T> vec<n,T> operator+(const vec<n,T>& lhs, const vec<n,T>& rhs) {
   vec<n,T> res = lhs;
   for(int i=0;i<n;i++) res[i]+=rhs[i];
   return res;
}

template<int n,typename T> vec<n,T> operator-(const vec<n,T>& lhs, const vec<n,T>& rhs) {
   vec<n,T> res = lhs;
   for(int i=0;i<n;i++) res[i]-=rhs[i];
   return res;
}
template<int n,typename T> vec<n,T> operator*(const vec<n,T>& lhs, const T& rhs) {
   vec<n,T> res = lhs;
   for(int i=0;i<n;i++) res[i]*=rhs;
   return res;
}

template<int n,typename T> vec<n,T> operator*(const T& lhs, const vec<n,T> &rhs) {
   return rhs * lhs;
}

template<int n,typename T> vec<n,T> operator/(const vec<n,T>& lhs, const T& rhs) {
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




template<int n> double norm(const vec<n>& v) {
   return std::sqrt(v*v);
}

template<int n> vec<n> normalized(const vec<n>& v) {
   return v / norm(v);
}

inline vec3 cross(const vec3 &v1, const vec3 &v2) {
   return {v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x};
}

template<int n> struct dt;

template<int nrows,int ncols> struct mat {
   vec<ncols> rows[nrows] = {{}};

         vec<ncols>& operator[] (const int idx)       { assert(idx>=0 && idx<nrows); return rows[idx]; }
   const vec<ncols>& operator[] (const int idx) const { assert(idx>=0 && idx<nrows); return rows[idx]; }

   static mat<nrows,ncols> identity() {
      mat<nrows,ncols> res;
      for (int i=nrows; i--; res[i][i]=1);
      return res;
   }

   double det() const {
      return dt<ncols>::det(*this);
   }
   double cofactor(const int row, const int col) const {
      mat<nrows-1,ncols-1> submatrix;
      for (int i=nrows-1; i--; )
         for (int j=ncols-1;j--; submatrix[i][j]=rows[i+int(i>=row)][j+int(j>=col)]);
      return submatrix.det() * ((row+col)%2 ? -1 : 1);
   }

   mat<nrows,ncols> invert_transpose() const {
      mat<nrows,ncols> adjugate_transpose; // transpose to ease determinant computation, check the last line
      for (int i=nrows; i--; )
         for (int j=ncols; j--; adjugate_transpose[i][j]=cofactor(i,j));
      return adjugate_transpose/(adjugate_transpose[0]*rows[0]);
   }

   mat<nrows,ncols> invert() const {
      return invert_transpose().transpose();
   }

   mat<ncols,nrows> transpose() const {
      mat<ncols,nrows> res;
      for (int i=ncols; i--; )
         for (int j=nrows; j--; res[i][j]=rows[j][i]);
      return res;
   }
};

template<int nrows,int ncols> vec<ncols> operator*(const vec<nrows>& lhs, const mat<nrows,ncols>& rhs) {
   return (mat<1,nrows>{{lhs}}*rhs)[0];
}

template<int nrows,int ncols> vec<nrows> operator*(const mat<nrows,ncols>& lhs, const vec<ncols>& rhs) {
   vec<nrows> res;
   for (int i=nrows; i--; res[i]=lhs[i]*rhs);
   return res;
}

template<int R1,int C1,int C2>mat<R1,C2> operator*(const mat<R1,C1>& lhs, const mat<C1,C2>& rhs) {
   mat<R1,C2> result;
   for (int i=R1; i--; )
      for (int j=C2; j--; )
         for (int k=C1; k--; result[i][j]+=lhs[i][k]*rhs[k][j]);
   return result;
}

template<int nrows,int ncols>mat<nrows,ncols> operator*(const mat<nrows,ncols>& lhs, const double& val) {
   mat<nrows,ncols> result;
   for (int i=nrows; i--; result[i] = lhs[i]*val);
   return result;
}

template<int nrows,int ncols>mat<nrows,ncols> operator/(const mat<nrows,ncols>& lhs, const double& val) {
   mat<nrows,ncols> result;
   for (int i=nrows; i--; result[i] = lhs[i]/val);
   return result;
}

template<int nrows,int ncols>mat<nrows,ncols> operator+(const mat<nrows,ncols>& lhs, const mat<nrows,ncols>& rhs) {
   mat<nrows,ncols> result;
   for (int i=nrows; i--; )
      for (int j=ncols; j--; result[i][j]=lhs[i][j]+rhs[i][j]);
   return result;
}

template<int nrows,int ncols>mat<nrows,ncols> operator-(const mat<nrows,ncols>& lhs, const mat<nrows,ncols>& rhs) {
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
   static double det(const mat<n,n>& src) {
      double res = 0;
      for (int i=n; i--; res += src[0][i] * src.cofactor(0,i));
      return res;
   }
};

template<> struct dt<1> {
   static double det(const mat<1,1>& src) {
      return src[0][0];
   }
};
