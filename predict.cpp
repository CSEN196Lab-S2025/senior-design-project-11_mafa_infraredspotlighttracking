//update velocity
struct Tag {
    std::string id;
    Point position;
    Point velocity;
    Point last_position;
};

Point velocity;
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

if (speed > min_speed_threshold) {
    // Update spotlight pitch/yaw here
}


//Predict next movement
float prediction_time = 0.1f; // 100ms into the future
Point predicted_position;
predicted_position.x = tracked_tag.position.x + tracked_tag.velocity.x * prediction_time;
predicted_position.y = tracked_tag.position.y + tracked_tag.velocity.y * prediction_time;
predicted_position.z = tracked_tag.position.z + tracked_tag.velocity.z * prediction_time;
