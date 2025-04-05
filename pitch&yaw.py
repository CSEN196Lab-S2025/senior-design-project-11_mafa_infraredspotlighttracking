import numpy as np
import matplotlib.pyplot as plt

def compute_yaw_pitch(origin, target):
    """Compute yaw (horizontal) and pitch (vertical) angles to make an object look at a target."""
    direction = target - origin  # Compute direction vector
    yaw = np.arctan2(direction[1], direction[0])  # Angle in XY plane
    pitch = np.arctan2(direction[2], np.sqrt(direction[0]**2 + direction[1]**2))  # Vertical angle
    return yaw, pitch  # Return angles in radians

def plot_spotlight(origin, target):
    """Plot the spotlight looking at the target with the computed yaw & pitch."""
    yaw, pitch = compute_yaw_pitch(origin, target)
    
    # Compute the spotlight's forward direction (unit vector)
    forward = np.array([np.cos(yaw) * np.cos(pitch), np.sin(yaw) * np.cos(pitch), np.sin(pitch)])

    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')

    # Plot origin and target
    ax.scatter(*origin, color='r', s=100, label='Spotlight (Origin)')
    ax.scatter(*target, color='b', s=100, label='Target')

    # Plot direction vector (spotlight's aim)
    ax.quiver(*origin, *forward, color='g', length=2, label='Look Direction')

    # Formatting
    ax.set_xlim(-5, 5)
    ax.set_ylim(-5, 5)
    ax.set_zlim(-5, 5)
    ax.set_xlabel("X")
    ax.set_ylabel("Y")
    ax.set_zlabel("Z")
    ax.legend()
    plt.show()

# Example
origin = np.array([0, 0, 10])  # Spotlight position
target = np.array([10, 3, 0])  # Target position

plot_spotlight(origin, target)
