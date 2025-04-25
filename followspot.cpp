#include <ola/DmxBuffer.h>
#include <ola/Logging.h>
#include <ola/client/StreamingClient.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <unistd.h>

using namespace std;

struct Point {
    float x, y, z;
};

struct Tag {
    std::string id;
    Point position;
};

// Convert 3D coordinates to pitch and yaw
void calculate_pitch_yaw(const Point &spotlight, const Point &target, float &pitch, float &yaw) {
    float dx = target.x - spotlight.x;
    float dy = target.y - spotlight.y;
    float dz = target.z - spotlight.z;
    float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
    
    pitch = std::atan2(dz, std::sqrt(dx * dx + dy * dy));  // vertical angle
    yaw = std::atan2(dy, dx);  // horizontal angle
    std::cout << "Pitch! " << pitch << endl;
    std::cout << "Yaw! " << yaw << endl;
}

// Normalize angle to DMX value
uint8_t angle_to_dmx(float angle, float range) {
    float normalized = (angle + range / 2.0f) / range;
    return static_cast<uint8_t>(std::max(0.0f, std::min(1.0f, normalized)) * 255);
}

// Parse coordinates file
bool parse_coordinates(const std::string &filename, Tag &tag) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        char id[5];
        float x, y, z;
        if (sscanf(line.c_str(), "POS,0,%4s,%f,%f,%f", id, &x, &y, &z) == 4) {
            tag.id = id;
            tag.position = {x, y, z};
            return true;
        }
    }
    return false;
}

int main() {
    ola::InitLogging(ola::OLA_LOG_WARN, ola::OLA_LOG_STDERR);
    ola::client::StreamingClient ola_client;
    if (!ola_client.Setup()) {
        std::cerr << "OLA client setup failed." << std::endl;
        return 1;
    }

    const Point spotlight = {0.76f, -4.0f, 1.0f};  // spotlight is 2 meters above center
    Tag tracked_tag;

    const unsigned int universe = 0;
    const unsigned int pan_channel = 2;   // DMX channels start from 1
    const unsigned int tilt_channel = 4;
    const float pitch_range = M_PI / 2.0f;  // +/- 90 degrees
    const float yaw_range = M_PI;          // +/- 180 degrees
    
    ola::DmxBuffer buffer;
    buffer.Blackout();
    usleep(1150000);
    buffer.SetChannel(2, 87);
    usleep(1150000);
    while (true) {
        if (!parse_coordinates("minicomOutput.txt", tracked_tag)) {
            std::cerr << "Failed to parse coordinates." << std::endl;
            usleep(100000);
            continue;
        }

        float pitch, yaw;
        calculate_pitch_yaw(spotlight, tracked_tag.position, pitch, yaw);

        uint8_t dmx_pan = angle_to_dmx(yaw, yaw_range);
        uint8_t dmx_tilt = angle_to_dmx(pitch, pitch_range);
        std::cout << "Pan! " << dmx_pan << endl;
        std::cout << "Tilt! " << dmx_tilt << endl;

        buffer.SetChannel(pan_channel, dmx_pan);
        buffer.SetChannel(tilt_channel, dmx_tilt);

        if (!ola_client.SendDmx(universe, buffer)) {
            std::cerr << "Failed to send DMX." << std::endl;
        }

        usleep(1150000); // 20 fps
    }

    return 0;
}