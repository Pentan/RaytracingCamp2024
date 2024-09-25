#include <cmath>
#include <doctest.h>
#include "../testsupport.h"

#include <spectrenotes/types.h>
#include <spectrenotes/bvh.h>
#include <spectrenotes/ray.h>
#include <spectrenotes/aabb.h>

using namespace Spectrenotes;

namespace {
}

TEST_CASE("BVH basic test [BVH]") {
	const int NUM = 20;
	std::vector<AABB> bounds(NUM);
	BVH bvh;
	for (int i = 0; i < NUM; i++) {
		RTFloat t = static_cast<RTFloat>(i) / NUM;
		Vector3 p0(sin(t * 1.9 + 0.1) * 2.0, sin(t * 2.1 - 0.6) * 3.0, sin(t * 4.2 + 1.6) * 4.0);
		Vector3 p1(sin(t * 2.7 + 1.2) * 2.0, sin(t * 5.7 - 1.1) * 3.0, sin(t * 6.1 + 0.2) * 4.0);
		bounds[i].clear();
		bounds[i].expand(p0);
		bounds[i].expand(p1);
		bounds[i].dataId = i;

		bvh.appendLeaf(&bounds[i]);
	}
	bvh.build();

	REQUIRE(true);
}
