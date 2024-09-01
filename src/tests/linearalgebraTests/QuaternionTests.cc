#include <cmath>
#include <doctest.h>
#include "../testsupport.h"

#include <linearalgebra/quaternion.h>

namespace {
    typedef linearalgebra::Vector3<double> Vector3;
    typedef linearalgebra::Matrix4<double> Matrix4;
    typedef linearalgebra::Quaternion<double> Quaternion;
}

TEST_CASE("Quaternion construct and initialize [quaternion]") {
    SUBCASE("implicit constructor") {
        Quaternion q;
        REQUIRE( q.x == 0.0 );
        REQUIRE( q.y == 0.0 );
        REQUIRE( q.z == 0.0 );
        REQUIRE( q.w == 1.0 );
    }
    
    SUBCASE("args constructor") {
        Quaternion q(1.0, 2.0, 3.0, 4.0);
        REQUIRE( q.x == 1.0 );
        REQUIRE( q.y == 2.0 );
        REQUIRE( q.z == 3.0 );
        REQUIRE( q.w == 4.0 );
    }
    
    SUBCASE("set values") {
        Quaternion q;
        q.set(1.0, 2.0, 3.0, 4.0);
        REQUIRE( q.x == 1.0 );
        REQUIRE( q.y == 2.0 );
        REQUIRE( q.z == 3.0 );
        REQUIRE( q.w == 4.0 );
    }
    
    SUBCASE("set array values") {
        Quaternion q;
        double v[4] = {1.0, 2.0, 3.0, 4.0};
        q.set(v);
        REQUIRE( q.x == 1.0 );
        REQUIRE( q.y == 2.0 );
        REQUIRE( q.z == 3.0 );
        REQUIRE( q.w == 4.0 );
    }
}

TEST_CASE("Quaternion norm test [quaternion]") {
    Quaternion q(1.0, 2.0, 3.0, 4.0);
    double n = q.norm();
    REQUIRE(n == doctest::Approx(sqrt(30.0)).epsilon(kTestEPS));
}

TEST_CASE("Quaternion normalize test [quaternion]") {
    Quaternion q(1.0, 2.0, 3.0, 4.0);
    q.normalize();
    REQUIRE(q.norm() == doctest::Approx(sqrt(1.0)).epsilon(kTestEPS));
}

TEST_CASE("Quaternion inverse test [quaternion]") {
    Quaternion q(1.0, 2.0, 3.0, 4.0);
    Quaternion qi = Quaternion::inversed(q);
    q = q * qi;
    REQUIRE(q.norm() == doctest::Approx(sqrt(1.0)).epsilon(kTestEPS));
}

TEST_CASE("Quaternion conjugate test [quaternion]") {
    Quaternion q(1.0, 2.0, 3.0, 4.0);
    q.conjugate();
    REQUIRE(q.x == -1.0);
    REQUIRE(q.y == -2.0);
    REQUIRE(q.z == -3.0);
    REQUIRE(q.w == 4.0);
}

