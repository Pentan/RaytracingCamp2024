#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <doctest.h>
#include "../testsupport.h"

#include <spectrenotes/types.h>
#include <spectrenotes/material.h>
#include <spectrenotes/random.h>
#include <spectrenotes/ray.h>

using namespace Spectrenotes;

namespace {
    void SaveVectorsAsObj(std::string filename, std::vector<Vector3>& vecs) {
        std::string outdir = "materialTests";
        CheckTestOutputDir(outdir);
        
        std::stringstream ss;
        ss << SPCTRNTS_TEST_OUTPUT_DIR << "/" << outdir << "/" << filename << ".obj";
        std::string outpath = ss.str();
        
        std::fstream fs(outpath, std::ios_base::out);
        if(!fs) {
            std::cerr << outpath << " open failed." << std::endl;
            return;
        }
        
        fs << "v 0.0 0.0 0.0\n";
        for(auto ite = vecs.begin(); ite != vecs.end(); ++ite) {
            auto v = *ite;
            fs << "v " << v.x << " " << v.y << " " << v.z << "\n";
        }
        for(size_t i = 0; i < vecs.size(); i++) {
            fs << "l 1 " << i + 2 << "\n";
        }
        fs << std::endl;
        fs.close();
    }
}

TEST_CASE("Sampling Basis test [Material]") {
    SUBCASE("axis align") {
        Vector3 vn, v0, v1;
        
        vn.set(1.0, 0.0, 0.0);
        Sampler::basisFromNormal(vn, &v0, &v1);
        REQUIRE(Vector3::dot(vn, v0) == doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE(Vector3::dot(vn, v1) == doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE(Vector3::dot(v0, v1) == doctest::Approx(0.0).epsilon(kTestEPS));
        
        vn.set(0.0, 1.0, 0.0);
        Sampler::basisFromNormal(vn, &v0, &v1);
        REQUIRE(Vector3::dot(vn, v0) == doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE(Vector3::dot(vn, v1) == doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE(Vector3::dot(v0, v1) == doctest::Approx(0.0).epsilon(kTestEPS));
        
        vn.set(0.0, 0.0, 1.0);
        Sampler::basisFromNormal(vn, &v0, &v1);
        REQUIRE(Vector3::dot(vn, v0) == doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE(Vector3::dot(vn, v1) == doctest::Approx(0.0).epsilon(kTestEPS));
        REQUIRE(Vector3::dot(v0, v1) == doctest::Approx(0.0).epsilon(kTestEPS));
    }
    
    SUBCASE("misc") {
        Vector3 vn, v0, v1;
        for(int ia = 0; ia < 256; ia++) {
            RTFloat a = ia * 2.4;
            RTFloat u = sin(a);
            RTFloat v = cos(a);
            for(int ie = 0; ie < 16; ie++) {
                RTFloat r = sin((ie + 0.5) / 16.0 * M_PI);
                RTFloat y = sqrt(1.0 - r * r);
                vn.set(u * r, y, v * r);
                Sampler::basisFromNormal(vn, &v0, &v1);
                REQUIRE(Vector3::dot(vn, v0) == doctest::Approx(0.0).epsilon(kTestEPS));
                REQUIRE(Vector3::dot(vn, v1) == doctest::Approx(0.0).epsilon(kTestEPS));
                REQUIRE(Vector3::dot(v0, v1) == doctest::Approx(0.0).epsilon(kTestEPS));
            }
        }
    }
}

TEST_CASE("Uniform Hemisphere test [Material]") {
    const int num = 512;
    Random rng;
    std::vector<Vector3> vecs;
    
    rng.setSeed(time(NULL));
    vecs.reserve(num + 1);
    
    SUBCASE("X") {
        vecs.clear();
        
        Vector3 n(1.0, 0.0, 0.0);
        vecs.push_back(n * 2.0);
        for(int i = 0; i < num; i++) {
            Sampler::SampleResult s = Sampler::sampleUniformHemisphere(n, rng);
            vecs.push_back(s.v);
            REQUIRE(Vector3::dot(n, s.v) >= 0.0);
        }
        SaveVectorsAsObj("uniform_x", vecs);
    }
    
    SUBCASE("Y") {
        vecs.clear();
        
        Vector3 n(0.0, 1.0, 0.0);
        vecs.push_back(n * 2.0);
        for(int i = 0; i < num; i++) {
            Sampler::SampleResult s = Sampler::sampleUniformHemisphere(n, rng);
            vecs.push_back(s.v);
            REQUIRE(Vector3::dot(n, s.v) >= 0.0);
        }
        SaveVectorsAsObj("uniform_y", vecs);
    }
    
    SUBCASE("Z") {
        vecs.clear();
        
        Vector3 n(0.0, 0.0, 1.0);
        vecs.push_back(n * 2.0);
        for(int i = 0; i < num; i++) {
            Sampler::SampleResult s = Sampler::sampleUniformHemisphere(n, rng);
            vecs.push_back(s.v);
            REQUIRE(Vector3::dot(n, s.v) >= 0.0);
        }
        SaveVectorsAsObj("uniform_z", vecs);
    }
    
    SUBCASE("1_1_1") {
        vecs.clear();
        
        Vector3 n(1.0, 1.0, 1.0);
        n.normalize();
        
        vecs.push_back(n * 2.0);
        for(int i = 0; i < num; i++) {
            Sampler::SampleResult s = Sampler::sampleUniformHemisphere(n, rng);
            vecs.push_back(s.v);
            REQUIRE(Vector3::dot(n, s.v) >= 0.0);
        }
        SaveVectorsAsObj("uniform_1_1_1", vecs);
    }
}

TEST_CASE("Cos Hemisphere test [Material]") {
    const int num = 512;
    Random rng;
    std::vector<Vector3> vecs;
    
    rng.setSeed(time(NULL));
    vecs.reserve(num + 1);
    
    SUBCASE("X") {
        vecs.clear();
        
        Vector3 n(1.0, 0.0, 0.0);
        vecs.push_back(n * 2.0);
        for(int i = 0; i < num; i++) {
            Sampler::SampleResult s = Sampler::sampleCosineWeightedHemisphere(n, rng);
            vecs.push_back(s.v);
            REQUIRE(Vector3::dot(n, s.v) >= 0.0);
        }
        SaveVectorsAsObj("cosweight_x", vecs);
    }
    
    SUBCASE("Y") {
        vecs.clear();
        
        Vector3 n(0.0, 1.0, 0.0);
        vecs.push_back(n * 2.0);
        for(int i = 0; i < num; i++) {
            Sampler::SampleResult s = Sampler::sampleCosineWeightedHemisphere(n, rng);
            vecs.push_back(s.v);
            REQUIRE(Vector3::dot(n, s.v) >= 0.0);
        }
        SaveVectorsAsObj("cosweight_y", vecs);
    }
    
    SUBCASE("Z") {
        vecs.clear();
        
        Vector3 n(0.0, 0.0, 1.0);
        vecs.push_back(n * 2.0);
        for(int i = 0; i < num; i++) {
            Sampler::SampleResult s = Sampler::sampleCosineWeightedHemisphere(n, rng);
            vecs.push_back(s.v);
            REQUIRE(Vector3::dot(n, s.v) >= 0.0);
        }
        SaveVectorsAsObj("cosweight_z", vecs);
    }
    
    SUBCASE("1_1_1") {
        vecs.clear();
        
        Vector3 n(1.0, 1.0, 1.0);
        n.normalize();
        
        vecs.push_back(n * 2.0);
        for(int i = 0; i < num; i++) {
            Sampler::SampleResult s = Sampler::sampleCosineWeightedHemisphere(n, rng);
            vecs.push_back(s.v);
            REQUIRE(Vector3::dot(n, s.v) >= 0.0);
        }
        SaveVectorsAsObj("cosweight_1_1_1", vecs);
    }
}
