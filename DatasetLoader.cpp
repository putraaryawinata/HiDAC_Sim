#include "DatasetLoader.h"
#include "CrowdWorld.h"
#include <iostream>
#include <algorithm>
#include <json/json.h>

DatasetLoader::DatasetLoader() {
    frameRate = 2.5f;  // Standard ETH/UCY frame rate
    pixelToMeter = 0.05f;  // Standard conversion (may need adjustment per dataset)
    currentFrame = 0;
    maxFrame = 0;
}

DatasetLoader::~DatasetLoader() {
}

bool DatasetLoader::loadDataset(const std::string& filename, const std::string& format) {
    trajectories.clear();
    frameData.clear();
    currentFrame = 0;
    maxFrame = 0;
    
    bool success = false;
    
    if (format == "eth" || format == "ucy") {
        success = parseETHFormat(filename);  // Both use similar format
    } else if (format == "trajnet") {
        success = parseTrajNetFormat(filename);
    } else {
        std::cerr << "Unknown dataset format: " << format << std::endl;
        return false;
    }
    
    if (success) {
        calculateVelocities();
        organizeByFrames();
        std::cout << "Successfully loaded dataset: " << filename << std::endl;
        printStatistics();
    }
    
    return success;
}

bool DatasetLoader::parseETHFormat(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << filename << std::endl;
        return false;
    }
    
    std::string line;
    std::map<int, AgentTrajectory> agentMap;
    
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        int frameId, agentId;
        float x, y;
        
        if (!(iss >> frameId >> agentId >> x >> y)) {
            continue;  // Skip malformed lines
        }
        
        // Convert from pixels to meters
        x *= pixelToMeter;
        y *= pixelToMeter;
        
        TrajectoryPoint point;
        point.frameId = frameId;
        point.agentId = agentId;
        point.x = x;
        point.y = y;
        point.vx = 0.0f;  // Will be calculated later
        point.vy = 0.0f;
        
        // Add to agent's trajectory
        if (agentMap.find(agentId) == agentMap.end()) {
            AgentTrajectory traj;
            traj.agentId = agentId;
            traj.startTime = frameId / frameRate;
            agentMap[agentId] = traj;
        }
        
        agentMap[agentId].points.push_back(point);
        maxFrame = std::max(maxFrame, frameId);
    }
    
    // Convert map to vector and set end times
    for (auto& pair : agentMap) {
        AgentTrajectory& traj = pair.second;
        if (!traj.points.empty()) {
            traj.endTime = traj.points.back().frameId / frameRate;
            trajectories.push_back(traj);
        }
    }
    
    file.close();
    return true;
}

bool DatasetLoader::parseTrajNetFormat(const std::string& filename) {
    // TrajNet format is typically JSON-based
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << filename << std::endl;
        return false;
    }
    
    Json::Value root;
    Json::Reader reader;
    
    if (!reader.parse(file, root)) {
        std::cerr << "Failed to parse JSON: " << reader.getFormatedErrorMessages() << std::endl;
        return false;
    }
    
    // Parse TrajNet JSON format
    // Format: {"scene": {"id": x, "s": start_frame, "e": end_frame}, 
    //          "tracks": [{"f": frame, "p": person_id, "x": x, "y": y}, ...]}
    
    if (!root.isMember("tracks")) {
        std::cerr << "Invalid TrajNet format: missing tracks" << std::endl;
        return false;
    }
    
    std::map<int, AgentTrajectory> agentMap;
    
    for (const Json::Value& track : root["tracks"]) {
        int frameId = track["f"].asInt();
        int agentId = track["p"].asInt();
        float x = track["x"].asFloat() * pixelToMeter;
        float y = track["y"].asFloat() * pixelToMeter;
        
        TrajectoryPoint point;
        point.frameId = frameId;
        point.agentId = agentId;
        point.x = x;
        point.y = y;
        point.vx = 0.0f;
        point.vy = 0.0f;
        
        if (agentMap.find(agentId) == agentMap.end()) {
            AgentTrajectory traj;
            traj.agentId = agentId;
            traj.startTime = frameId / frameRate;
            agentMap[agentId] = traj;
        }
        
        agentMap[agentId].points.push_back(point);
        maxFrame = std::max(maxFrame, frameId);
    }
    
    // Convert to vector
    for (auto& pair : agentMap) {
        AgentTrajectory& traj = pair.second;
        if (!traj.points.empty()) {
            traj.endTime = traj.points.back().frameId / frameRate;
            trajectories.push_back(traj);
        }
    }
    
    file.close();
    return true;
}

