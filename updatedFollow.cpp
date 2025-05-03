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

ola::DmxBuffer buffer;
ola::client::StreamingClient ola_client;

const unsigned int universe = 1;
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
uint8_t pitch_to_dmx(float angle, float range) {
    float degree = angle *(180/M_PI);
    
    cout << "PITCH DEGREE: " << degree << endl;

    float normalized = ((degree + 45) / 270) * 255;
    cout << "normalized tilt: " << normalized << endl;

    return normalized;
}  

uint8_t yaw_to_dmx(float angle, float range) {
    float degree = angle  * (180/M_PI);
    cout << "YAW DEGREE: " << degree << endl;

    float normalized = 88 + ((degree / 540.0f) * 255);
    cout << "normalized pan: " << normalized << endl;
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
	resetDMX();
	exit(signum);
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

    usleep(1150000);

    static float current_pan = 0;
    static float current_tilt = 0;

    std::ifstream file(filename);
    while(true){
        file.open(filename, std::ios::in);
        if (!file.is_open()){ 
            cout << "no file :(" << endl;
            return false;
        }
        else{
            std::string line;

            while (std::getline(file, line)) {
                char id[5];
                float x, y, z;
                if (sscanf(line.c_str(), "POS,0,%4s,%f,%f,%f", id, &x, &y, &z)) {
                    tracked_tag.id = id;
                    if(x == x || y == y || z == z){
                        tracked_tag.position = {x, y, z};
                    }
                }
            }
            file.close();
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
        
        float pitch, yaw;
        calculate_pitch_yaw(spotlight, tracked_tag.position, pitch, yaw); //updated to predicted_position instead of tracked_tag.position

        unsigned int dmx_pan = yaw_to_dmx(yaw, yaw_range);
        unsigned int dmx_tilt = pitch_to_dmx(pitch, pitch_range);
        std::cout << "Pan! " << dmx_pan << endl;
        std::cout << "Tilt! " << dmx_tilt << endl;

        buffer.SetChannel(pan_channel, dmx_pan);
        buffer.SetChannel(tilt_channel, dmx_tilt);
        buffer.SetChannel(light_channel, 146);

        if (!ola_client.SendDmx(universe, buffer)) {
            std::cerr << "Failed to send DMX." << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    return 0;
}
