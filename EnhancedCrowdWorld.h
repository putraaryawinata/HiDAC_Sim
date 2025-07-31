#ifndef _ENHANCED_CROWD_WORLD_H_
#define _ENHANCED_CROWD_WORLD_H_

#include "CrowdWorld.h"
#include "ORCAAgent.h"
#include "DatasetLoader.h"
#include <memory>

enum SimulationMode {
    SOCIAL_FORCE,    // Original force-based simulation
    ORCA_SIMULATION, // ORCA-based simulation
    DATASET_PLAYBACK // ETH/UCY dataset playback
};

class EnhancedCrowdWorld : public CrowdWorld {
private:
    SimulationMode mode;
    std::unique_ptr<DatasetLoader> datasetLoader;
    std::vector<ORCAAgent*> orcaAgents;
    
    float currentTime;
    bool isPlaying;
    
    // ORCA-specific methods
    void initializeORCA();
    void updateORCA(float deltaT);
    
    // Dataset playback methods
    void initializeDatasetPlayback(const std::string& filename, const std::string& format);
    void updateDatasetPlayback(float deltaT);
    void syncAgentsWithDataset();
    
    // Agent management
    void createORCAAgent(const Json::Value& config);
    void createDatasetAgent(const TrajectoryPoint& point);
    void clearAgents();

public:
    EnhancedCrowdWorld();
    EnhancedCrowdWorld(const Json::Value& config);
    ~EnhancedCrowdWorld();
    
    // Mode management
    void setMode(SimulationMode newMode);
    SimulationMode getMode() const { return mode; }
    
    // Dataset functionality
    bool loadDataset(const std::string& filename, const std::string& format = "eth");
    void setDatasetParameters(float frameRate, float pixelToMeter);
    
    // Enhanced simulation control
    void reset();
    void play() { isPlaying = true; }
    void pause() { isPlaying = false; }
    void step(float deltaT);
    
    // Override base class methods
    void updateAgents() override;
    void calcForces() override;
    void stepWorld(float deltaT) override;
    
    // Statistics and analysis
    void exportTrajectories(const std::string& filename);
    void printSimulationStats();
    
    // ORCA configuration
    void setORCAParameters(float timeHorizon, float neighborDist, int maxNeighbors);
    
    // Getters
    float getCurrentTime() const { return currentTime; }
    bool getIsPlaying() const { return isPlaying; }
    int getCurrentFrame() const;
    
    // Comparison and evaluation
    void compareWithGroundTruth(const std::string& gtFilename);
    float calculateADE(const std::vector<TrajectoryPoint>& predicted, 
                      const std::vector<TrajectoryPoint>& groundTruth);
    float calculateFDE(const std::vector<TrajectoryPoint>& predicted, 
                      const std::vector<TrajectoryPoint>& groundTruth);
};

#endif
