//
//  ofxAssimpNode.hpp
//  bridge
//
//  Created by ole on 17/08/2018.
//

#pragma once

#include "ofMain.h"

class ofxAssimp3dPrimitive : public of3dPrimitive {
    
public:
    
    static ofxAssimp3dPrimitive * thePrimitive;
    static int counter;
    
    ofxAssimp3dPrimitive();
    ofxAssimp3dPrimitive(ofMesh &_mesh);
    ofxAssimp3dPrimitive(ofMesh &_mesh, ofxAssimp3dPrimitive& vnParent);
    ofxAssimp3dPrimitive(ofxAssimp3dPrimitive& vnParent);

    ~ofxAssimp3dPrimitive();
    
    template<class T>
    struct del_fun_t
    {
        del_fun_t& operator()(T* p) {
            delete p;
            p = NULL;
            return *this;
        }
    };
    
    template<class T>
    del_fun_t<T> del_fun()
    {
        return del_fun_t<T>();
    }
    
    ofVec3f minBounds;
    ofVec3f maxBounds;
    ofBoxPrimitive boundingBox;
    bool bDraw;
    int level;
    string name;
    ofMesh bakedMesh;
    
    // useful data structure for transitions
    ofVec3f positionOverideTarget;
    float positionOverideAmount = 0;
    ofVec3f positionOverideOrigin;
    
    bool bOffsetPositionOveride = false;
    ofxAssimp3dPrimitive * transitionRef = nullptr;
    
    ofVec3f renderRotation;
    bool bRenderRotationSet = false;
    void setRenderRotation(ofVec3f rot) {
        renderRotation = rot;
        bRenderRotationSet = true;
    }
    
    ofVec3f renderPosOffset;
    ofVec3f renderScale;
    bool bRenderScaleSet = false;
    ofVec3f velocity;
    
    std::set<ofxAssimp3dPrimitive*> primitiveChildren;
    vector<ofxAssimp3dPrimitive *> primitiveChildrenVector;
    bool bChildrenChangedUpdateVector;
    
    void calculateBoundingBox();
    
    void setParent(ofNode& p, bool bMaintainGlobalTransform = false );
    
    /// \brief Remove parent node linking
    void clearParent(bool bMaintainGlobalTransform = false);
    
    void removeChild(ofxAssimp3dPrimitive & vnChild);
    
    void addChild(ofxAssimp3dPrimitive & vnChild);
    
    void clearChildren();
    
    vector<ofxAssimp3dPrimitive *> getChildren();
    
    set<ofxAssimp3dPrimitive *> getChildrenInSphere(ofPoint point, float radius, bool recursive = false);
    
    vector<ofxAssimp3dPrimitive *> getNearestChildren(ofPoint point, int maxNum=1, bool recursive = false);
    
    void update();
    
    void recursiveDraw();
    
    ofMesh & getBakedMesh();
    vector<ofMesh> getBakedMeshesRecursive();
    
    template<typename meshType>
    static meshType bakeMesh(meshType _m, const ofNode & _node) {
        
        // todo cache if node didn't change
        for(auto & v : _m.getVertices()) {
            v.set(v * _node.getGlobalTransformMatrix());
        }
        
        return _m;
    }
    
};
