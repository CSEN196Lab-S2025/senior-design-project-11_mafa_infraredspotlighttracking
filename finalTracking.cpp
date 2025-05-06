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
#include <csignal>
#include <atomic>

using namespace std;

struct Point {
    float x, y, z;
};

struct Tag {
    std::string id;
    Point position;
    Point velocity;
    Point last_position;
};

std::atomic<bool> running(true);

ola::DmxBuffer buffer;
ola::client::StreamingClient ola_client;

const unsigned int universe = 0;
const unsigned int pan_channel = 1;
const unsigned int tilt_channel = 3;
const unsigned int light_channel = 0;
 
// Convert 3D coordinates to pitch and yaw
void calculate_pitch_yaw(const Point &spotlight, const Point &target, float &pitch, float &yaw) {
    float dx = target.x - spotlight.x;
    float dy = target.y - spotlight.y;
    float dz = target.z - spotlight.z;
    float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
    pitch = std::atan2(dz, std::sqrt(dx * dx + dy * dy));  // vertical angle
    yaw = std::atan2(dx, dy);  // horizontal angle
    std::cout << "Pitch! " << pitch << endl;
    std::cout << "Yaw! " << yaw << endl;
}

// Normalize angle to DMX value
uint8_t pitch_to_dmx(float angle) {
    float degree = angle *(180/M_PI);
    
    cout << "PITCH DEGREE: " << degree << endl;

    float normalized = ((degree + 45) / 135) * 127;
    cout << "normalized tilt: " << normalized << endl;

    return normalized;
}  

uint8_t yaw_to_dmx(float angle) {
    float degree = angle  * (180/M_PI);
    cout << "YAW DEGREE: " << degree << endl;
    float offset;
    if(degree <=0){
	offset = 88;
    }
    else{
	offset = 89;
    }	
    float normalized = offset + ((degree / 540.0f) * 255);
    cout << "normalized pan: " << normalized << endl;
    normalized = ceil(normalized);
    return normalized;
}

void resetDMX() {
	cout << "resetting DMX values..." << endl;
	buffer.SetChannel(light_channel, 0);
	buffer.SetChannel(pan_channel, 0);
	buffer.SetChannel(tilt_channel, 0);

	if(!ola_client.SendDmx(universe, buffer)) {
		std::cerr << "Failed to send DMX reset." << endl;
	}
}

void signalHandler(int signum) {
	cout << "\nCtrl+C pressed." << endl;
        running = false;
}

int main() {
    signal(SIGINT, signalHandler);
    ola::InitLogging(ola::OLA_LOG_WARN, ola::OLA_LOG_STDERR);
    
    if (!ola_client.Setup()) {
        std::cerr << "OLA client setup failed." << std::endl;
        return 1;
    }

    const Point spotlight = {0.13f, -7.33f, 3.95f}; 
    Tag tracked_tag;

    string filename = "minicomOutput.txt";
    std::ifstream file(filename);

    if (!file.is_open()){ 
        cout << "no file :(" << endl;
        return false;
    }

    file.seekg(0, std::ios::end);

    while(running) {
        std::string line;

        if (std::getline(file, line)) {
            char id[5];
            float x, y, z;

            if (sscanf(line.c_str(), "POS,0,%4s,%f,%f,%f", id, &x, &y, &z)) {
                tracked_tag.id = id;

                if(x == x || y == y || z == z){
                    tracked_tag.position = {x, y, z};

                    float pitch, yaw;
                    calculate_pitch_yaw(spotlight, tracked_tag.position, pitch, yaw); //updated to predicted_position instead of tracked_tag.position

                    unsigned int dmx_pan = yaw_to_dmx(yaw);
                    unsigned int dmx_tilt = pitch_to_dmx(pitch);
                    std::cout << "Pan! " << dmx_pan << endl;
                    std::cout << "Tilt! " << dmx_tilt << endl;

                    buffer.SetChannel(pan_channel, dmx_pan);
                    buffer.SetChannel(tilt_channel, dmx_tilt);
                    buffer.SetChannel(light_channel, 255);
                    buffer.SetChannel(10,130);

                    if (!ola_client.SendDmx(universe, buffer)) {
                        std::cerr << "Failed to send DMX." << std::endl;
                    }
                }
            }
        }
        else {
            file.clear();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }    
    }

    resetDMX();
    return 0;
}

     
/*        Point velocity;
        velocity.x = (tracked_tag.position.x - tracked_tag.last_position.x) / delta_time;
        velocity.y = (tracked_tag.position.y - tracked_tag.last_position.y) / delta_time;
        velocity.z = (tracked_tag.position.z - tracked_tag.last_position.z) / delta_time;
        tracked_tag.velocity = velocity;
        tracked_tag.last_position = tracked_tag.position;

        //if we want to use speed to smooth movement
        float speed = std::sqrt(
            velocity.x * velocity.x +
            velocity.y * velocity.y +
            velocity.z * velocity.z
        );

        //Predict next movement
        float prediction_time = 0.1f; // 100ms into the future
        Point predicted_position;
        predicted_position.x = tracked_tag.position.x + tracked_tag.velocity.x * prediction_time;
        predicted_position.y = tracked_tag.position.y + tracked_tag.velocity.y * prediction_time;
        predicted_position.z = tracked_tag.position.z + tracked_tag.velocity.z * prediction_time; */
        
