import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.animation as animation
import time

def extract_coordinates(filename, step=5):
    count = 0
    coordinates = []
    pattern = re.compile(r'POS,0,048F,([-\d.]+),([-\d.]+),([-\d.]+),([-\d]+),x06')

    with open(filename, 'r') as file:
        for i,line in enumerate(file):
            count += 1
            if i%step == 0:
                match = pattern.search(line)
                if match:
                    x, y, z, qf = map(float, match.groups())
                    coordinates.append((x, y, z))

    return coordinates

def compute_yaw_pitch(origin, target):
    direction = target - origin
    yaw = np.arctan2(direction[1], direction[0])
    pitch = np.arctan2(direction[2], np.sqrt(direction[0]**2 + direction[1]**2))
    return yaw, pitch

def compute_velocity(coords, delta_t=1.0):
    velocities = []
    for i in range(1, len(coords)):
        current_pos = np.array(coords[i])
        previous_pos = np.array(coords[i - 1])
        displacement = current_pos - previous_pos
        velocity = displacement / delta_t
        velocities.append(velocity)
    return np.array(velocities)

# Runge-Kutta prediction
def runge_kutta_predict(position, velocity, delta_t, acceleration_func=None):
    k1_pos = velocity
    k1_vel = acceleration_func(position, velocity) if acceleration_func else np.zeros_like(position)
    
    k2_pos = velocity + 0.5 * delta_t * k1_vel
    k2_vel = acceleration_func(position + 0.5 * delta_t * k1_pos, velocity + 0.5 * delta_t * k1_vel) if acceleration_func else np.zeros_like(position)
    
    k3_pos = velocity + 0.5 * delta_t * k2_vel
    k3_vel = acceleration_func(position + 0.5 * delta_t * k2_pos, velocity + 0.5 * delta_t * k2_vel) if acceleration_func else np.zeros_like(position)
    
    k4_pos = velocity + delta_t * k3_vel
    k4_vel = acceleration_func(position + delta_t * k3_pos, velocity + delta_t * k3_vel) if acceleration_func else np.zeros_like(position)
    
    new_position = position + (delta_t / 6) * (k1_pos + 2 * k2_pos + 2 * k3_pos + k4_pos)
    new_velocity = velocity + (delta_t / 6) * (k1_vel + 2 * k2_vel + 2 * k3_vel + k4_vel)
    
    return new_position, new_velocity

# Compare predicted and actual position (Euclidean distance error)
def calculate_error(predicted, actual):
    return np.linalg.norm(predicted - actual)

def update_plot(frame, predicted_coords=None):
    ax.cla()

    # Use actual coordinates for the path
    target = np.array(coords[frame])

    # If predicted coordinates are available, use them for spotlight direction
    if predicted_coords:
        predicted_target = np.array(predicted_coords[frame])
        yaw, pitch = compute_yaw_pitch(origin, predicted_target)  # Spotlight direction based on prediction
    else:
        yaw, pitch = compute_yaw_pitch(origin, target)  # Default to actual coordinates for spotlight direction

    forward = np.array([np.cos(yaw) * np.cos(pitch), np.sin(yaw) * np.cos(pitch), np.sin(pitch)])

    ax.set_xlim(-5, 5)
    ax.set_ylim(-5, 5)
    ax.set_zlim(-5, 5)
    ax.set_xlabel("X")
    ax.set_ylabel("Y")
    ax.set_zlabel("Z")
    ax.set_title("Tracked Path with Spotlight")

    anchors = {
        "A1": (-1.24, -0.79, 0.40),
        "A2": (1.24, -0.79, 0.70),
        "A3": (1.24, 0.79, 0.40),
        "A4": (-1.24, 0.79, 0.70)
    }
    for label, (ax_x, ax_y, ax_z) in anchors.items():
        ax.scatter(ax_x, ax_y, ax_z, color='red', s=100, label=label)
        ax.text(ax_x, ax_y, ax_z, label, color='black')

    # Plot the actual path
    x_vals, y_vals, z_vals = zip(*coords[:frame+1])
    ax.plot(x_vals, y_vals, z_vals, marker='o', linestyle='-', color='blue', label='Tracked Path', markersize=1)

    # Highlight the current actual position and spotlight
    ax.scatter(*target, color='b', s=100, label='Current Target')
    ax.scatter(*origin, color='r', s=100, label='Spotlight')
    ax.quiver(*origin, *forward, color='g', length=2, label='Look Direction')
    ax.legend()

    plt.draw()
    plt.pause(0.02)

def animate_spotlight(predicted_coords=None):
    for frame in range(len(coords)):
        update_plot(frame, predicted_coords)  # Pass predicted coordinates for spotlight direction
    plt.show()

def test_prediction_accuracy(coords, delta_t=1.0):
    velocities = compute_velocity(coords, delta_t)
    
    total_error = 0
    predicted_coords = []  # Store predicted coordinates

    start_time = time.time()  # Start timing

    for i in range(1, len(coords) - 1):
        current_pos = np.array(coords[i])
        next_pos = np.array(coords[i + 1])
        velocity = velocities[i - 1]
        
        # Predict next position using Runge-Kutta method
        predicted_pos, _ = runge_kutta_predict(current_pos, velocity, delta_t)
        predicted_coords.append(predicted_pos)
        
        # Calculate the error between predicted and actual position
        error = calculate_error(predicted_pos, next_pos)
        total_error += error
        
    end_time = time.time()  # End timing

    # Calculate average error
    average_error = total_error / (len(coords) - 2)  # Exclude first and last points
    time_taken = end_time - start_time  # Total time taken for predictions

    print(f"Average Prediction Error: {average_error:.4f}")
    print(f"Time Taken for Prediction (seconds): {time_taken:.4f}")
    
    animate_spotlight(predicted_coords)  # Animate with predicted coordinates for spotlight direction

if __name__ == "__main__":
    filename = "minicomOutput.txt"  # Ensure this file exists
    coords = extract_coordinates(filename, step=5)
    origin = np.array([0, -2, 2])
    
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    
    test_prediction_accuracy(coords)
