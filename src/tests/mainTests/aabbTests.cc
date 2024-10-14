#include <doctest.h>
#include "../testsupport.h"

#include <petals/types.h>
#include <petals/ray.h>
#include <petals/aabb.h>

using namespace Petals;

namespace {
    bool IsHitRayAndAABB(const AABB& aabb, Vector3 o, Vector3 d, RTFloat n=1e-2, RTFloat f=1e8) {
        return aabb.isIntersect(Ray(o, Vector3::normalized(d)), n, f);
    }
    
    RTFloat hitRayAndAABBExact(const AABB& aabb, Vector3 ro, Vector3 rd, RTFloat nr=1e-2, RTFloat fr=1e8) {
        RTFloat t = -1.0;
        for(int i = 0; i < 2; i++) {
            auto pp = (i == 0) ? aabb.min : aabb.max;
            for(int j = 0; j < 3; j++) {
                Vector3 pn;
                RTFloat a = (i == 0) ? -1.0 : 1.0;
                pn.x = (j == 0) ? a : 0.0;
                pn.y = (j == 1) ? a : 0.0;
                pn.z = (j == 2) ? a : 0.0;
                
                Vector3 op = ro - pp;
                RTFloat od = Vector3::dot(pn, op);
                RTFloat vd = -Vector3::dot(pn, rd);
                RTFloat d = od / vd;
                if((d > nr && d < fr) && (t < 0.0 || d < t)) {
                    Vector3 hp = ro + rd * d;
                    bool h = false;
                    switch (j) {
                        case 0:
                            h = (aabb.min.y <= hp.y && hp.y <= aabb.max.y) && (aabb.min.z <= hp.z && hp.z <= aabb.max.z);
                            break;
                        case 1:
                            h = (aabb.min.x <= hp.x && hp.x <= aabb.max.x) && (aabb.min.z <= hp.z && hp.z <= aabb.max.z);
                            break;
                        case 2:
                            h = (aabb.min.x <= hp.x && hp.x <= aabb.max.x) && (aabb.min.y <= hp.y && hp.y <= aabb.max.y);
                            break;
                    }
                    if(h) { t = d; }
                }
            }
        }
        return t;
    }
    
    RTFloat genHitRay(AABB aabb, RTFloat t, int pn, Vector3 ro, Vector3* rd) {
        Vector3 op = Vector3::lerp(aabb.min, aabb.max, t);
        switch (pn) {
            case 0:
                op.x = aabb.min.x; // 0:-x
                break;
            case 1:
                op.y = aabb.min.y; // 1:-y
                break;
            case 2:
                op.z = aabb.min.z; // 2:-z
                break;
            case 3:
                op.x = aabb.max.x; // 3:+x
                break;
            case 4:
                op.y = aabb.max.y; // 4:+y
                break;
            case 5:
                op.z = aabb.max.z; // 5:+z
                break;
        }
        *rd = op - ro;
        RTFloat d = rd->length();
        *rd = *rd / d;
        return d;
    };
}

TEST_CASE("AABB basic test [AABB]") {
    SUBCASE("construct with 2 scalar") {
        AABB aabb(1.0, 2.0);
        
        REQUIRE( aabb.min.x == 1.0 );
        REQUIRE( aabb.min.y == 1.0 );
        REQUIRE( aabb.min.z == 1.0 );
        REQUIRE( aabb.max.x == 2.0 );
        REQUIRE( aabb.max.y == 2.0 );
        REQUIRE( aabb.max.z == 2.0 );
    }
    
    SUBCASE("construct with min and max") {
        AABB aabb(Vector3(-1.0, -2.0, -3.0), Vector3(1.0, 2.0, 3.0));
        
        REQUIRE( aabb.min.x == -1.0 );
        REQUIRE( aabb.min.y == -2.0 );
        REQUIRE( aabb.min.z == -3.0 );
        REQUIRE( aabb.max.x == 1.0 );
        REQUIRE( aabb.max.y == 2.0 );
        REQUIRE( aabb.max.z == 3.0 );
    }
    
    SUBCASE("expand") {
        AABB aabb;
        
        aabb.expand(Vector3(1.0, 2.0, -4.0));
        aabb.expand(Vector3(0.0, -1.0, 3.0));
        
        REQUIRE( aabb.min.x == 0.0 );
        REQUIRE( aabb.min.y == -1.0 );
        REQUIRE( aabb.min.z == -4.0 );
        REQUIRE( aabb.max.x == 1.0 );
        REQUIRE( aabb.max.y == 2.0 );
        REQUIRE( aabb.max.z == 3.0 );
    }
}

