#include "gtest/gtest.h" // Google Test framework
#include "Vec.hpp"       // Your Vec.hpp header

template <int n, typename T>
struct VecTestBase : public ::testing::Test {
    // You can define common data or setup here if needed
};

// --- Test Vec Constructors and Accessors ---
// Test vec<2>
TEST(Vec2Test, ConstructorsAndAccessors) {
    vec2 v1; // Default constructor
    ASSERT_EQ(v1.x, 0.0);
    ASSERT_EQ(v1.y, 0.0);
    ASSERT_EQ(v1[0], 0.0);
    ASSERT_EQ(v1[1], 0.0);

    vec2 v2(1.0, 2.0); // Custom constructor
    ASSERT_EQ(v2.x, 1.0);
    ASSERT_EQ(v2.y, 2.0);
    ASSERT_EQ(v2[0], 1.0);
    ASSERT_EQ(v2[1], 2.0);

    vec2 v3 = v2; // Copy constructor
    ASSERT_EQ(v3.x, 1.0);
    ASSERT_EQ(v3.y, 2.0);

    // Test modification via operator[]
    v1[0] = 5.0;
    v1[1] = 10.0;
    ASSERT_EQ(v1.x, 5.0);
    ASSERT_EQ(v1.y, 10.0);
}

// Test vec<3>
TEST(Vec3Test, ConstructorsAndAccessors) {
    vec3 v1; // Default constructor
    ASSERT_EQ(v1.x, 0.0);
    ASSERT_EQ(v1.y, 0.0);
    ASSERT_EQ(v1.z, 0.0);
    ASSERT_EQ(v1[0], 0.0);
    ASSERT_EQ(v1[1], 0.0);
    ASSERT_EQ(v1[2], 0.0);

    vec3 v2(1.0, 2.0, 3.0); // Custom constructor
    ASSERT_EQ(v2.x, 1.0);
    ASSERT_EQ(v2.y, 2.0);
    ASSERT_EQ(v2.z, 3.0);
    ASSERT_EQ(v2[0], 1.0);
    ASSERT_EQ(v2[1], 2.0);
    ASSERT_EQ(v2[2], 3.0);

    vec3 v3 = v2; // Copy constructor
    ASSERT_EQ(v3.x, 1.0);
    ASSERT_EQ(v3.y, 2.0);
    ASSERT_EQ(v3.z, 3.0);

    // Test modification via operator[]
    v1[0] = 5.0;
    v1[1] = 10.0;
    v1[2] = 15.0;
    ASSERT_EQ(v1.x, 5.0);
    ASSERT_EQ(v1.y, 10.0);
    ASSERT_EQ(v1.z, 15.0);
}

// Test vec<4>
TEST(Vec4Test, ConstructorsAndAccessors) {
    vec4 v1; // Default constructor
    ASSERT_EQ(v1.x, 0.0);
    ASSERT_EQ(v1.y, 0.0);
    ASSERT_EQ(v1.z, 0.0);
    ASSERT_EQ(v1.w, 0.0);

    vec4 v2(1.0, 2.0, 3.0, 4.0); // Custom constructor
    ASSERT_EQ(v2.x, 1.0);
    ASSERT_EQ(v2.y, 2.0);
    ASSERT_EQ(v2.z, 3.0);
    ASSERT_EQ(v2.w, 4.0);

}

// Test generic vec template and initializer list constructor
TEST(VecGenericTest, InitializerListAndScalarConstructor) {
    vec<5> v1(7.0); // Scalar constructor
    for (int i = 0; i < 5; ++i) {
        ASSERT_EQ(v1[i], 7.0);
    }

    vec<3> v2 = {1.0, 2.0, 3.0}; // Initializer list
    ASSERT_EQ(v2[0], 1.0);
    ASSERT_EQ(v2[1], 2.0);
    ASSERT_EQ(v2[2], 3.0);

    vec<2, int> v2i = {10, 20}; // Initializer list with int type
    ASSERT_EQ(v2i[0], 10);
    ASSERT_EQ(v2i[1], 20);
}


// --- Test Vector Operators ---
TEST(VecOperatorsTest, Addition) {
    vec3 v1(1.0, 2.0, 3.0);
    vec3 v2(4.0, 5.0, 6.0);
    vec3 sum = v1 + v2;
    ASSERT_EQ(sum.x, 5.0);
    ASSERT_EQ(sum.y, 7.0);
    ASSERT_EQ(sum.z, 9.0);
}