void DatasetLoader::calculateVelocities() {
    for (AgentTrajectory& traj : trajectories) {
        for (size_t i = 0; i < traj.points.size(); ++i) {
            if (i == 0) {
                // First point - use forward difference
                if (traj.points.size() > 1) {
                    float dt = (traj.points[1].frameId - traj.points[0].frameId) / frameRate;
                    traj.points[i].vx = (traj.points[1].x - traj.points[0].x) / dt;
                    traj.points[i].vy = (traj.points[1].y - traj.points[0].y) / dt;
                }
            } else if (i == traj.points.size() - 1) {
                // Last point - use backward difference
                float dt = (traj.points[i].frameId - traj.points[i-1].frameId) / frameRate;
                traj.points[i].vx = (traj.points[i].x - traj.points[i-1].x) / dt;
                traj.points[i].vy = (traj.points[i].y - traj.points[i-1].y) / dt;
            } else {
                // Middle point - use central difference
                float dt = (traj.points[i+1].frameId - traj.points[i-1].frameId) / frameRate;
                traj.points[i].vx = (traj.points[i+1].x - traj.points[i-1].x) / dt;
                traj.points[i].vy = (traj.points[i+1].y - traj.points[i-1].y) / dt;
            }
        }
    }
}

void DatasetLoader::organizeByFrames() {
    frameData.clear();
    
    for (const AgentTrajectory& traj : trajectories) {
        for (const TrajectoryPoint& point : traj.points) {
            frameData[point.frameId].push_back(point);
        }
    }
}

std::vector<TrajectoryPoint> DatasetLoader::getCurrentFrameData() {
    return getFrameData(currentFrame);
}

std::vector<TrajectoryPoint> DatasetLoader::getFrameData(int frameId) {
    if (frameData.find(frameId) != frameData.end()) {
        return frameData[frameId];
    }
    return std::vector<TrajectoryPoint>();
}

std::vector<int> DatasetLoader::getActiveAgents(int frameId) {
    std::vector<int> agentIds;
    std::vector<TrajectoryPoint> points = getFrameData(frameId);
    
    for (const TrajectoryPoint& point : points) {
        agentIds.push_back(point.agentId);
    }
    
    return agentIds;
}

TrajectoryPoint DatasetLoader::getAgentPosition(int agentId, int frameId) {
    std::vector<TrajectoryPoint> points = getFrameData(frameId);
    
    for (const TrajectoryPoint& point : points) {
        if (point.agentId == agentId) {
            return point;
        }
    }
    
    // Return empty point if not found
    TrajectoryPoint empty;
    empty.frameId = frameId;
    empty.agentId = agentId;
    empty.x = empty.y = empty.vx = empty.vy = 0.0f;
    return empty;
}

bool DatasetLoader::isAgentActive(int agentId, int frameId) {
    std::vector<TrajectoryPoint> points = getFrameData(frameId);
    
    for (const TrajectoryPoint& point : points) {
        if (point.agentId == agentId) {
            return true;
        }
    }
    return false;
}

