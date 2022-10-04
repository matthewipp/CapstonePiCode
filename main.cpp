/**
 * @file main.cpp
 * @author EMNEM
 * @brief 
 * @version 0.1
 * @date 2022-10-02
 * 
 * Tests Piece Recognition files
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "PieceRecognition.h"

int main(int argc, char** argv) {
    // Check arguments
    if(argc != 2) {
        std::cout << "Must use exactly 1 argument, the file path\n";
        return 0;
    }
    // Read image
    std::string filename = argv[1];
    cv::Mat img = cv::imread(filename);
    if(img.empty()) {
        std::cout << "Could not read file: " << filename << "\n"; 
    }
    std::cout << "Successfully Read file: " << filename << "\n";
    // Get points lists
    int test = CLUSTER_MIN_POINTS;
    std::vector<std::vector<Point>> points;
    getPointsInImage(img, points);
    std::vector<Point> bluePoints = points.back();
    points.pop_back();
    std::vector<Point> redPoints = points.back();
    points.pop_back();
    // Write points to file
    /*
    std::ofstream fStream;
    fStream.open("Coutput.txt");
    for(Point p : bluePoints) {
        fStream << "(" << p.x << ", " << p.y << ") " << p.type << "\n";
    }
    */
    // Print points
    std::cout << "There are " << bluePoints.size() << " blue points and ";
    std::cout << redPoints.size() << " red points\n";
    // Clusterize
    std::vector<Cluster> blueClusters;
    clusterize(bluePoints, true, blueClusters);
    std::vector<Cluster> redClusters;
    clusterize(redPoints, false, redClusters);
    // Print clusters
    std::cout << "There are " << blueClusters.size() << " blue clusters and ";
    std::cout << redClusters.size() << " red clusters\n";

    return 0;
}