TEST_CASE("Quaternion rotation test [quaternion]") {
    SUBCASE("y 0 radian") {
        Quaternion q = Quaternion::makeRotation(0.0, 0.0, 1.0, 0.0);
        Vector3 p(2.0, 0.0, 0.0);
        Vector3 r = q.rotate(p);
        REQUIRE(r.x == doctest::Approx(2.0).epsilon(kTestEPS));
        REQUIRE(r.y == doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE(r.z == doctest::Approx(0.0).epsilon(kTestEPS));
    }
    
    SUBCASE("y 45 radian") {
        Quaternion q = Quaternion::makeRotation(M_PI * 0.25, 0.0, 1.0, 0.0);
        Vector3 p(2.0, 0.0, 0.0);
        Vector3 r = q.rotate(p);
        REQUIRE(r.x == doctest::Approx(2.0 / sqrt(2.0)).epsilon(kTestEPS));
        REQUIRE(r.y == doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE(r.z == doctest::Approx(-2.0 / sqrt(2.0)).epsilon(kTestEPS));
    }
    
    SUBCASE("y 90 radian") {
        Quaternion q = Quaternion::makeRotation(M_PI * 0.5, 0.0, 1.0, 0.0);
        Vector3 p(2.0, 0.0, 0.0);
        Vector3 r = q.rotate(p);
        REQUIRE(r.x == doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE(r.y == doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE(r.z == doctest::Approx(-2.0).epsilon(kTestEPS));
    }
    
    SUBCASE("x 0 radian") {
        Quaternion q = Quaternion::makeRotation(0.0, 1.0, 0.0, 0.0);
        Vector3 p(0.0, 0.0, 2.0);
        Vector3 r = q.rotate(p);
        REQUIRE(r.x == doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE(r.y == doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE(r.z == doctest::Approx(2.0).epsilon(kTestEPS));
    }
    
    SUBCASE("x 45 radian") {
        Quaternion q = Quaternion::makeRotation(M_PI * 0.25, 1.0, 0.0, 0.0);
        Vector3 p(0.0, 0.0, 2.0);
        Vector3 r = q.rotate(p);
        REQUIRE(r.x == doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE(r.y == doctest::Approx(-2.0 / sqrt(2.0)).epsilon(kTestEPS));
        REQUIRE(r.z == doctest::Approx(2.0 / sqrt(2.0)).epsilon(kTestEPS));
    }
    
    SUBCASE("x 90 radian") {
        Quaternion q = Quaternion::makeRotation(M_PI * 0.5, 1.0, 0.0, 0.0);
        Vector3 p(0.0, 0.0, 2.0);
        Vector3 r = q.rotate(p);
        REQUIRE(r.x == doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE(r.y == doctest::Approx(-2.0).epsilon(kTestEPS));
        REQUIRE(r.z == doctest::Approx(0.0).epsilon(kTestEPS));
    }
}

TEST_CASE("Quaternion matrix test [quaternion]") {
    SUBCASE("y 0 radian") {
        Quaternion q = Quaternion::makeRotation(0.0, 0.0, 1.0, 0.0);
        Vector3 p(2.0, 0.0, 0.0);
        Vector3 rq = q.rotate(p);
        Matrix4 m = q.getMatrix();
        Vector3 rm = Matrix4::transformV3(m, p);
        REQUIRE(rq.x == doctest::Approx(rm.x).epsilon(kTestEPS));
        REQUIRE(rq.y == doctest::Approx(rm.y).epsilon(kTestEPS));
        REQUIRE(rq.z == doctest::Approx(rm.z).epsilon(kTestEPS));
    }

    SUBCASE("y 45 radian") {
        Quaternion q = Quaternion::makeRotation(M_PI * 0.25, 0.0, 1.0, 0.0);
        Vector3 p(2.0, 0.0, 0.0);
        Vector3 rq = q.rotate(p);
        Matrix4 m = q.getMatrix();
        Vector3 rm = Matrix4::transformV3(m, p);
        REQUIRE(rq.x == doctest::Approx(rm.x).epsilon(kTestEPS));
        REQUIRE(rq.y == doctest::Approx(rm.y).epsilon(kTestEPS));
        REQUIRE(rq.z == doctest::Approx(rm.z).epsilon(kTestEPS));
    }

    SUBCASE("y 90 radian") {
        Quaternion q = Quaternion::makeRotation(M_PI * 0.5, 0.0, 1.0, 0.0);
        Vector3 p(2.0, 0.0, 0.0);
        Vector3 rq = q.rotate(p);
        Matrix4 m = q.getMatrix();
        Vector3 rm = Matrix4::transformV3(m, p);
        REQUIRE(rq.x == doctest::Approx(rm.x).epsilon(kTestEPS));
        REQUIRE(rq.y == doctest::Approx(rm.y).epsilon(kTestEPS));
        REQUIRE(rq.z == doctest::Approx(rm.z).epsilon(kTestEPS));
    }

    SUBCASE("x 0 radian") {
        Quaternion q = Quaternion::makeRotation(0.0, 1.0, 0.0, 0.0);
        Vector3 p(0.0, 0.0, 2.0);
        Vector3 rq = q.rotate(p);
        Matrix4 m = q.getMatrix();
        Vector3 rm = Matrix4::transformV3(m, p);
        REQUIRE(rq.x == doctest::Approx(rm.x).epsilon(kTestEPS));
        REQUIRE(rq.y == doctest::Approx(rm.y).epsilon(kTestEPS));
        REQUIRE(rq.z == doctest::Approx(rm.z).epsilon(kTestEPS));
    }

    SUBCASE("x 45 radian") {
        Quaternion q = Quaternion::makeRotation(M_PI * 0.25, 1.0, 0.0, 0.0);
        Vector3 p(0.0, 0.0, 2.0);
        Vector3 rq = q.rotate(p);
        Matrix4 m = q.getMatrix();
        Vector3 rm = Matrix4::transformV3(m, p);
        REQUIRE(rq.x == doctest::Approx(rm.x).epsilon(kTestEPS));
        REQUIRE(rq.y == doctest::Approx(rm.y).epsilon(kTestEPS));
        REQUIRE(rq.z == doctest::Approx(rm.z).epsilon(kTestEPS));
    }

    SUBCASE("x 90 radian") {
        Quaternion q = Quaternion::makeRotation(M_PI * 0.5, 1.0, 0.0, 0.0);
        Vector3 p(0.0, 0.0, 2.0);
        Vector3 rq = q.rotate(p);
        Matrix4 m = q.getMatrix();
        Vector3 rm = Matrix4::transformV3(m, p);
        REQUIRE(rq.x == doctest::Approx(rm.x).epsilon(kTestEPS));
        REQUIRE(rq.y == doctest::Approx(rm.y).epsilon(kTestEPS));
        REQUIRE(rq.z == doctest::Approx(rm.z).epsilon(kTestEPS));
    }

    SUBCASE("free rotation") {
        Quaternion q = Quaternion::makeRotation(M_PI * 0.5, 0.577, 0.577, 0.577);
        Vector3 p(0.0, 0.0, 2.0);
        Vector3 rq = q.rotate(p);
        Matrix4 m = q.getMatrix();
        Vector3 rm = Matrix4::transformV3(m, p);
        REQUIRE(rq.x == doctest::Approx(rm.x).epsilon(kTestEPS));
        REQUIRE(rq.y == doctest::Approx(rm.y).epsilon(kTestEPS));
        REQUIRE(rq.z == doctest::Approx(rm.z).epsilon(kTestEPS));
    }
}

TEST_CASE("Quaternion lerp test [quaternion]") {
    Quaternion q0(1.0, 2.0, 3.0, 4.0);
    Quaternion q1(5.0, 6.0, 7.0, 8.0);
    Quaternion q;
    
    const double N = 8.0;
    for (double i = 0.0; i <= N; i+= 1.0) {
        double t = i / N;
        q = Quaternion::lerp(q0, q1, t);
        REQUIRE(q.x == doctest::Approx(q0.x * (1.0 - t) + q1.x * t).epsilon(kTestEPS));
        REQUIRE(q.y == doctest::Approx(q0.y * (1.0 - t) + q1.y * t).epsilon(kTestEPS));
        REQUIRE(q.z == doctest::Approx(q0.z * (1.0 - t) + q1.z * t).epsilon(kTestEPS));
        REQUIRE(q.w == doctest::Approx(q0.w * (1.0 - t) + q1.w * t).epsilon(kTestEPS));
    }
}

TEST_CASE("Quaternion slerp test [quaternion]") {
    Quaternion q0 = Quaternion::makeRotation(0.0, 1.0, 0.0, 0.0);
    Quaternion q1 = Quaternion::makeRotation(M_PI * -0.5, 1.0, 0.0, 0.0);
    Quaternion q;
    Vector3 v(0.0, 0.0, 2.0);

    const double N = 8.0;
    for (double i = 0.0; i <= N; i+= 1.0) {
        double t = i / N;
        q = Quaternion::slerp(q0, q1, t);
        REQUIRE(q.norm() == doctest::Approx(1.0).epsilon(kTestEPS));
        
        Vector3 r = q.rotate(v);
        double c = cos(M_PI * -0.5 * t);
        double s = -sin(M_PI * -0.5 * t);
        REQUIRE(r.x == doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE(r.y == doctest::Approx(s * v.z).epsilon(kTestEPS));
        REQUIRE(r.z == doctest::Approx(c * v.z).epsilon(kTestEPS));
    }
}

TEST_CASE("Quaternion operator test [quaternion]") {
    SUBCASE("add") {
        Quaternion q0(1.0, 2.0, 3.0, 4.0);
        Quaternion q1(-1.0, -2.0, -3.0, -4.0);
        Quaternion q = q0 + q1;
        REQUIRE(q.x == 0.0);
        REQUIRE(q.y == 0.0);
        REQUIRE(q.z == 0.0);
        REQUIRE(q.w == 0.0);
    }

    SUBCASE("sub") {
        Quaternion q0(1.0, 2.0, 3.0, 4.0);
        Quaternion q1(1.0, 2.0, 3.0, 4.0);
        Quaternion q = q0 - q1;
        REQUIRE(q.x == 0.0);
        REQUIRE(q.y == 0.0);
        REQUIRE(q.z == 0.0);
        REQUIRE(q.w == 0.0);
    }

    SUBCASE("mul scalar") {
        Quaternion q0(1.0, 2.0, 3.0, 4.0);
        Quaternion q = q0 * 2.0;
        REQUIRE(q.x == 2.0);
        REQUIRE(q.y == 4.0);
        REQUIRE(q.z == 6.0);
        REQUIRE(q.w == 8.0);
    }

    SUBCASE("div scalar") {
        Quaternion q0(2.0, 4.0, 6.0, 8.0);
        Quaternion q = q0 / 2.0;
        REQUIRE(q.x == 1.0);
        REQUIRE(q.y == 2.0);
        REQUIRE(q.z == 3.0);
        REQUIRE(q.w == 4.0);
    }

    SUBCASE("mul quaternion") {
        Quaternion q0(1.0, 2.0, 3.0, 4.0);
        Quaternion q1(5.0, 6.0, 7.0, 8.0);
        Quaternion q = q0 * q1;
        double s0 = q0.w;
        double s1 = q1.w;
        Vector3 v0(q0.x, q0.y, q0.z);
        Vector3 v1(q1.x, q1.y, q1.z);
        Vector3 v = s0 * v1 + s1 * v0 + Vector3::cross(v0, v1);
        REQUIRE(q.x == v.x);
        REQUIRE(q.y == v.y);
        REQUIRE(q.z == v.z);
        REQUIRE(q.w == s0 * s1 - Vector3::dot(v0, v1));
    }
}

