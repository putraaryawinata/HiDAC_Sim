#include "ORCAAgent.h"
#include "constants.h"
#include <cmath>
#include <algorithm>

const float RVO_EPSILON = 0.00001f;

ORCAAgent::ORCAAgent() : Agent() {
    timeHorizon = 2.0f;
    timeHorizonObst = 2.0f;
    neighborDist = 10.0f;
    maxNeighbors = 10;
    v2fMult(prefVelocity, 0.0f, prefVelocity);
}

ORCAAgent::ORCAAgent(Json::Value a) : Agent(a) {
    timeHorizon = a.get("timeHorizon", 2.0f).asFloat();
    timeHorizonObst = a.get("timeHorizonObst", 2.0f).asFloat();
    neighborDist = a.get("neighborDist", 10.0f).asFloat();
    maxNeighbors = a.get("maxNeighbors", 10).asInt();
    
    v2fMult(prefVelocity, 0.0f, prefVelocity);
}

ORCAAgent::~ORCAAgent() {
}

void ORCAAgent::calculateORCAVelocity(const std::vector<Agent*>& neighbors, float deltaT) {
    // Simplified ORCA implementation
    v2f myPos, myVel;
    getPos(myPos);
    getVelocity(myVel);
    
    // Get current max velocity
    float currentMaxVel = getMaxVelocity();
    
    // Get goal direction from preferred velocity
    v2f goalDirection;
    v2fCopy(prefVelocity, goalDirection);
    v2fNormalize(goalDirection, goalDirection);
    
    // Simple collision avoidance with neighbors
    v2f avoidanceForce;
    v2fMult(avoidanceForce, 0.0f, avoidanceForce);
    
    int neighborCount = 0;
    for (Agent* neighbor : neighbors) {
        if (neighbor == this) continue;
        
        v2f neighborPos;
        neighbor->getPos(neighborPos);
        
        float distance = getDistance(neighborPos);
        if (distance < neighborDist && distance > 0.1f) {
            v2f avoidDirection;
            v2fSub(myPos, neighborPos, avoidDirection);
            v2fNormalize(avoidDirection, avoidDirection);
            
            // Weight by inverse distance
            float weight = 1.0f / (distance + 0.1f);
            v2f weightedAvoid;
            v2fMult(avoidDirection, weight, weightedAvoid);
            v2fAdd(avoidanceForce, weightedAvoid, avoidanceForce);
            neighborCount++;
        }
    }
    
    // Combine goal direction with avoidance
    v2f newVelocity;
    if (neighborCount > 0) {
        v2fNormalize(avoidanceForce, avoidanceForce);
        v2fMult(goalDirection, 0.5f, goalDirection);
        v2fMult(avoidanceForce, 0.5f, avoidanceForce);
        v2fAdd(goalDirection, avoidanceForce, newVelocity);
    } else {
        v2fCopy(goalDirection, newVelocity);
    }
    
    // Scale to max velocity
    float speed = v2fLen(newVelocity);
    if (speed > 0.001f) {
        v2fNormalize(newVelocity, newVelocity);
        v2fMult(newVelocity, fminf(speed, currentMaxVel), newVelocity);
    }
    
    setVelocity(newVelocity);
}

void ORCAAgent::setPrefVelocity(const v2f& goal) {
    v2f myPos, direction;
    getPos(myPos);
    
    // Create mutable copy of goal for v2fSub
    v2f goalCopy;
    goalCopy[0] = goal[0];
    goalCopy[1] = goal[1];
    
    v2fSub(goalCopy, myPos, direction);
    
    float distance = v2fLen(direction);
    float currentMaxVel = getMaxVelocity();
    
    if (distance > RVO_EPSILON) {
        v2fNormalize(direction, direction);
        v2fMult(direction, currentMaxVel, prefVelocity);
    } else {
        v2fMult(prefVelocity, 0.0f, prefVelocity);
    }
}

// Simplified implementations for the rest
void ORCAAgent::computeORCALines(const std::vector<Agent*>& neighbors, float deltaT) {
    // Simplified - not implemented
}

void ORCAAgent::computeAgentORCA(const Agent* other, float deltaT) {
    // Simplified - not implemented  
}

void ORCAAgent::computeNewVelocity(float deltaT, v2f result) {
    v2fCopy(prefVelocity, result);
}

bool ORCAAgent::linearProgram1(const std::vector<ORCALine>& lines, int lineNo, 
                              float radius, const v2f& optVelocity, bool dirOpt, v2f& result) {
    return true;
}

int ORCAAgent::linearProgram2(const std::vector<ORCALine>& lines, float radius, 
                             const v2f& optVelocity, bool dirOpt, v2f& result) {
    return lines.size();
}

void ORCAAgent::linearProgram3(const std::vector<ORCALine>& lines, int numObstLines, 
                              int beginLine, float radius, v2f& result) {
    // Simplified - not implemented
}

float ORCAAgent::det(const v2f& vector1, const v2f& vector2) {
    return vector1[0] * vector2[1] - vector1[1] * vector2[0];
}

void ORCAAgent::applyForces(float deltaT) {
    // For ORCA agents, directly integrate velocity to update position
    // Get current position and velocity
    v2f currentPos, currentVel;
    getPos(currentPos);
    getVelocity(currentVel);
    
    // Update position: newPos = oldPos + velocity * deltaT
    v2f deltaPos;
    v2fMult(currentVel, deltaT, deltaPos);
    v2fAdd(currentPos, deltaPos, currentPos);
    
    // Set new position
    setPos(currentPos);
}

float ORCAAgent::distSqPointLineSegment(const v2f& vector1, const v2f& vector2, const v2f& vector3) {
    return 0.0f; // Simplified
}
