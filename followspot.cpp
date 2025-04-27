#include <ola/DmxBuffer.h>
#include <ola/Logging.h>
#include <ola/client/StreamingClient.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <string.h>
#include <cstdint>
#include <algorithm>

using namespace std;

struct Point {
    float x, y, z;
};

struct Tag {
    std::string id;
    Point position;
};

std::streampos last_pos = 0;

// Convert 3D coordinates to pitch and yaw
void calculate_pitch_yaw(const Point &spotlight, const Point &target, float &pitch, float &yaw) {
    float dx = target.x - spotlight.x;
    float dy = target.y - spotlight.y;
    float dz = target.z - spotlight.z;
    float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
    pitch = std::atan2(-dz, std::sqrt(dx * dx + dy * dy));  // vertical angle //change dy to dz???
    yaw = std::atan2(dy, dx);  // horizontal angle
    std::cout << "Pitch! " << pitch << endl;
    std::cout << "Yaw! " << yaw << endl;
}

// Normalize angle to DMX value
uint8_t pitch_to_dmx(float angle, float range) {
    float degree = angle *(180/M_PI);
    cout << "PITCH DEGREE: " << degree << endl;
    degree = std::clamp(degree,-135.0f,135.0f);
    //degree = -55;
    //float normalized = (((degree + 90.0f)*(127.0f-43.0f))/(180.0f));
    float normalized = -degree + 27;
    cout << "normal: " << normalized << endl;
    //dmx -= 95;
    return normalized;
}

uint8_t yaw_to_dmx(float angle, float range) {
    float degree = angle *(180/M_PI);
    
    cout << "YAW DEGREE: " << degree << endl;
    range = range *(180/M_PI);
    range = 230;
    cout << "YAW RANGE: " << range << endl;
    //degree = std::clamp(degree,-180.0f,180.0f);
    float normalized = (degree + range) / (2*range);
    cout << "normal: " << normalized << endl;
    return normalized * 255.0f;
}

// Parse coordinates file
bool parse_coordinates(const std::string &filename, Tag &tag) {

    return false;
}

int main() {
    ola::InitLogging(ola::OLA_LOG_WARN, ola::OLA_LOG_STDERR);
    ola::client::StreamingClient ola_client;
    if (!ola_client.Setup()) {
        std::cerr << "OLA client setup failed." << std::endl;
        return 1;
    }

    const Point spotlight = {-0.76f, 4.0f, 1.0f}; 
    Tag tracked_tag;

    const unsigned int universe = 0;
    const unsigned int pan_channel = 1;   // DMX channels start from 1
    const unsigned int tilt_channel = 3;
    const float pitch_range = M_PI / 2.0f;  // +/- 90 degrees
    const float yaw_range = M_PI;          // +/- 180 degrees
    
    string filename = "minicomOutput.txt";
    ola::DmxBuffer buffer;
    buffer.Blackout();
    usleep(1150000);
    //buffer.SetChannel(1, 87);
    usleep(1150000);
    std::ifstream file(filename);
    while(true){
        file.open(filename, std::ios::in);
        if (!file.is_open()){ 
            cout << "no file :(" << endl;
            return false;
        }
        else{
            //file.seekg(last_pos);
            std::string line;

            while (std::getline(file, line)) {
                //cout << "line" << endl;
                char id[5];
                float x, y, z;
                if (sscanf(line.c_str(), "POS,0,%4s,%f,%f,%f", id, &x, &y, &z)) {
                    tracked_tag.id = id;
                    if(x == x || y == y || z == z){
                        tracked_tag.position = {x, y, z};
                    }
                    //cout << tracked_tag.position.x << ", " << tracked_tag.position.y << ", " << tracked_tag.position.z << endl;
                }
            }
            file.close();
        }
        float pitch, yaw;
        calculate_pitch_yaw(spotlight, tracked_tag.position, pitch, yaw);

        unsigned int dmx_pan = yaw_to_dmx(yaw, yaw_range);
        unsigned int dmx_tilt = pitch_to_dmx(pitch, pitch_range);
        std::cout << "Pan! " << dmx_pan << endl;
        std::cout << "Tilt! " << dmx_tilt << endl;

        buffer.SetChannel(pan_channel, dmx_pan);
        buffer.SetChannel(tilt_channel, dmx_tilt);

        if (!ola_client.SendDmx(universe, buffer)) {
            std::cerr << "Failed to send DMX." << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    
    

    return 0;
}