TEST_CASE("AABB intersect [AABB]") {
    AABB aabb(Vector3(-1.0), Vector3(1.0));
    
    REQUIRE( IsHitRayAndAABB(aabb, Vector3(0.0, 0.0, 5.0), Vector3(0.0, 0.0, -1.0)) );
    REQUIRE( IsHitRayAndAABB(aabb, Vector3(0.0, 0.0, 0.5), Vector3(0.0, 0.0, -1.0)) );
    REQUIRE( IsHitRayAndAABB(aabb, Vector3(0.0, 0.0, 10.0), Vector3(1.0, 1.0, 1.0-10.0)) );
    
    REQUIRE_FALSE( IsHitRayAndAABB(aabb, Vector3(0.0, 0.0, 5.0), Vector3(0.0, 0.0, 1.0)) );
    REQUIRE_FALSE( IsHitRayAndAABB(aabb, Vector3(0.0, 0.0, 5.0), Vector3(0.0, 0.5, 0.5)) );
    
    REQUIRE_FALSE( IsHitRayAndAABB(aabb, Vector3(0.0, 0.0, 5.0), Vector3(0.0, 0.0, -1.0), 10.0, 20.0) );
    REQUIRE_FALSE( IsHitRayAndAABB(aabb, Vector3(0.0, 0.0, 5.0), Vector3(0.0, 0.0, -1.0), 1e-2, 3.0) );
}


