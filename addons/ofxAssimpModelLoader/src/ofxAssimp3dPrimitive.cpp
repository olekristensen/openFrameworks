//
//  ofxAssimpNode.cpp
//  bridge
//
//  Created by ole on 17/08/2018.
//

#include "ofxAssimp3dPrimitive.hpp"

int ofxAssimp3dPrimitive::counter = 0;
ofxAssimp3dPrimitive * ofxAssimp3dPrimitive::thePrimitive = nullptr;

ofxAssimp3dPrimitive::ofxAssimp3dPrimitive() {
    counter++;
    bDraw = true;
    level = 0;
    clearParent();
};

ofxAssimp3dPrimitive::ofxAssimp3dPrimitive(ofMesh &_mesh) : of3dPrimitive(_mesh) {
    counter++;
    level = 0;
    clearParent();
    calculateBoundingBox();
};

ofxAssimp3dPrimitive::ofxAssimp3dPrimitive(ofMesh &_mesh, ofxAssimp3dPrimitive& vnParent) : of3dPrimitive(_mesh) {
    counter++;
    bDraw = true;
    
    level = vnParent.level+1;
    
    // make the mesh vertices local for the node
    // move th relative position to the nodes position
    // transformations on the unit are around the centroid of the mesh not the centroid of the parent
    
    calculateBoundingBox();
    
    setParent(vnParent);
    
};

ofxAssimp3dPrimitive::ofxAssimp3dPrimitive(ofxAssimp3dPrimitive& vnParent) {
    counter++;
    bDraw = true;
    
    level = vnParent.level+1;
    
    // make the mesh vertices local for the node
    // move th relative position to the nodes position
    // transformations on the unit are around the centroid of the mesh not the centroid of the parent
        
    setParent(vnParent);
    
};


void ofxAssimp3dPrimitive::calculateBoundingBox() {
    
    minBounds = mesh->getVertex(0);
    maxBounds = mesh->getVertex(0);
    
    for(int i=0; i<mesh->getNumVertices(); i++) {
        
        minBounds.x = min(mesh->getVertex(i).x, minBounds.x);
        minBounds.y = min(mesh->getVertex(i).y, minBounds.y);
        minBounds.z = min(mesh->getVertex(i).z, minBounds.z);
        maxBounds.x = max(mesh->getVertex(i).x, maxBounds.x);
        maxBounds.y = max(mesh->getVertex(i).y, maxBounds.y);
        maxBounds.z = max(mesh->getVertex(i).z, maxBounds.z);
    }
    
    
    float width = abs(maxBounds.x - minBounds.x);
    float height = abs(maxBounds.y - minBounds.y);
    float depth = abs(maxBounds.z - minBounds.z);
    
    boundingBox.set(width, height, depth);
    boundingBox.setPosition(maxBounds.getMiddle(minBounds));
}

void ofxAssimp3dPrimitive::setParent(ofNode& p, bool bMaintainGlobalTransform){
    if(this->parent != nullptr){
        clearParent(bMaintainGlobalTransform);
    }
    if (ofxAssimp3dPrimitive* newVoroParent = dynamic_cast<ofxAssimp3dPrimitive *>(&p)){
        // our new parent is a ofxAssimp3dPrimitive
        newVoroParent->addChild(*this);
    }
    ofNode::setParent(p, bMaintainGlobalTransform);
};

void ofxAssimp3dPrimitive::clearParent(bool bMaintainGlobalTransform){
    if(parent){
        // we have a parent allready
        if (ofxAssimp3dPrimitive* oldVoroParent = dynamic_cast<ofxAssimp3dPrimitive*>(this->parent)) {
            // our old parent is also ofxAssimp3dPrimitive
            // remove this from parents' children.
            oldVoroParent->removeChild(*this);
        }
        ofNode::clearParent(bMaintainGlobalTransform);
    }
}

