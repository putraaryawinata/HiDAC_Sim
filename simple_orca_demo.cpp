#include "Agent.h"
#include "ORCAAgent.h"
#include "CrowdObject.h"
#include "Wall.h"
#include "CrowdWorld.h"
#include "Render.h"
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <json/reader.h>
#include <json/value.h>
#include <stdio.h>
#include <time.h>
#include <vector>

using namespace std;

Json::Value readJsonFromFile(const char* file);
int mysleep(unsigned long millis);

Json::Value readJsonFromFile(const char* file) {
    Json::Reader reader;
    Json::Value root;
    std::ifstream data_file;
    data_file.open(file);
    
    if (!data_file.is_open()) {
        std::cout << "Could not open file: " << file << std::endl;
        exit(1);
    }
    
    std::string the_str(std::istreambuf_iterator<char>(data_file),
                       (std::istreambuf_iterator<char>()));
    
    data_file.close();
    
    bool success = reader.parse(the_str, root);
    
    if (!success) {
        std::cout << "bad json data!\n";
        std::cout << reader.getFormattedErrorMessages();
        exit(1);
    }
    
    return root;
}

int mysleep(unsigned long millis) {
    struct timespec req = {0};
    time_t sec = (int)(millis / 1000);
    millis = millis - (sec * 1000);
    req.tv_sec = sec;
    req.tv_nsec = millis * 1000000L;
    return nanosleep(&req, NULL);
}

int main(int argc, char** argv) {
    std::cout << "Starting ORCA Demonstration..." << std::endl;
    
    // Load configuration
    Json::Value data;
    if (argc == 2) {
        data = readJsonFromFile(argv[1]);
    } else {
        data = readJsonFromFile("data/orca_demo.json");
    }
    
    std::srand(0);
    
    // Create ORCA agents
    std::vector<ORCAAgent*> orcaAgents;
    for (const Json::Value& agentConfig : data["agents"]) {
        ORCAAgent* agent = new ORCAAgent(agentConfig);
        orcaAgents.push_back(agent);
        
        // Set preferred velocity toward attractor
        if (agentConfig.isMember("attractor")) {
            v2f attractorPos;
            attractorPos[0] = agentConfig["attractor"]["pos"][0u].asFloat();
            attractorPos[1] = agentConfig["attractor"]["pos"][1u].asFloat();
            agent->setPrefVelocity(attractorPos);
        }
        
        std::cout << "Created ORCA agent at position: ";
        v2f pos;
        agent->getPos(pos);
        std::cout << "(" << pos[0] << ", " << pos[1] << ")" << std::endl;
    }
    
    // Initialize rendering
    Render* r = Render::getInstance();
    while (!r->isInitialized()) {
        mysleep(10);
    }
    
    // Simulation parameters
    int steps = data.get("steps", 1000).asInt();
    float deltaT = data.get("timeslice", 0.4f).asFloat();
    
    std::cout << "Running ORCA simulation for " << steps << " steps..." << std::endl;
    
    // Convert to Agent* vector for compatibility
    std::vector<Agent*> allAgents;
    for (ORCAAgent* orca : orcaAgents) {
        allAgents.push_back(orca);
    }
    
    // Main simulation loop
    for (int i = 0; i < steps; ++i) {
        // Update each ORCA agent
        for (ORCAAgent* agent : orcaAgents) {
            agent->calculateORCAVelocity(allAgents, deltaT);
            agent->applyForces(deltaT);
        }
        
        // Print positions every 100 steps
        if (i % 100 == 0) {
            std::cout << "Step " << i << ":" << std::endl;
            for (size_t j = 0; j < orcaAgents.size(); ++j) {
                v2f pos;
                orcaAgents[j]->getPos(pos);
                std::cout << "  Agent " << j << ": (" << pos[0] << ", " << pos[1] << ")" << std::endl;
            }
        }
        
        // Update rendering
        r->update(deltaT);
        mysleep(10);
    }
    
    std::cout << "ORCA simulation completed!" << std::endl;
    
    // Cleanup
    for (ORCAAgent* agent : orcaAgents) {
        delete agent;
    }
    
    return 0;
}
