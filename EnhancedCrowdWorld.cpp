#include "EnhancedCrowdWorld.h"
#include <iostream>

EnhancedCrowdWorld::EnhancedCrowdWorld() : CrowdWorld() {
    mode = SOCIAL_FORCE;
    currentTime = 0.0f;
    isPlaying = false;
    datasetLoader = std::make_unique<DatasetLoader>();
}

EnhancedCrowdWorld::EnhancedCrowdWorld(const Json::Value& config) : CrowdWorld(config) {
    mode = SOCIAL_FORCE;
    currentTime = 0.0f;
    isPlaying = false;
    datasetLoader = std::make_unique<DatasetLoader>();
    
    // Check if mode is specified in config
    if (config.isMember("simulation") && config["simulation"].isMember("mode")) {
        std::string modeStr = config["simulation"]["mode"].asString();
        if (modeStr == "orca") {
            mode = ORCA_SIMULATION;
        } else if (modeStr == "dataset") {
            mode = DATASET_PLAYBACK;
        }
    }
}

EnhancedCrowdWorld::~EnhancedCrowdWorld() {
    clearAgents();
}

void EnhancedCrowdWorld::setMode(SimulationMode newMode) {
    mode = newMode;
    reset();
}

bool EnhancedCrowdWorld::loadDataset(const std::string& filename, const std::string& format) {
    return datasetLoader->loadDataset(filename, format);
}

void EnhancedCrowdWorld::setDatasetParameters(float frameRate, float pixelToMeter) {
    datasetLoader->setFrameRate(frameRate);
    datasetLoader->setPixelToMeter(pixelToMeter);
}

void EnhancedCrowdWorld::reset() {
    currentTime = 0.0f;
    isPlaying = false;
    
    if (mode == DATASET_PLAYBACK) {
        datasetLoader->reset();
    }
}

void EnhancedCrowdWorld::step(float deltaT) {
    if (!isPlaying) return;
    
    switch (mode) {
        case SOCIAL_FORCE:
            updateAgents();
            calcForces();
            stepWorld(deltaT);
            break;
            
        case ORCA_SIMULATION:
            updateORCA(deltaT);
            break;
            
        case DATASET_PLAYBACK:
            updateDatasetPlayback(deltaT);
            break;
    }
    
    currentTime += deltaT;
}

void EnhancedCrowdWorld::updateAgents() {
    if (mode == ORCA_SIMULATION) {
        // For ORCA, we handle updates differently
        return;
    }
    CrowdWorld::updateAgents();
}

void EnhancedCrowdWorld::calcForces() {
    if (mode == ORCA_SIMULATION) {
        // ORCA doesn't use forces
        return;
    }
    CrowdWorld::calcForces();
}

void EnhancedCrowdWorld::stepWorld(float deltaT) {
    if (mode == ORCA_SIMULATION || mode == DATASET_PLAYBACK) {
        // These modes handle stepping differently
        return;
    }
    CrowdWorld::stepWorld(deltaT);
}

void EnhancedCrowdWorld::updateORCA(float deltaT) {
    // Convert regular agents to ORCA agents if needed
    std::vector<Agent*> allAgents;
    for (Agent* agent : agentList) {
        allAgents.push_back(agent);
    }
    
    // Update each ORCA agent
    for (ORCAAgent* orcaAgent : orcaAgents) {
        orcaAgent->calculateORCAVelocity(allAgents, deltaT);
        orcaAgent->applyForces(deltaT);
    }
}

void EnhancedCrowdWorld::updateDatasetPlayback(float deltaT) {
    if (!datasetLoader->hasNextFrame()) {
        isPlaying = false;
        return;
    }
    
    syncAgentsWithDataset();
    datasetLoader->nextFrame();
}

void EnhancedCrowdWorld::syncAgentsWithDataset() {
    // Clear existing agents
    clearAgents();
    
    // Get current frame data
    std::vector<TrajectoryPoint> currentFrame = datasetLoader->getCurrentFrameData();
    
    // Create agents for current frame
    for (const TrajectoryPoint& point : currentFrame) {
        createDatasetAgent(point);
    }
}

void EnhancedCrowdWorld::createDatasetAgent(const TrajectoryPoint& point) {
    Json::Value agentConfig = datasetLoader->createAgentJson(point);
    Agent* agent = new Agent(agentConfig);
    agentList.push_back(agent);
    objectList.push_back(agent);
}

void EnhancedCrowdWorld::clearAgents() {
    // Clear ORCA agents
    for (ORCAAgent* agent : orcaAgents) {
        delete agent;
    }
    orcaAgents.clear();
    
    // Note: Regular agents are cleaned up by base class
}

void EnhancedCrowdWorld::setORCAParameters(float timeHorizon, float neighborDist, int maxNeighbors) {
    for (ORCAAgent* agent : orcaAgents) {
        agent->setTimeHorizon(timeHorizon);
        agent->setNeighborDist(neighborDist);
        agent->setMaxNeighbors(maxNeighbors);
    }
}

int EnhancedCrowdWorld::getCurrentFrame() const {
    if (mode == DATASET_PLAYBACK) {
        return datasetLoader->getCurrentFrame();
    }
    return (int)(currentTime * 2.5f); // Assume 2.5 fps default
}

void EnhancedCrowdWorld::exportTrajectories(const std::string& filename) {
    if (mode == DATASET_PLAYBACK) {
        datasetLoader->exportToJson(filename);
    } else {
        std::cout << "Trajectory export only available in dataset mode" << std::endl;
    }
}

void EnhancedCrowdWorld::printSimulationStats() {
    std::cout << "Simulation Statistics:" << std::endl;
    std::cout << "  Mode: ";
    switch (mode) {
        case SOCIAL_FORCE: std::cout << "Social Force"; break;
        case ORCA_SIMULATION: std::cout << "ORCA"; break;
        case DATASET_PLAYBACK: std::cout << "Dataset Playback"; break;
    }
    std::cout << std::endl;
    std::cout << "  Current time: " << currentTime << "s" << std::endl;
    std::cout << "  Number of agents: " << agentList.size() << std::endl;
    
    if (mode == DATASET_PLAYBACK) {
        std::cout << "  Current frame: " << getCurrentFrame() << std::endl;
        std::cout << "  Total frames: " << datasetLoader->getMaxFrame() + 1 << std::endl;
    }
}

float EnhancedCrowdWorld::calculateADE(const std::vector<TrajectoryPoint>& predicted, 
                                     const std::vector<TrajectoryPoint>& groundTruth) {
    // Average Displacement Error calculation
    float totalError = 0.0f;
    int count = 0;
    
    size_t minSize = std::min(predicted.size(), groundTruth.size());
    for (size_t i = 0; i < minSize; ++i) {
        float dx = predicted[i].x - groundTruth[i].x;
        float dy = predicted[i].y - groundTruth[i].y;
        totalError += sqrt(dx * dx + dy * dy);
        count++;
    }
    
    return count > 0 ? totalError / count : 0.0f;
}

float EnhancedCrowdWorld::calculateFDE(const std::vector<TrajectoryPoint>& predicted, 
                                     const std::vector<TrajectoryPoint>& groundTruth) {
    // Final Displacement Error calculation
    if (predicted.empty() || groundTruth.empty()) {
        return 0.0f;
    }
    
    const TrajectoryPoint& predFinal = predicted.back();
    const TrajectoryPoint& gtFinal = groundTruth.back();
    
    float dx = predFinal.x - gtFinal.x;
    float dy = predFinal.y - gtFinal.y;
    
    return sqrt(dx * dx + dy * dy);
}