void ofxAssimp3dPrimitive::removeChild(ofxAssimp3dPrimitive & vnChild){
    primitiveChildren.erase(&vnChild);
    bChildrenChangedUpdateVector = true;
}

void ofxAssimp3dPrimitive::addChild(ofxAssimp3dPrimitive & vnChild){
    primitiveChildren.insert(&vnChild);
    bChildrenChangedUpdateVector = true;
    
}

void ofxAssimp3dPrimitive::clearChildren(){
    for_each(primitiveChildren.begin(), primitiveChildren.end(), del_fun<ofxAssimp3dPrimitive>());
    primitiveChildren.clear();
    bChildrenChangedUpdateVector = true;
    
}

vector<ofxAssimp3dPrimitive *> ofxAssimp3dPrimitive::getChildren() {
    
    if(bChildrenChangedUpdateVector) {
        primitiveChildrenVector = vector<ofxAssimp3dPrimitive *> (primitiveChildren.begin(), primitiveChildren.end());
        bChildrenChangedUpdateVector = false;
    }
    
    
    return primitiveChildrenVector;
};


ofMesh & ofxAssimp3dPrimitive::getBakedMesh() {
    
    bakedMesh = *mesh;
    // todo cache if node didn't change
    int iv = 0;
    for(auto v : mesh->getVertices()) {
        bakedMesh.setVertex(iv++, toOf(v) * toOf(getGlobalTransformMatrix()) );
    }
    
    return bakedMesh;
}

vector<ofMesh> ofxAssimp3dPrimitive::getBakedMeshesRecursive(){
    vector<ofMesh> retVector;
    retVector.push_back(getBakedMesh());
    for (auto child : getChildren()) {
        auto list = child->getBakedMeshesRecursive();
        for (auto grandchild : list){
            retVector.push_back(grandchild);
        }
    }
    return retVector;
}


vector<ofxAssimp3dPrimitive *> ofxAssimp3dPrimitive::getNearestChildren(ofPoint point, int maxNum, bool recursive) {
    
    vector<ofxAssimp3dPrimitive *> sortme;
    
    for(auto n : primitiveChildren) {
        sortme.push_back(n);
        
        // Todo: make recursive
        /*if(recursive) {
         for(auto nn : n->getChildren()) {
         sortme.push_back(nn);
         }
         sortme.insert(n->getChildren().begin(), n->getChildren().end());
         }*/
    }
    
    std::sort(sortme.begin(), sortme.end(), [point](ofxAssimp3dPrimitive * x, ofxAssimp3dPrimitive * y){ return toOf(x->getGlobalPosition()).distance(point) < toOf(y->getGlobalPosition()).distance(point); });
    
    vector<ofxAssimp3dPrimitive *> select(sortme.begin(), sortme.begin()+maxNum);
    
    return select;
}

// global flag ?
set<ofxAssimp3dPrimitive *> ofxAssimp3dPrimitive::getChildrenInSphere(ofPoint point, float radius, bool recursive) {
    
    set<ofxAssimp3dPrimitive *> select;
    
    for(auto n : primitiveChildren) {
        if(toOf(n->getGlobalPosition()).distance(point) < radius) {
            select.insert(n);
        }
        
        if(recursive) {
            set<ofxAssimp3dPrimitive *> rSelect = n->getChildrenInSphere(point, radius);
            select.insert(rSelect.begin(), rSelect.end());
        }
    }
    
    return select;
}

void ofxAssimp3dPrimitive::update() {
    
    for(auto c : getChildren()) {
        c->update();
    }
};

void ofxAssimp3dPrimitive::recursiveDraw() {
    of3dPrimitive::draw();
    for(auto c : getChildren()) {
        c->recursiveDraw();
    }
};


ofxAssimp3dPrimitive::~ofxAssimp3dPrimitive() {
    
    if(this->parent){
        ofNode::clearParent();
    }
    
    clearChildren();
    counter--;
    
};





