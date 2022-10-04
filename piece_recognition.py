"""
author: EMNEM
date-created: 10/2/22
last-modified: 10/2/22

piece_recognition.py
This file contains helper methods for detecting checkers pieces
"""

import cv2
import numpy as np
import time

# The max distance in pixels squared
CLUSTER_MAX_RANGE = 10**2

# Contains info about group of pixels and how to add new ones
# point is tuple of (x, y, type), where 0 = blue, 1 = red, 2 = yellow
class Cluster:
    def __init__(self):
        self.red = 0
        self.blue = 0
        self.yellow = 0
        self.is_blue = None
        self.is_king = None
        self.points = []
        self.x_sum = 0
        self.y_sum = 0
        self.x = None
        self.y = None
        self.is_valid = False
        return
    
    # Adds point to self
    def add_point(self, point):
        # Update sumes
        self.points.append(point)
        self.x_sum += point[0]
        self.y_sum += point[1]
        # Check color
        if point[2] == 0:
            self.blue += 1
        elif point[2] == 1:
            self.red += 1
        else:
            self.yellow += 1
        return
    
    # checks of point is within range
    # returns True or False depending on whether close point found
    def check_range(self, p1):
        # Check for every point, break once a close one is found
        found = False
        for p2 in self.points:
            # Check colors match
            if p2[2] == 2 or p1[2] == 2 or p2[2] == p1[2]:
                # Check colors dont match
                if (p2[0] - p1[0])**2 + (p2[1] - p1[1])**2 < CLUSTER_MAX_RANGE:
                    # Point is close
                    self.add_point(p1)
                    found = True
                    break
        return found
    
    # finalizes cluster and dtermines center and type
    # returns true if cluster is likely a piece
    def finalize(self):
        total = self.red + self.blue + self.yellow
        if total < 5:
            # Not enough points for piece to exist
            return False
        # Check color
        if self.red > self.blue:
            self.is_blue = False
            if self.red < 10:
                # Not enough colored points
                return False
        else:
            if self.blue < 10:
                # Not enough colored points
                return False
            self.is_blue = True
        if self.yellow > 10:
            self.is_king = True
        else:
            self.is_king = False
        # Calc center
        self.x = int(self.x_sum / total)
        self.y = int(self.y_sum / total)
        self.is_valid = True
        return True

# Returns list of points of color in image
def get_points_in_img(img, scale=1):
    # Split and filter
    blue_img = img[:,:,0].astype(np.int16)
    green_img = img[:,:,1].astype(np.int16)
    red_img = img[:,:,2].astype(np.int16)
    blue_filter = blue_img - green_img - red_img
    print(blue_filter)
    red_filter = red_img - blue_img - green_img
    yellow_filter = 0.5*(red_img + green_img) - 2*blue_img
    # Find points
    kernel_size = 4//scale
    kernel_area = kernel_size ** 2
    filter_cutoff = 160 * kernel_area
    blue_points = []
    red_points = []
    for i in range(0, 1088, kernel_size):
        for j in range(0, 1920, kernel_size):
            # Check for blue
            if blue_filter[i:i+kernel_size,j:j+kernel_size].sum() > filter_cutoff:
                blue_points.append((i+kernel_size//2, j+kernel_size//2, 0))
            elif red_filter[i:i+kernel_size,j:j+kernel_size].sum() > filter_cutoff:
                red_points.append((i+kernel_size//2, j+kernel_size//2, 1))
            elif yellow_filter[i:i+kernel_size,j:j+kernel_size].sum() > filter_cutoff:
                blue_points.append((i+kernel_size//2, j+kernel_size//2, 2))
                red_points.append((i+kernel_size//2, j+kernel_size//2, 2))
    return blue_points, red_points

# turns list of points into list of clusters
def clusterize(p_list, is_blue):
    # create clusters as linked list
    clusters = []
    for point in p_list:
        # Go through all clusters and stop after finding one
        found = False
        for c in clusters:
            if c.check_range(point):
                c.add_point(point)
                found = True
                break
        if not found:
            # Create new cluster
            new_cluster = Cluster()
            new_cluster.add_point(point)
            clusters.append(new_cluster)
    # Finish cluster calculation
    final_clusters = []
    for c in clusters:
        c.finalize()
        if c.is_valid and c.is_blue == is_blue:
            # Probably a piece
            final_clusters.append(c)
    return final_clusters

# Test methods if run directly
if __name__ == "__main__":
    import sys
    start = time.perf_counter()
    # Check arguments
    if len(sys.argv) != 2:
        print("piece_recognition.py expects exactly one argument: the file name or 'pic'")
        exit()
    filename = sys.argv[1]
    img = None
    if filename == "pic":
        img = None
    else:
        img = cv2.imread(filename)
    img = img.astype(np.uint16)
    # img = cv2.resize(img, (0,0), fx=0.5, fy=0.5)
    # Check to make sure image is valid
    if img is None:
        print(f"File could not be read: {filename}")
        exit()
    # Start analyzing image
    blue_points, red_points = get_points_in_img(img, scale=1)
    points_end = time.perf_counter()
    print(f"# of blue: {len(blue_points)}, # of red: {len(red_points)}")
    blue_clusters = clusterize(blue_points, True)
    red_clusters = clusterize(red_points, False)
    print(f"There are {len(blue_clusters)} blue clusters and {len(red_clusters)} red clusters")
    end = time.perf_counter()
    """
    print("blue:")
    for c in blue_clusters:
        print(c.x, c.y)
    print("red:")
    for c in red_clusters:
        print(c.x, c.y)
    """
    print(f"Took {points_end - start:.3f} seconds for points")
    print(f"Took {end - points_end:.3f} seconds to clusterize")
    print(f"Took {end - start:.3f} seconds total")
    