TEST_CASE("AABB exact test [AABB]") {
    AABB aabb(Vector3(-1.0), Vector3(1.0));
    REQUIRE_EQ(hitRayAndAABBExact(aabb, Vector3( 0.2,  0.1,  2.0), Vector3(0.0, 0.0, -1.0)), doctest::Approx(1.0).epsilon(kTestEPS));
    REQUIRE_EQ(hitRayAndAABBExact(aabb, Vector3( 0.5, -0.3, -2.0), Vector3(0.0, 0.0,  1.0)), doctest::Approx(1.0).epsilon(kTestEPS));
    REQUIRE_EQ(hitRayAndAABBExact(aabb, Vector3( 0.9,  2.0, -0.8), Vector3(0.0, -1.0, 0.0)), doctest::Approx(1.0).epsilon(kTestEPS));
    REQUIRE_EQ(hitRayAndAABBExact(aabb, Vector3( 0.0, -2.0,  0.0), Vector3(0.0,  1.0, 0.0)), doctest::Approx(1.0).epsilon(kTestEPS));
    REQUIRE_EQ(hitRayAndAABBExact(aabb, Vector3( 2.0, -0.2, -0.0), Vector3(-1.0, 0.0, 0.0)), doctest::Approx(1.0).epsilon(kTestEPS));
    REQUIRE_EQ(hitRayAndAABBExact(aabb, Vector3(-2.0,  0.3,  0.4), Vector3( 1.0, 0.0, 0.0)), doctest::Approx(1.0).epsilon(kTestEPS));

    REQUIRE_EQ(hitRayAndAABBExact(aabb, Vector3( 1.0,  1.0,  2.0), Vector3(0.0, 0.0, -1.0)), doctest::Approx(1.0).epsilon(kTestEPS));
    REQUIRE_EQ(hitRayAndAABBExact(aabb, Vector3( 1.0, -1.0, -2.0), Vector3(0.0, 0.0,  1.0)), doctest::Approx(1.0).epsilon(kTestEPS));
    REQUIRE_EQ(hitRayAndAABBExact(aabb, Vector3( 1.0,  2.0, -1.0), Vector3(0.0, -1.0, 0.0)), doctest::Approx(1.0).epsilon(kTestEPS));
    REQUIRE_EQ(hitRayAndAABBExact(aabb, Vector3( 1.0, -2.0,  1.0), Vector3(0.0,  1.0, 0.0)), doctest::Approx(1.0).epsilon(kTestEPS));
    REQUIRE_EQ(hitRayAndAABBExact(aabb, Vector3( 2.0, -1.0, -1.0), Vector3(-1.0, 0.0, 0.0)), doctest::Approx(1.0).epsilon(kTestEPS));
    REQUIRE_EQ(hitRayAndAABBExact(aabb, Vector3(-2.0,  1.0,  1.0), Vector3( 1.0, 0.0, 0.0)), doctest::Approx(1.0).epsilon(kTestEPS));
    
    REQUIRE(hitRayAndAABBExact(aabb, Vector3( 1.01,  1.0,  2.0), Vector3(0.0, 0.0, -1.0)) < 0.0);
    REQUIRE(hitRayAndAABBExact(aabb, Vector3( 1.01, -1.0, -2.0), Vector3(0.0, 0.0,  1.0)) < 0.0);
    REQUIRE(hitRayAndAABBExact(aabb, Vector3( 1.01,  2.0, -1.0), Vector3(0.0, -1.0, 0.0)) < 0.0);
    REQUIRE(hitRayAndAABBExact(aabb, Vector3( 1.01, -2.0,  1.0), Vector3(0.0,  1.0, 0.0)) < 0.0);
    REQUIRE(hitRayAndAABBExact(aabb, Vector3( 2.0, -1.01, -1.0), Vector3(-1.0, 0.0, 0.0)) < 0.0);
    REQUIRE(hitRayAndAABBExact(aabb, Vector3(-2.0,  1.01,  1.0), Vector3( 1.0, 0.0, 0.0)) < 0.0);
    
    aabb.clear();
    aabb.expand(Vector3(-1.0, -4.0, -3.0));
    aabb.expand(Vector3(3.0,  2.0,  4.0));
    
    {
        Vector3 rd, ro(-5.0, 2.0, 6.0);
        RTFloat d = genHitRay(aabb, 0.5, 0, ro, &rd);
        RTFloat hd = hitRayAndAABBExact(aabb, ro, rd);
        REQUIRE_EQ(d, doctest::Approx(hd).epsilon(kTestEPS));
        
        d = genHitRay(aabb, 1.2, 0, ro, &rd);
        hd = hitRayAndAABBExact(aabb, ro, rd);
        REQUIRE(hd < 0.0);
    }
    
    {
        Vector3 rd, ro(2.0, -6.0, -6.0);
        RTFloat d = genHitRay(aabb, 0.2, 1, ro, &rd);
        RTFloat hd = hitRayAndAABBExact(aabb, ro, rd);
        REQUIRE_EQ(d, doctest::Approx(hd).epsilon(kTestEPS));
        
        d = genHitRay(aabb, -0.2, 1, ro, &rd);
        hd = hitRayAndAABBExact(aabb, ro, rd);
        REQUIRE(hd < 0.0);
    }
    
    {
        Vector3 rd, ro(0.0, -3.0, -8.0);
        RTFloat d = genHitRay(aabb, 0.9, 2, ro, &rd);
        RTFloat hd = hitRayAndAABBExact(aabb, ro, rd);
        REQUIRE_EQ(d, doctest::Approx(hd).epsilon(kTestEPS));
        
        d = genHitRay(aabb, 1.2, 2, ro, &rd);
        hd = hitRayAndAABBExact(aabb, ro, rd);
        REQUIRE(hd < 0.0);
    }
    
    {
        Vector3 rd, ro(5.0, 0.0, 2.0);
        RTFloat d = genHitRay(aabb, 0.35, 3, ro, &rd);
        RTFloat hd = hitRayAndAABBExact(aabb, ro, rd);
        REQUIRE_EQ(d, doctest::Approx(hd).epsilon(kTestEPS));
        
        d = genHitRay(aabb, 1.2, 3, ro, &rd);
        hd = hitRayAndAABBExact(aabb, ro, rd);
        REQUIRE(hd < 0.0);
    }
    
    {
        Vector3 rd, ro(-2.0, 9.0, -6.0);
        RTFloat d = genHitRay(aabb, 0.25, 4, ro, &rd);
        RTFloat hd = hitRayAndAABBExact(aabb, ro, rd);
        REQUIRE_EQ(d, doctest::Approx(hd).epsilon(kTestEPS));
        
        d = genHitRay(aabb, -2.0, 4, ro, &rd);
        hd = hitRayAndAABBExact(aabb, ro, rd);
        REQUIRE(hd < 0.0);
    }
    
    {
        Vector3 rd, ro(2.0, 9.0, 8.0);
        RTFloat d = genHitRay(aabb, 0.85, 5, ro, &rd);
        RTFloat hd = hitRayAndAABBExact(aabb, ro, rd);
        REQUIRE_EQ(d, doctest::Approx(hd).epsilon(kTestEPS));
        
        d = genHitRay(aabb, 2.2, 5, ro, &rd);
        hd = hitRayAndAABBExact(aabb, ro, rd);
        REQUIRE(hd < 0.0);
    }
}

