//
//  node.cpp
//  Spectrenotes
//
//  Created by SatoruNAKAJIMA on 2019/08/16.
//

#include "node.h"
#include "mesh.h"
#include "tracablestructure.h"
#include "bvh.h"

#include "types.h"
using namespace Spectrenotes;

Node::Node(int i):
    index(i),
    contentType(kContentTypeEmpty),
    animatedFlag(0),
    parent(nullptr),
    isTransformDirty(false)
{
    // clear content
    memset(&content, 0, sizeof(content));

    // init transform
    initialTransform.globalMatrix.setIdentity();
    initialTransform.matrix.setIdentity();
    initialTransform.translate.set(0.0, 0.0, 0.0);
    initialTransform.rotation.set(0.0, 0.0, 0.0, 1.0);
    initialTransform.scale.set(1.0, 1.0, 1.0);
    currentTransform = initialTransform;
}

Node::~Node() {
    
}

Matrix4 Node::computeGlobalMatrix(RTTimeType tr) const {
    if(animatedFlag == 0) {
        return initialTransform.globalMatrix;
    } else {
        Matrix4 pgm;
        if(parent == nullptr) {
            pgm.setIdentity();
        } else {
            pgm = parent->computeGlobalMatrix(tr);
        }
        
        if((animatedFlag & Node::kAnimatedDirect) == 0) {
            return pgm * initialTransform.matrix;
        } else {
            int lastindex = static_cast<int>(transformCache.size() - 1);
            RTFloat ti = tr * static_cast<float>(lastindex);
            RTFloat t = std::floor(ti);
            int i0 = static_cast<int>(ti - t);
            int i1 = std::min(i0 + 1, lastindex);
            Transform tf = Transform::interpolate(transformCache[i0], transformCache[i1], t);
            tf.makeMatrix();
            return pgm * tf.matrix;
        }
    }
}

Node::Transform Node::Transform::interpolate(const Transform& tf0, const Transform& tf1, RTFloat t) {
    Transform ret;
    ret.translate = Vector3::lerp(tf0.translate, tf1.translate, t);
    ret.rotation = Quaterion::slerp(tf0.rotation, tf1.rotation, t);
    ret.scale = Vector3::lerp(tf0.scale, tf1.scale, t);
    ret.makeMatrix();
    return ret;
}

void Node::Transform::makeMatrix() {
    matrix.setIdentity();
    matrix.translate(translate);
    matrix = matrix * rotation.getMatrix();
    matrix.scale(scale);
}
