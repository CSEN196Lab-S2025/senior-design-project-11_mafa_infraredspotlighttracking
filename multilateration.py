import numpy as np

def multilateration(anchors, distances):
    """
    Solve for (x, y, z) using multilateration.
    
    anchors: List of (x, y, z) coordinates for each anchor.
    distances: List of distances from the tag to each anchor.
    
    Returns estimated (x, y, z).
    """
    num_anchors = len(anchors)
    
    if num_anchors < 4:
        raise ValueError("At least 4 anchors are required for 3D multilateration.")
    
    # Convert input to NumPy arrays
    anchors = np.array(anchors)
    distances = np.array(distances)
    
    # Choose the first anchor as reference (anchor 0)
    x0, y0, z0 = anchors[0]
    d0 = distances[0]
    
    # Formulate the system of equations
    A = []
    b = []
    
    for i in range(1, num_anchors):
        x_i, y_i, z_i = anchors[i]
        d_i = distances[i]
        
        # Linearized equation: (x-x0)^2 + (y-y0)^2 + (z-z0)^2 - d0^2 = (x-xi)^2 + (y-yi)^2 + (z-zi)^2 - di^2
        A.append([2*(x_i - x0), 2*(y_i - y0), 2*(z_i - z0)])
        b.append(d0**2 - d_i**2 - x0**2 + x_i**2 - y0**2 + y_i**2 - z0**2 + z_i**2)
    
    # Solve the linear system Ax = b using Least Squares
    A = np.array(A)
    b = np.array(b)
    
    position = np.linalg.lstsq(A, b, rcond=None)[0]
    
    return tuple(position)

# Example anchor positions (x, y, z) in millimeters
anchors = [
    (0, 0, 0),
    (5000, 0, 0),
    (0, 5000, 0),
    (5000, 5000, 3000)
]

# Example distances (in mm)
distances = [4242, 2828, 2828, 5291]

# Compute tag position
tag_position = multilateration(anchors, distances)
print("Estimated Tag Position:", tag_position)