TEST(VecOperatorsTest, Subtraction) {
    vec3 v1(5.0, 7.0, 9.0);
    vec3 v2(1.0, 2.0, 3.0);
    vec3 diff = v1 - v2;
    ASSERT_EQ(diff.x, 4.0);
    ASSERT_EQ(diff.y, 5.0);
    ASSERT_EQ(diff.z, 6.0);
}

TEST(VecOperatorsTest, ScalarMultiplicationVecScalar) {
    vec3 v(1.0, 2.0, 3.0);
    double scalar = 2.0;
    vec3 res = v * scalar;
    ASSERT_EQ(res.x, 2.0);
    ASSERT_EQ(res.y, 4.0);
    ASSERT_EQ(res.z, 6.0);
}

TEST(VecOperatorsTest, ScalarMultiplicationScalarVec) {
    vec3 v(1.0, 2.0, 3.0);
    double scalar = 2.0;
    vec3 res = scalar * v;
    ASSERT_EQ(res.x, 2.0);
    ASSERT_EQ(res.y, 4.0);
    ASSERT_EQ(res.z, 6.0);
}

TEST(VecOperatorsTest, Division) {
    vec3 v(6.0, 8.0, 10.0);
    double scalar = 2.0;
    vec3 res = v / scalar;
    ASSERT_EQ(res.x, 3.0);
    ASSERT_EQ(res.y, 4.0);
    ASSERT_EQ(res.z, 5.0);
}

TEST(VecOperatorsTest, DotProduct) {
    vec3 v1(1.0, 2.0, 3.0);
    vec3 v2(4.0, 5.0, 6.0);
    double dot = v1 * v2; // Using operator* for dot product
    // (1*4) + (2*5) + (3*6) = 4 + 10 + 18 = 32
    ASSERT_EQ(dot, 32.0);

    vec2 u1(1.0, 2.0);
    vec2 u2(3.0, 4.0);
    double dot2 = u1 * u2;
    // (1*3) + (2*4) = 3 + 8 = 11
    ASSERT_EQ(dot2, 11.0);
}


// --- Test Norm and Normalized ---
TEST(VecMathTest, Norm) {
    vec3 v(3.0, 4.0, 0.0);
    ASSERT_EQ(norm(v), 5.0); // sqrt(9+16+0) = 5

    vec3 v_zero(0.0, 0.0, 0.0);
    ASSERT_EQ(norm(v_zero), 0.0);
}

TEST(VecMathTest, Normalized) {
    vec3 v(3.0, 4.0, 0.0);
    vec3 n_v = normalized(v);
    ASSERT_NEAR(n_v.x, 3.0 / 5.0, 1e-9); // Use ASSERT_NEAR for floating point
    ASSERT_NEAR(n_v.y, 4.0 / 5.0, 1e-9);
    ASSERT_NEAR(n_v.z, 0.0, 1e-9);

    // Test norm of normalized vector is 1
    ASSERT_NEAR(norm(n_v), 1.0, 1e-9);
}

// --- Test Cross Product ---
TEST(VecMathTest, CrossProduct) {
    vec3 v1(1.0, 0.0, 0.0); // X-axis
    vec3 v2(0.0, 1.0, 0.0); // Y-axis
    vec3 cp = cross(v1, v2); // Should be Z-axis
    ASSERT_EQ(cp.x, 0.0);
    ASSERT_EQ(cp.y, 0.0);
    ASSERT_EQ(cp.z, 1.0);

    vec3 v3(0.0, 0.0, 1.0); // Z-axis
    vec3 cp2 = cross(v1, v3); // X-axis cross Z-axis = -Y-axis
    ASSERT_EQ(cp2.x, 0.0);
    ASSERT_EQ(cp2.y, -1.0);
    ASSERT_EQ(cp2.z, 0.0);
}