void DatasetLoader::printStatistics() {
    std::cout << "Dataset Statistics:" << std::endl;
    std::cout << "  Number of agents: " << trajectories.size() << std::endl;
    std::cout << "  Number of frames: " << maxFrame + 1 << std::endl;
    std::cout << "  Frame rate: " << frameRate << " fps" << std::endl;
    std::cout << "  Pixel to meter ratio: " << pixelToMeter << std::endl;
    std::cout << "  Duration: " << (maxFrame / frameRate) << " seconds" << std::endl;
    
    // Calculate average trajectory length
    float avgLength = 0.0f;
    for (const AgentTrajectory& traj : trajectories) {
        avgLength += traj.points.size();
    }
    avgLength /= trajectories.size();
    std::cout << "  Average trajectory length: " << avgLength << " points" << std::endl;
}

Json::Value DatasetLoader::createAgentJson(const TrajectoryPoint& point) {
    Json::Value agent;
    
    // Basic agent configuration compatible with existing system
    agent["pos"] = Json::Value(Json::arrayValue);
    agent["pos"].append(point.x);
    agent["pos"].append(point.y);
    
    agent["vel"] = Json::Value(Json::arrayValue);
    agent["vel"].append(point.vx);
    agent["vel"].append(point.vy);
    
    // Default parameters
    agent["atWeight"] = 0.5;
    agent["waWeight"] = 0.6;
    agent["obWeight"] = 0.5;
    agent["accel"] = 0.2;
    agent["maxVel"] = 1.5;
    agent["visDist"] = 3.0;
    agent["visWid"] = 2.0;
    agent["pspace"] = 0.4;
    agent["radius"] = 0.25;
    agent["mesh"] = "blue.mesh";
    
    // Set attractor to next position if available
    AgentTrajectory* traj = findTrajectory(point.agentId);
    if (traj) {
        // Find next point
        for (size_t i = 0; i < traj->points.size() - 1; ++i) {
            if (traj->points[i].frameId == point.frameId) {
                agent["attractor"] = Json::Value();
                agent["attractor"]["type"] = "attractor";
                agent["attractor"]["pos"] = Json::Value(Json::arrayValue);
                agent["attractor"]["pos"].append(traj->points[i+1].x);
                agent["attractor"]["pos"].append(traj->points[i+1].y);
                agent["attractor"]["norm"] = Json::Value(Json::arrayValue);
                agent["attractor"]["norm"].append(0.0);
                agent["attractor"]["norm"].append(0.0);
                break;
            }
        }
    }
    
    return agent;
}

AgentTrajectory* DatasetLoader::findTrajectory(int agentId) {
    for (AgentTrajectory& traj : trajectories) {
        if (traj.agentId == agentId) {
            return &traj;
        }
    }
    return nullptr;
}

bool DatasetLoader::exportToJson(const std::string& filename) {
    Json::Value root;
    
    // Export configuration
    root["steps"] = maxFrame + 1;
    root["timeslice"] = 1.0f / frameRate;
    root["frameRate"] = frameRate;
    root["pixelToMeter"] = pixelToMeter;
    
    // Export first frame as example
    std::vector<TrajectoryPoint> firstFrame = getFrameData(0);
    root["agents"] = Json::Value(Json::arrayValue);
    
    for (const TrajectoryPoint& point : firstFrame) {
        root["agents"].append(createAgentJson(point));
    }
    
    // Export trajectory data
    root["trajectories"] = Json::Value(Json::arrayValue);
    for (const AgentTrajectory& traj : trajectories) {
        Json::Value trajJson;
        trajJson["agentId"] = traj.agentId;
        trajJson["startTime"] = traj.startTime;
        trajJson["endTime"] = traj.endTime;
        trajJson["points"] = Json::Value(Json::arrayValue);
        
        for (const TrajectoryPoint& point : traj.points) {
            Json::Value pointJson;
            pointJson["frame"] = point.frameId;
            pointJson["x"] = point.x;
            pointJson["y"] = point.y;
            pointJson["vx"] = point.vx;
            pointJson["vy"] = point.vy;
            trajJson["points"].append(pointJson);
        }
        
        root["trajectories"].append(trajJson);
    }
    
    // Write to file
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open output file: " << filename << std::endl;
        return false;
    }
    
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "  ";
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(root, &file);
    
    file.close();
    return true;
}
