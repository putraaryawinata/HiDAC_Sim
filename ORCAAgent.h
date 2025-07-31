#ifndef _ORCA_AGENT_H_
#define _ORCA_AGENT_H_

#include "Agent.h"
#include <vector>
#include <algorithm>

// ORCA Line structure representing a linear constraint
struct ORCALine {
    v2f point;      // Point on the line
    v2f direction;  // Direction of the line (normalized)
};

class ORCAAgent : public Agent {
private:
    float timeHorizon;          // Time horizon for collision avoidance (default: 2.0)
    float timeHorizonObst;      // Time horizon for obstacle avoidance (default: 2.0)
    float neighborDist;         // Maximum distance to consider neighbors (default: 10.0)
    int maxNeighbors;           // Maximum number of neighbors to consider (default: 10)
    
    std::vector<ORCALine> orcaLines;  // ORCA constraints for this timestep
    v2f prefVelocity;           // Preferred velocity (toward goal)
    std::vector<const Agent*> currentNeighbors;  // Store current neighbors for collision avoidance
    
    // Helper functions for ORCA computation
    void computeORCALines(const std::vector<Agent*>& neighbors, float deltaT);
    void computeAgentORCA(const Agent* other, float deltaT);
    void computeObstacleORCA(const CrowdObject* obstacle, float deltaT);
    
    void computeNewVelocity(float deltaT, v2f result);
    bool linearProgram1(const std::vector<ORCALine>& lines, int lineNo, 
                       float radius, const v2f& optVelocity, bool dirOpt, v2f& result);
    int linearProgram2(const std::vector<ORCALine>& lines, float radius, 
                      const v2f& optVelocity, bool dirOpt, v2f& result);
    void linearProgram3(const std::vector<ORCALine>& lines, int numObstLines, 
                       int beginLine, float radius, v2f& result);
    
    float det(const v2f& vector1, const v2f& vector2);
    float distSqPointLineSegment(const v2f& vector1, const v2f& vector2, const v2f& vector3);

public:
    ORCAAgent();
    ORCAAgent(Json::Value a);
    ~ORCAAgent();
    
    // Override force calculation to use ORCA
    void calculateORCAVelocity(const std::vector<Agent*>& neighbors, float deltaT);
    
    // ORCA-specific setters
    void setTimeHorizon(float th) { timeHorizon = th; }
    void setTimeHorizonObst(float tho) { timeHorizonObst = tho; }
    void setNeighborDist(float nd) { neighborDist = nd; }
    void setMaxNeighbors(int mn) { maxNeighbors = mn; }
    
    // Goal-directed behavior
    void setPrefVelocity(const v2f& goal);
    void getPrefVelocity(v2f ret) { v2fCopy(prefVelocity, ret); }
    
    // Override physics to use velocity directly
    void applyForces(float deltaT);
};

#endif