// --- Test Face Normal ---
TEST(VecMathTest, FaceNormal) {
    vec3 v_a(0.0, 0.0, 0.0);
    vec3 v_b(1.0, 0.0, 0.0);
    vec3 v_c(0.0, 1.0, 0.0);
    // Triangle in XY plane, CCW order from above, normal should be (0,0,1)
    vec3 normal = face_nomral(v_a, v_b, v_c);
    ASSERT_NEAR(normal.x, 0.0, 1e-9);
    ASSERT_NEAR(normal.y, 0.0, 1e-9);
    ASSERT_NEAR(normal.z, 1.0, 1e-9);

    vec3 v_d(0.0, 0.0, 0.0);
    vec3 v_e(0.0, 1.0, 0.0);
    vec3 v_f(1.0, 0.0, 0.0);
    // Same triangle, but CW order, normal should be (0,0,-1)
    vec3 normal_rev = face_nomral(v_d, v_e, v_f);
    ASSERT_NEAR(normal_rev.x, 0.0, 1e-9);
    ASSERT_NEAR(normal_rev.y, 0.0, 1e-9);
    ASSERT_NEAR(normal_rev.z, -1.0, 1e-9);
}
/// --- Test converting vec to new size ---
TEST(VecTest, ConvertSize){
    vec<3,int> v = {1,2,3};
    vec<2,int> v2 = convert_to_size<2>(v);
    ASSERT_EQ(v2.x, 1);
    ASSERT_EQ(v2.y, 2);

    vec<3,int> v3 = convert_to_size<3>(v);
    ASSERT_EQ(v3.x, 1);
    ASSERT_EQ(v3.y, 2);
    ASSERT_EQ(v3.z, 3);

    vec<4,int> v4 = convert_to_size<4>(v);
    ASSERT_EQ(v4.x, 1);
    ASSERT_EQ(v4.y, 2);
    ASSERT_EQ(v4.z, 3);
    ASSERT_EQ(v4.w, 0);
}

// --- Test Matrix Operations ---
// Note: Matrix tests can get extensive. This is a basic example.

TEST(MatrixTest, IdentityMatrix) {
    mat<3,3> I = mat<3,3>::identity();
    ASSERT_EQ(I[0][0], 1.0);
    ASSERT_EQ(I[1][1], 1.0);
    ASSERT_EQ(I[2][2], 1.0);
    ASSERT_EQ(I[0][1], 0.0); // Check off-diagonal
    ASSERT_EQ(I[1][0], 0.0);
}

TEST(MatrixTest, MatrixVectorMultiplication) {
    mat<3,3> m = mat<3,3>::identity();
    m[0][0] = 2.0; // Scale X
    vec3 v(1.0, 2.0, 3.0);
    vec3 res = m * v; // (2,0,0) * (1,2,3) = 2, (0,1,0)*(1,2,3) = 2, (0,0,1)*(1,2,3) = 3
                      // Incorrect interpretation above. It's row-major * column vector
                      // First row (2,0,0) * (1,2,3) = 2*1 + 0*2 + 0*3 = 2
                      // Second row (0,1,0) * (1,2,3) = 0*1 + 1*2 + 0*3 = 2
                      // Third row (0,0,1) * (1,2,3) = 0*1 + 0*2 + 1*3 = 3
    ASSERT_EQ(res.x, 2.0);
    ASSERT_EQ(res.y, 2.0);
    ASSERT_EQ(res.z, 3.0);

    // Test row vector * matrix
    vec3 v_row(1.0, 2.0, 3.0);
    mat<3,3> m_scale = mat<3,3>::identity();
    m_scale[1][1] = 5.0; // Scale Y
    vec3 res_row = v_row * m_scale; // Should be (1, 10, 3)
    ASSERT_EQ(res_row.x, 1.0);
    ASSERT_EQ(res_row.y, 10.0);
    ASSERT_EQ(res_row.z, 3.0);
}

TEST(MatrixTest, MatrixMatrixMultiplication) {
    mat<2,2> m1 = { vec2{1.0, 2.0}, vec2{3.0, 4.0} };
    mat<2,2> m2 = { vec2{5.0, 6.0}, vec2{7.0, 8.0} };
    mat<2,2> res = m1 * m2;

    // Expected result:
    // (1*5 + 2*7) = 5+14 = 19
    // (1*6 + 2*8) = 6+16 = 22
    // (3*5 + 4*7) = 15+28 = 43
    // (3*6 + 4*8) = 18+32 = 50
    ASSERT_EQ(res[0][0], 19.0);
    ASSERT_EQ(res[0][1], 22.0);
    ASSERT_EQ(res[1][0], 43.0);
    ASSERT_EQ(res[1][1], 50.0);
}


// --- Main for tests ---
// This is typically provided by gtest_main, so you don't need to write it.
// If your tests/CMakeLists.txt links to gtest_main, this section is implicitly
// included and you don't need `int main(int argc, char **argv) { ... }`