#ifndef _DATASET_LOADER_H_
#define _DATASET_LOADER_H_

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include "Agent.h"
// Forward declarations to avoid circular includes
class CrowdWorld;

// Structure to hold trajectory data for a single agent
struct TrajectoryPoint {
    int frameId;
    int agentId;
    float x, y;
    float vx, vy;  // velocities (calculated)
};

// Structure to hold agent trajectory
struct AgentTrajectory {
    int agentId;
    std::vector<TrajectoryPoint> points;
    float startTime;
    float endTime;
};

// ETH/UCY dataset loader and simulator
class DatasetLoader {
private:
    std::vector<AgentTrajectory> trajectories;
    std::map<int, std::vector<TrajectoryPoint>> frameData;  // frameId -> agents in that frame
    
    float frameRate;  // frames per second (usually 2.5 fps for ETH/UCY)
    float pixelToMeter;  // conversion factor from pixels to meters
    int currentFrame;
    int maxFrame;
    
    // Parse different dataset formats
    bool parseETHFormat(const std::string& filename);
    bool parseUCYFormat(const std::string& filename); 
    bool parseTrajNetFormat(const std::string& filename);
    
    // Helper functions
    void calculateVelocities();
    void organizeByFrames();
    AgentTrajectory* findTrajectory(int agentId);
    
public:
    DatasetLoader();
    ~DatasetLoader();
    
    // Load dataset from file
    bool loadDataset(const std::string& filename, const std::string& format = "eth");
    
    // Configuration
    void setFrameRate(float fps) { frameRate = fps; }
    void setPixelToMeter(float ptm) { pixelToMeter = ptm; }
    
    // Simulation control
    void reset() { currentFrame = 0; }
    bool hasNextFrame() { return currentFrame <= maxFrame; }
    void nextFrame() { currentFrame++; }
    int getCurrentFrame() { return currentFrame; }
    int getMaxFrame() { return maxFrame; }
    
    // Get data for current frame
    std::vector<TrajectoryPoint> getCurrentFrameData();
    std::vector<TrajectoryPoint> getFrameData(int frameId);
    
    // Agent management
    std::vector<int> getActiveAgents(int frameId);
    TrajectoryPoint getAgentPosition(int agentId, int frameId);
    bool isAgentActive(int agentId, int frameId);
    
    // Dataset statistics
    int getNumAgents() { return trajectories.size(); }
    int getNumFrames() { return maxFrame + 1; }
    void printStatistics();
    
    // Export functionality
    bool exportToJson(const std::string& filename);
    
    // Integration with existing simulation
    void populateCrowdWorld(CrowdWorld& world, int frameId);
    Json::Value createAgentJson(const TrajectoryPoint& point);
};

// ETH/UCY format specifications:
// ETH Format: frame_id agent_id pos_x pos_y
// UCY Format: frame_id agent_id pos_x pos_y
// Both use 2.5 fps, coordinates in pixels
// Standard conversion: 1 pixel = 0.05 meters (varies by dataset)

#endif
