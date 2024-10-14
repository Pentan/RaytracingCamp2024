//
//  camera.cpp
//  Petals
//
//  Created by SatoruNAKAJIMA on 2019/08/16.
//

#include "camera.h"

#include "types.h"
using namespace Petals;

Camera::Camera():
    type(CameraType::kPerspectiveCamera),
    sensorWidth(24.0),
    sensorHeight(18.0),
    focalLength(100.0),
    fNumber(0.0),
    focusDistance(1.0),
    focusPlaneWidth(0.24),
    focusPlaneHeight(0.18)
{
}

Camera::~Camera() {}

void Camera::initWithType(CameraType t)
{
    type = t;
    switch (type) {
        default:
        case kPerspectiveCamera:
            perspective.aspect = 1.0;
            perspective.yfov = 1.0;
            perspective.znear = 0.0;
            perspective.zfar = 1e8;
            getRayFunc = (fNumber > 0.0) ? getThinLensRay : getPerspectiveRay;
            break;
        case kOrthographicsCamera:
            orthographics.xmag = 1.0;
            orthographics.ymag = 1.0;
            orthographics.znear = 0.0;
            orthographics.zfar = 1e8;
            getRayFunc = getOrthoRay;
            break;
        case kFocusPlanePerspectiveCamera:
            getRayFunc = getThinLensRayFromFocusPlane;
            break;
    }
}

Ray Camera::getRay(RTFloat tx, RTFloat ty, Random* rng)
{
    return getRayFunc(this, tx, ty, rng);
}

Ray Camera::getThinLensRay(Camera* cam, RTFloat tx, RTFloat ty, Random* rng) {
    // forcus position
    RTFloat h = std::tan(cam->perspective.yfov * 0.5);
    Vector3 p(tx * h * cam->perspective.aspect, ty * h, -1);
    p = p * cam->focusDistance;

    // h = tan(fov/2) = (sensor height / 2) / focul length
    // sensor height := 24[mm]
    // focul length = 12 / h
    // aperture diameter = focul length / f number
    RTFloat apertuerR = 12.0 / (h * cam->fNumber) * 0.0005; // 0.5[diameter->radius] * 1/1000[mm->m]

    // circle sample
    RTFloat r = std::sqrt(rng->nextDoubleCC());
    RTFloat theta = rng->nextDoubleCO() * 2.0 * kPI;
    RTFloat sx = r * cos(theta);
    RTFloat sy = r * sin(theta);

    Vector3 o(sx * apertuerR, sy * apertuerR, 0.0);
    Vector3 d = p - o;
    d.normalize();

    return Ray(o, d);
}

Ray Camera::getPerspectiveRay(Camera* cam, RTFloat tx, RTFloat ty, Random* rng) {
    RTFloat y = std::tan(cam->perspective.yfov * 0.5);
    Vector3 o(0.0, 0.0, 0.0);
    Vector3 d(tx * y * cam->perspective.aspect, ty * y, -1);
    d.normalize();
    return Ray(o, d);
}

Ray Camera::getOrthoRay(Camera* cam, RTFloat tx, RTFloat ty, Random* rng) {
    Vector3 o(tx * cam->orthographics.xmag, ty * cam->orthographics.ymag, 0.0);
    Vector3 d(0.0, 0.0, -1.0);
    return Ray(o, d);
}

Ray Camera::getThinLensRayFromFocusPlane(Camera* cam, RTFloat tx, RTFloat ty, Random* rng) {
    Vector3 p(
        tx * cam->focusPlaneWidth * -0.5,
        ty * cam->focusPlaneHeight * -0.5,
        cam->focusDistance);
    RTFloat apertuerR = (cam->fNumber <= 0.0) ? 0.0 : cam->focalLength / cam->fNumber * 0.0005; // [m]

    // circle sample
    RTFloat r = std::sqrt(rng->nextDoubleCC());
    RTFloat theta = rng->nextDoubleCO() * 2.0 * kPI;
    RTFloat sx = r * cos(theta);
    RTFloat sy = r * sin(theta);

    Vector3 o(sx * apertuerR, sy * apertuerR, 0.0);
    Vector3 d = p - o;
    d.normalize();

    return Ray(o, d);
}
