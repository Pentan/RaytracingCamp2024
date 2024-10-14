#include <string>
#include <sstream>

#include <doctest.h>
#include "../testsupport.h"

#include <petals/types.h>
#include <petals/animation.h>
#include <petals/keyframesampler.h>

//#include <petals/assetlibrary.h>
//#include <petals/scene.h>

using namespace Petals;

TEST_CASE("KayframeSampler uniform key v3 Test [Animation]") {
    KeyframeSampler ks;
    ks.interpolation = KeyframeSampler::kLinear;
    ks.sampleComponents = 3;
    
    ks.timeStamps.push_back(0.0);
    ks.sampleBuffer.push_back(0.0);
    ks.sampleBuffer.push_back(1.0);
    ks.sampleBuffer.push_back(2.0);
    
    ks.timeStamps.push_back(0.33);
    ks.sampleBuffer.push_back(0.33);
    ks.sampleBuffer.push_back(1.0);
    ks.sampleBuffer.push_back(1.5);
    
    ks.timeStamps.push_back(0.66);
    ks.sampleBuffer.push_back(0.66);
    ks.sampleBuffer.push_back(2.0);
    ks.sampleBuffer.push_back(-0.5);
    
    ks.timeStamps.push_back(1.0);
    ks.sampleBuffer.push_back(1.0);
    ks.sampleBuffer.push_back(3.0);
    ks.sampleBuffer.push_back(0.2);
    
    SUBCASE("sample out of range negative") {
        auto v = ks.sampleVector3(-0.2);
        REQUIRE_EQ(v.x, doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.y, doctest::Approx(1.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.z, doctest::Approx(2.0).epsilon(kTestEPS));
    }
    
    SUBCASE("sample out of range positive") {
        auto v = ks.sampleVector3(1.2);
        REQUIRE_EQ(v.x, doctest::Approx(1.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.y, doctest::Approx(3.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.z, doctest::Approx(0.2).epsilon(kTestEPS));
    }
    
    SUBCASE("on key frame") {
        Vector3 v;
        
        v = ks.sampleVector3(0.0);
        REQUIRE_EQ(v.x, doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.y, doctest::Approx(1.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.z, doctest::Approx(2.0).epsilon(kTestEPS));
        
        v = ks.sampleVector3(0.33);
        REQUIRE_EQ(v.x, doctest::Approx(0.33).epsilon(kTestEPS));
        REQUIRE_EQ(v.y, doctest::Approx(1.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.z, doctest::Approx(1.5).epsilon(kTestEPS));
        
        v = ks.sampleVector3(0.66);
        REQUIRE_EQ(v.x, doctest::Approx(0.66).epsilon(kTestEPS));
        REQUIRE_EQ(v.y, doctest::Approx(2.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.z, doctest::Approx(-0.5).epsilon(kTestEPS));
        
        v = ks.sampleVector3(1.0);
        REQUIRE_EQ(v.x, doctest::Approx(1.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.y, doctest::Approx(3.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.z, doctest::Approx(0.2).epsilon(kTestEPS));
    }
    
    SUBCASE("iterpolation") {
        const RTTimeType t = 0.5;
        Vector3 v;
        Vector3 v0(0.33, 1.0, 1.5);
        Vector3 v1(0.66, 2.0, -0.5);
        Vector3 vc = Vector3::lerp(v0, v1, (t - 0.33) / (0.66 - 0.33));
        
        v = ks.sampleVector3(t);
        REQUIRE_EQ(v.x, doctest::Approx(vc.x).epsilon(kTestEPS));
        REQUIRE_EQ(v.y, doctest::Approx(vc.y).epsilon(kTestEPS));
        REQUIRE_EQ(v.z, doctest::Approx(vc.z).epsilon(kTestEPS));
    }
}

TEST_CASE("KayframeSampler v4 Test [Animation]") {
    KeyframeSampler ks;
    ks.interpolation = KeyframeSampler::kLinear;
    ks.sampleComponents = 4;
    
    ks.timeStamps.push_back(0.0);
    ks.sampleBuffer.push_back(0.0);
    ks.sampleBuffer.push_back(1.0);
    ks.sampleBuffer.push_back(2.0);
    ks.sampleBuffer.push_back(3.0);
    
    ks.timeStamps.push_back(0.33);
    ks.sampleBuffer.push_back(0.33);
    ks.sampleBuffer.push_back(1.0);
    ks.sampleBuffer.push_back(1.5);
    ks.sampleBuffer.push_back(2.0);
    
    ks.timeStamps.push_back(0.66);
    ks.sampleBuffer.push_back(0.66);
    ks.sampleBuffer.push_back(2.0);
    ks.sampleBuffer.push_back(-0.5);
    ks.sampleBuffer.push_back(-2.0);
    
    ks.timeStamps.push_back(1.0);
    ks.sampleBuffer.push_back(1.0);
    ks.sampleBuffer.push_back(3.0);
    ks.sampleBuffer.push_back(0.2);
    ks.sampleBuffer.push_back(0.0);
    
    SUBCASE("sample out of range negative") {
        auto v = ks.sampleVector4(-0.2);
        REQUIRE_EQ(v.x, doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.y, doctest::Approx(1.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.z, doctest::Approx(2.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.w, doctest::Approx(3.0).epsilon(kTestEPS));
    }
    
    SUBCASE("sample out of range positive") {
        auto v = ks.sampleVector4(1.2);
        REQUIRE_EQ(v.x, doctest::Approx(1.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.y, doctest::Approx(3.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.z, doctest::Approx(0.2).epsilon(kTestEPS));
        REQUIRE_EQ(v.w, doctest::Approx(0.0).epsilon(kTestEPS));
    }
    
    SUBCASE("on key frame") {
        Vector4 v;
        
        v = ks.sampleVector4(0.0);
        REQUIRE_EQ(v.x, doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.y, doctest::Approx(1.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.z, doctest::Approx(2.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.w, doctest::Approx(3.0).epsilon(kTestEPS));
        
        v = ks.sampleVector4(0.33);
        REQUIRE_EQ(v.x, doctest::Approx(0.33).epsilon(kTestEPS));
        REQUIRE_EQ(v.y, doctest::Approx(1.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.z, doctest::Approx(1.5).epsilon(kTestEPS));
        REQUIRE_EQ(v.w, doctest::Approx(2.0).epsilon(kTestEPS));
        
        v = ks.sampleVector4(0.66);
        REQUIRE_EQ(v.x, doctest::Approx(0.66).epsilon(kTestEPS));
        REQUIRE_EQ(v.y, doctest::Approx(2.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.z, doctest::Approx(-0.5).epsilon(kTestEPS));
        REQUIRE_EQ(v.w, doctest::Approx(-2.0).epsilon(kTestEPS));
        
        v = ks.sampleVector4(1.0);
        REQUIRE_EQ(v.x, doctest::Approx(1.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.y, doctest::Approx(3.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.z, doctest::Approx(0.2).epsilon(kTestEPS));
        REQUIRE_EQ(v.w, doctest::Approx(0.0).epsilon(kTestEPS));
    }
    
    SUBCASE("iterpolation") {
        const RTTimeType t = 0.5;
        Vector4 v;
        Vector4 v0(0.33, 1.0, 1.5, 2.0);
        Vector4 v1(0.66, 2.0, -0.5, -2.0);
        Vector4 vc = Vector4::lerp(v0, v1, (t - 0.33) / (0.66 - 0.33));
        
        v = ks.sampleVector4(t);
        REQUIRE_EQ(v.x, doctest::Approx(vc.x).epsilon(kTestEPS));
        REQUIRE_EQ(v.y, doctest::Approx(vc.y).epsilon(kTestEPS));
        REQUIRE_EQ(v.z, doctest::Approx(vc.z).epsilon(kTestEPS));
        REQUIRE_EQ(v.w, doctest::Approx(vc.w).epsilon(kTestEPS));
    }
}

TEST_CASE("KayframeSampler quaternion Test [Animation]") {
    Quaterion tmpq;
    
    KeyframeSampler ks;
    ks.interpolation = KeyframeSampler::kLinear;
    ks.sampleComponents = 4;
    
    ks.timeStamps.push_back(0.0);
    ks.sampleBuffer.push_back(0.0);
    ks.sampleBuffer.push_back(0.0);
    ks.sampleBuffer.push_back(0.0);
    ks.sampleBuffer.push_back(1.0);
    
    tmpq = Quaterion::makeRotation(M_PI * 0.5, 0.0, 0.0, 1.0);
    ks.timeStamps.push_back(0.5);
    ks.sampleBuffer.push_back(tmpq.x);
    ks.sampleBuffer.push_back(tmpq.y);
    ks.sampleBuffer.push_back(tmpq.z);
    ks.sampleBuffer.push_back(tmpq.w);
    
    tmpq = Quaterion::makeRotation(M_PI, 0.0, 0.0, 1.0);
    ks.timeStamps.push_back(1.0);
    ks.sampleBuffer.push_back(tmpq.x);
    ks.sampleBuffer.push_back(tmpq.y);
    ks.sampleBuffer.push_back(tmpq.z);
    ks.sampleBuffer.push_back(tmpq.w);
    
    SUBCASE("sample out of range negative") {
        auto v = ks.sampleQuaternion(-0.2);
        REQUIRE_EQ(v.x, doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.y, doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.z, doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE_EQ(v.w, doctest::Approx(1.0).epsilon(kTestEPS));
    }
    
    SUBCASE("sample out of range positive") {
        tmpq = Quaterion::makeRotation(M_PI, 0.0, 0.0, 1.0);
        auto v = ks.sampleQuaternion(1.2);
        REQUIRE_EQ(v.x, doctest::Approx(tmpq.x).epsilon(kTestEPS));
        REQUIRE_EQ(v.y, doctest::Approx(tmpq.y).epsilon(kTestEPS));
        REQUIRE_EQ(v.z, doctest::Approx(tmpq.z).epsilon(kTestEPS));
        REQUIRE_EQ(v.w, doctest::Approx(tmpq.w).epsilon(kTestEPS));
    }
    
    SUBCASE("on key frame") {
        Quaterion q;
        
        q = ks.sampleQuaternion(0.0);
        REQUIRE_EQ(q.x, doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE_EQ(q.y, doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE_EQ(q.z, doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE_EQ(q.w, doctest::Approx(1.0).epsilon(kTestEPS));
        
        tmpq = Quaterion::makeRotation(M_PI * 0.5, 0.0, 0.0, 1.0);
        q = ks.sampleQuaternion(0.5);
        REQUIRE_EQ(q.x, doctest::Approx(tmpq.x).epsilon(kTestEPS));
        REQUIRE_EQ(q.y, doctest::Approx(tmpq.y).epsilon(kTestEPS));
        REQUIRE_EQ(q.z, doctest::Approx(tmpq.z).epsilon(kTestEPS));
        REQUIRE_EQ(q.w, doctest::Approx(tmpq.w).epsilon(kTestEPS));
        
        tmpq = Quaterion::makeRotation(M_PI, 0.0, 0.0, 1.0);
        q = ks.sampleQuaternion(1.0);
        REQUIRE_EQ(q.x, doctest::Approx(tmpq.x).epsilon(kTestEPS));
        REQUIRE_EQ(q.y, doctest::Approx(tmpq.y).epsilon(kTestEPS));
        REQUIRE_EQ(q.z, doctest::Approx(tmpq.z).epsilon(kTestEPS));
        REQUIRE_EQ(q.w, doctest::Approx(tmpq.w).epsilon(kTestEPS));
    }
    
    SUBCASE("iterpolation") {
        Quaterion q;
        
        tmpq = Quaterion::makeRotation(M_PI * 0.125, 0.0, 0.0, 1.0);
        q = ks.sampleQuaternion(0.125);
        REQUIRE_EQ(q.x, doctest::Approx(tmpq.x).epsilon(kTestEPS));
        REQUIRE_EQ(q.y, doctest::Approx(tmpq.y).epsilon(kTestEPS));
        REQUIRE_EQ(q.z, doctest::Approx(tmpq.z).epsilon(kTestEPS));
        REQUIRE_EQ(q.w, doctest::Approx(tmpq.w).epsilon(kTestEPS));
        
        tmpq = Quaterion::makeRotation(M_PI * 0.25, 0.0, 0.0, 1.0);
        q = ks.sampleQuaternion(0.25);
        REQUIRE_EQ(q.x, doctest::Approx(tmpq.x).epsilon(kTestEPS));
        REQUIRE_EQ(q.y, doctest::Approx(tmpq.y).epsilon(kTestEPS));
        REQUIRE_EQ(q.z, doctest::Approx(tmpq.z).epsilon(kTestEPS));
        REQUIRE_EQ(q.w, doctest::Approx(tmpq.w).epsilon(kTestEPS));
        
        tmpq = Quaterion::makeRotation(M_PI * 0.75, 0.0, 0.0, 1.0);
        q = ks.sampleQuaternion(0.75);
        REQUIRE_EQ(q.x, doctest::Approx(tmpq.x).epsilon(kTestEPS));
        REQUIRE_EQ(q.y, doctest::Approx(tmpq.y).epsilon(kTestEPS));
        REQUIRE_EQ(q.z, doctest::Approx(tmpq.z).epsilon(kTestEPS));
        REQUIRE_EQ(q.w, doctest::Approx(tmpq.w).epsilon(kTestEPS));
    }
}
