#include <algorithm>
#include <cmath>
#include <limits>
#include "aabb.h"

using namespace Spectrenotes;

AABB::AABB():
	min(std::numeric_limits<RTFloat>::max()),
	max(-std::numeric_limits<RTFloat>::max())
{}

AABB::AABB(const RTFloat minval, const RTFloat maxval)
{
	if(minval < maxval) {
		min = Vector3(minval);
		max = Vector3(maxval);
	} else {
		min = Vector3(maxval);
		max = Vector3(minval);
	}
}

AABB::AABB(const Vector3 minvec, const Vector3 maxvec)
{
	for(int i = 0; i < 3; i++) {
		if(minvec.v[i] < maxvec.v[i]) {
			min.v[i] = minvec.v[i];
			max.v[i] = maxvec.v[i];
		} else {
			min.v[i] = maxvec.v[i];
			max.v[i] = minvec.v[i];
		}
	}
}

void AABB::clear() {
	min = Vector3(std::numeric_limits<RTFloat>::max());
	max = Vector3(-std::numeric_limits<RTFloat>::max());
}

Vector3 AABB::size() const {
	return max - min;
}

void AABB::expand(const Vector3 &p) {
	if(p.x < min.x) min.x = p.x;
	if(p.y < min.y) min.y = p.y;
	if(p.z < min.z) min.z = p.z;
	
	if(p.x > max.x) max.x = p.x;
	if(p.y > max.y) max.y = p.y;
	if(p.z > max.z) max.z = p.z;
}

void AABB::expand(const AABB &aabb) {
	if(min.x > aabb.min.x) min.x = aabb.min.x;
	if(min.y > aabb.min.y) min.y = aabb.min.y;
	if(min.z > aabb.min.z) min.z = aabb.min.z;
	
	if(max.x < aabb.max.x) max.x = aabb.max.x;
	if(max.y < aabb.max.y) max.y = aabb.max.y;
	if(max.z < aabb.max.z) max.z = aabb.max.z;
}

Vector3 AABB::centroid() const {
	return (min + max) * 0.5;
}

bool AABB::isInside(const Vector3 &p) const {
	return ((p.x > min.x && p.y > min.y && p.z > min.z) &&
			(p.x < max.x && p.y < max.y && p.z < max.z) );
}

bool AABB::isIntersect(const Ray &ray, RTFloat tnear, RTFloat tfar) const {
    RTFloat t = intersectDistance(ray);
    return (tnear <= t) && (t <= tfar);
}

RTFloat AABB::mightIntersectContent(const Ray &ray, RTFloat tfar) const {
    RTFloat tmin, tmax;
    bool ishit = testIntersect(ray, &tmin, &tmax);
    // miss
    if(!ishit) return -1.0;
    
    // returns nearest distance
    // from inside
    if(tmin <= 0.0) return 0.0;
    // from outside
    return (tmin <= tfar) ? tmin : -1.0;
}

RTFloat AABB::intersectDistance(const Ray &ray) const {
    RTFloat tmin, tmax;
    bool ishit = testIntersect(ray, &tmin, &tmax);
    if(!ishit) {
        return -1.0;
    }
    return (tmin < 0.0) ? tmax : tmin;
}

bool AABB::testIntersect(const Ray &ray, RTFloat* otmin, RTFloat* otmax) const {
	RTFloat largest_min = -std::numeric_limits<RTFloat>::max();
    RTFloat smallest_max = std::numeric_limits<RTFloat>::max();
    bool ishit = true;
	
	for(int i = 0; i < 3; i++) {
		RTFloat vdiv = 1.0 / ray.direction.v[i];
		RTFloat tmpmin = (min.v[i] - ray.origin.v[i]) * vdiv;
		RTFloat tmpmax = (max.v[i] - ray.origin.v[i]) * vdiv;
        if(vdiv < 0.0) {
            std::swap(tmpmin, tmpmax);
        }
        
        largest_min = std::max(largest_min, tmpmin);
        smallest_max = std::min(smallest_max, tmpmax);
        
		if(smallest_max < largest_min) {
            ishit = false;
            break;
		}
	}
    
    if(otmin != nullptr) { *otmin = largest_min; }
    if(otmax != nullptr) { *otmax = smallest_max; }
    return ishit;
}

AABB AABB::transformed(const AABB& a, const Matrix4& m) {
    AABB ret;
    Vector3 tmpv;
    tmpv.set(a.min.x, a.min.y, a.min.z);
    ret.expand(Matrix4::transformV3(m, tmpv));
    tmpv.set(a.max.x, a.min.y, a.min.z);
    ret.expand(Matrix4::transformV3(m, tmpv));
    tmpv.set(a.min.x, a.max.y, a.min.z);
    ret.expand(Matrix4::transformV3(m, tmpv));
    tmpv.set(a.max.x, a.max.y, a.min.z);
    ret.expand(Matrix4::transformV3(m, tmpv));
    tmpv.set(a.min.x, a.min.y, a.max.z);
    ret.expand(Matrix4::transformV3(m, tmpv));
    tmpv.set(a.max.x, a.min.y, a.max.z);
    ret.expand(Matrix4::transformV3(m, tmpv));
    tmpv.set(a.min.x, a.max.y, a.max.z);
    ret.expand(Matrix4::transformV3(m, tmpv));
    tmpv.set(a.max.x, a.max.y, a.max.z);
    ret.expand(Matrix4::transformV3(m, tmpv));
    return ret;
}
