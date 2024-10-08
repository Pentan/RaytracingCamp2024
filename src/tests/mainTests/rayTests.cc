#include <cmath>
#include <doctest.h>
#include "../testsupport.h"

#include <spectrenotes/types.h>
#include <spectrenotes/ray.h>
#include <spectrenotes/aabb.h>
#include <spectrenotes/mesh.h>
#include <spectrenotes/bvh.h>
#include <spectrenotes/camera.h>
#include <spectrenotes/random.h>

using namespace Spectrenotes;

namespace {

}


TEST_CASE("Ray transform test [Ray]") {
    Ray ray(Vector3(0.0, 0.0, 0.0), Vector3(0.0, 0.0, -1.0));
    Quaterion q = Quaterion::makeRotation(M_PI * 0.25, 1.0, 0.0, 0.0);
    Matrix4 m = q.getMatrix();

    Vector3 p(0.0, 0.0, -1.0);
    Vector3 pm = Matrix4::transformV3(m, p);
    Vector3 pq = q.rotate(p);

    Ray ray1 = ray.transformed(m);
    REQUIRE(ray1.origin.x == doctest::Approx(ray.origin.x).epsilon(kTestEPS));
    REQUIRE(ray1.origin.y == doctest::Approx(ray.origin.y).epsilon(kTestEPS));
    REQUIRE(ray1.origin.z == doctest::Approx(ray.origin.z).epsilon(kTestEPS));

}

TEST_CASE("Ray triangle test [Ray]") {
    Mesh::Triangle tri;
    Vector3 va(0.0, 0.0, 0.0);
    Vector3 vb(1.0, 0.0, 0.0);
    Vector3 vc(0.0, 1.0, 0.0);
    tri.initialize(va, vb, vc);

    Ray ray(Vector3(0.0, 0.0, 1.0), Vector3(0.0, 0.0, -1.0));

    RTFloat bb, bc;
    bool h;
    
    h = tri.intersection(ray, 0.0, 10.0, &bb, &bc);
    REQUIRE(h);
    REQUIRE_EQ(bb, doctest::Approx(0.0));
    REQUIRE_EQ(bc, doctest::Approx(0.0));

    ray.origin.set(1.0, 0.0, 0.0);
    h = tri.intersection(ray, 0.0, 10.0, &bb, &bc);
    REQUIRE(!h);
    REQUIRE_EQ(bb, doctest::Approx(1.0));
    REQUIRE_EQ(bc, doctest::Approx(0.0));

    ray.origin.set(0.0, 1.0, 0.0);
    h = tri.intersection(ray, 0.0, 10.0, &bb, &bc);
    REQUIRE(!h);
    REQUIRE_EQ(bb, doctest::Approx(0.0));
    REQUIRE_EQ(bc, doctest::Approx(1.0));
}

TEST_CASE("Ray generate test [Ray] [Camera]") {
    Camera camera;

    camera.sensorWidth = 24.0;
    camera.sensorHeight = 18.0;
    camera.fNumber = 4.0;
    camera.focalLength = 100.0;
    camera.focusDistance = 1.0;
    camera.focusPlaneWidth = 0.24;
    camera.focusPlaneHeight = 0.18;
    camera.initWithType(Camera::CameraType::kFocusPlanePerspectiveCamera);

    Random rng;
    Ray ray;

    ray = camera.getRay(0.0, 0.0, &rng);
    ray.direction = ray.direction / ray.direction.z;

    ray = camera.getRay(-1.0, 0.0, &rng);
    ray.direction = ray.direction / ray.direction.z;
    ray = camera.getRay(0.0, -1.0, &rng);
    ray.direction = ray.direction / ray.direction.z;

    ray = camera.getRay(1.0, 0.0, &rng);
    ray.direction = ray.direction / ray.direction.z;
    ray = camera.getRay(0.0, 1.0, &rng);
    ray.direction = ray.direction / ray.direction.z;

    ray = camera.getRay(-1.0, -1.0, &rng);
    ray.direction = ray.direction / ray.direction.z;
    ray = camera.getRay(1.0, 1.0, &rng);
    ray.direction = ray.direction / ray.direction.z;
    REQUIRE_EQ(ray.direction.x + ray.origin.x, doctest::Approx(camera.focusPlaneWidth * -0.5));
    REQUIRE_EQ(ray.direction.y + ray.origin.y, doctest::Approx(camera.focusPlaneHeight * -0.5));
}