TEST_CASE("AABB distance test [AABB]") {
    AABB aabb(Vector3(-3.0, -4.0, -2.0), Vector3(2.0,  3.0,  4.0));
    
    {
        Vector3 rd, ro(2.0, 9.0, 8.0);
        RTFloat d = genHitRay(aabb, 0.85, 5, ro, &rd);
        Ray ray(ro, rd);
        
        RTFloat xhd = hitRayAndAABBExact(aabb, ro, rd);
        RTFloat hd = aabb.intersectDistance(ray);
        
        REQUIRE(!aabb.isInside(ro));
        REQUIRE_EQ(d, doctest::Approx(xhd).epsilon(kTestEPS));
        REQUIRE_EQ(d, doctest::Approx(hd).epsilon(kTestEPS));
    }
    
    {
        Vector3 rd, ro(3.0, -9.0, 2.0);
        RTFloat d = genHitRay(aabb, 0.5, 1, ro, &rd);
        Ray ray(ro, rd);
        
        RTFloat xhd = hitRayAndAABBExact(aabb, ro, rd);
        RTFloat hd = aabb.intersectDistance(ray);
        
        REQUIRE(!aabb.isInside(ro));
        REQUIRE_EQ(d, doctest::Approx(xhd).epsilon(kTestEPS));
        REQUIRE_EQ(d, doctest::Approx(hd).epsilon(kTestEPS));
    }
    
    {
        Vector3 rd, ro(5.0, -4.0, -7.0);
        RTFloat d = genHitRay(aabb, 0.4, 2, ro, &rd);
        Ray ray(ro, rd);
        
        RTFloat xhd = hitRayAndAABBExact(aabb, ro, rd);
        RTFloat hd = aabb.intersectDistance(ray);
        
        REQUIRE(!aabb.isInside(ro));
        REQUIRE_EQ(d, doctest::Approx(xhd).epsilon(kTestEPS));
        REQUIRE_EQ(d, doctest::Approx(hd).epsilon(kTestEPS));
    }
    
    {
        Vector3 rd, ro(5.0, -4.0, -7.0);
        RTFloat d = genHitRay(aabb, 0.75, 3, ro, &rd);
        Ray ray(ro, rd);
        
        RTFloat xhd = hitRayAndAABBExact(aabb, ro, rd);
        RTFloat hd = aabb.intersectDistance(ray);
        
        REQUIRE(!aabb.isInside(ro));
        REQUIRE_EQ(d, doctest::Approx(xhd).epsilon(kTestEPS));
        REQUIRE_EQ(d, doctest::Approx(hd).epsilon(kTestEPS));
    }
    
    {
        Vector3 rd, ro(-2.0, 9.0, 2.0);
        RTFloat d = genHitRay(aabb, 0.4, 4, ro, &rd);
        Ray ray(ro, rd);
        
        RTFloat xhd = hitRayAndAABBExact(aabb, ro, rd);
        RTFloat hd = aabb.intersectDistance(ray);
        
        REQUIRE(!aabb.isInside(ro));
        REQUIRE_EQ(d, doctest::Approx(xhd).epsilon(kTestEPS));
        REQUIRE_EQ(d, doctest::Approx(hd).epsilon(kTestEPS));
    }
    
    {
        Vector3 rd, ro(2.0, -4.0, 7.0);
        RTFloat d = genHitRay(aabb, 0.6, 5, ro, &rd);
        Ray ray(ro, rd);
        
        RTFloat xhd = hitRayAndAABBExact(aabb, ro, rd);
        RTFloat hd = aabb.intersectDistance(ray);
        
        REQUIRE(!aabb.isInside(ro));
        REQUIRE_EQ(d, doctest::Approx(xhd).epsilon(kTestEPS));
        REQUIRE_EQ(d, doctest::Approx(hd).epsilon(kTestEPS));
    }
    
    // inside
    {
        Vector3 rd, ro(1.0, -1.0, 2.0);
        RTFloat d = genHitRay(aabb, 0.6, 0, ro, &rd);
        Ray ray(ro, rd);
        
        RTFloat xhd = hitRayAndAABBExact(aabb, ro, rd);
        RTFloat hd = aabb.intersectDistance(ray);
        
        REQUIRE(aabb.isInside(ro));
        REQUIRE_EQ(d, doctest::Approx(xhd).epsilon(kTestEPS));
        REQUIRE_EQ(d, doctest::Approx(hd).epsilon(kTestEPS));
    }
    
    {
        Vector3 rd, ro(-2.0, -2.0, 2.0);
        RTFloat d = genHitRay(aabb, 0.4, 1, ro, &rd);
        Ray ray(ro, rd);
        
        RTFloat xhd = hitRayAndAABBExact(aabb, ro, rd);
        RTFloat hd = aabb.intersectDistance(ray);
        
        REQUIRE(aabb.isInside(ro));
        REQUIRE_EQ(d, doctest::Approx(xhd).epsilon(kTestEPS));
        REQUIRE_EQ(d, doctest::Approx(hd).epsilon(kTestEPS));
    }
    
    {
        Vector3 rd, ro(-2.5, 1.0, 1.0);
        RTFloat d = genHitRay(aabb, 0.4, 2, ro, &rd);
        Ray ray(ro, rd);
        
        RTFloat xhd = hitRayAndAABBExact(aabb, ro, rd);
        RTFloat hd = aabb.intersectDistance(ray);
        
        REQUIRE(aabb.isInside(ro));
        REQUIRE_EQ(d, doctest::Approx(xhd).epsilon(kTestEPS));
        REQUIRE_EQ(d, doctest::Approx(hd).epsilon(kTestEPS));
    }
    
    {
        Vector3 rd, ro(-2.0, -1.0, 1.0);
        RTFloat d = genHitRay(aabb, 0.4, 3, ro, &rd);
        Ray ray(ro, rd);
        
        RTFloat xhd = hitRayAndAABBExact(aabb, ro, rd);
        RTFloat hd = aabb.intersectDistance(ray);
        
        REQUIRE(aabb.isInside(ro));
        REQUIRE_EQ(d, doctest::Approx(xhd).epsilon(kTestEPS));
        REQUIRE_EQ(d, doctest::Approx(hd).epsilon(kTestEPS));
    }
    
    {
        Vector3 rd, ro(1.0, -3.0, 0.0);
        RTFloat d = genHitRay(aabb, 0.25, 4, ro, &rd);
        Ray ray(ro, rd);
        
        RTFloat xhd = hitRayAndAABBExact(aabb, ro, rd);
        RTFloat hd = aabb.intersectDistance(ray);
        
        REQUIRE(aabb.isInside(ro));
        REQUIRE_EQ(d, doctest::Approx(xhd).epsilon(kTestEPS));
        REQUIRE_EQ(d, doctest::Approx(hd).epsilon(kTestEPS));
    }
    
    {
        Vector3 rd, ro(1.0, -3.0, -1.0);
        RTFloat d = genHitRay(aabb, 0.4, 5, ro, &rd);
        Ray ray(ro, rd);
        
        RTFloat xhd = hitRayAndAABBExact(aabb, ro, rd);
        RTFloat hd = aabb.intersectDistance(ray);
        
        REQUIRE(aabb.isInside(ro));
        REQUIRE_EQ(d, doctest::Approx(xhd).epsilon(kTestEPS));
        REQUIRE_EQ(d, doctest::Approx(hd).epsilon(kTestEPS));
    }
    
}
