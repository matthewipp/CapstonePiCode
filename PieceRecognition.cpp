/**
 * @file PieceRecognition.cpp
 * @author EMNEM
 * @brief 
 * @version 0.1
 * @date 2022-10-02
 * 
 * C++ implementation of piece_recognition.py
 */

#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "PieceRecognition.h"

Cluster::Cluster() {
    red = 0;
    blue = 0;
    yellow = 0;
    isBlue = false;
    isKing = false;
    isValid = false;
    xSum = 0;
    ySum = 0;
    x = 0;
    y = 0;
}

void Cluster::addPoint(Point& p) {
    // Add point to list and update sums
    points.push_back(p);
    xSum += p.x;
    ySum += p.y;
    // Check color and add to appropriate counter
    if(p.type == RED) {
        red += 1;
    }
    else if(p.type == BLUE) {
        blue += 1;
    }
    else {
        yellow += 1;
    }
}

bool Cluster::checkRange(Point& p1) {
    bool found = false;
    // Check for close points until one is found
    // BREAKS WHEN POINT IS FOUND
    for(Point& p2 : points) {
        // Check if points are same color or one is yellow
        if(p2.type == YELLOW || p1.type == YELLOW || p2.type == p1.type) {
            // Check distance
            int xDiff = p2.x - p1.x;
            int yDiff = p2.y - p1.y;
            if(xDiff*xDiff + yDiff*yDiff < CLUSTER_MAX_DISTANCE_SQUARE) {
                // Point is within range
                addPoint(p1);
                found = true;
                break;
            } 
        }
    }
    // Return whether close point was found
    return found;
}

bool Cluster::finalize() {
    // Check total number of points
    int totalPoints = red + blue + yellow;
    if(totalPoints < CLUSTER_MIN_POINTS) {
        // Not enough points to be piece
        return false;
    }
    // Check color
    if(red > blue) {
        isBlue = false;
        if(red < CLUSTER_MIN_POINTS) {
            // Not enough red points to be piece
            return false;
        }
    }
    else {
        isBlue = true;
        if(blue < CLUSTER_MIN_POINTS) {
            // Not enough blue points to be piece
            return false;
        }
    }
    // Check for king
    if(yellow >= CLUSTER_KING_MIN_POINTS) {
        isKing = true;
    }
    else {
        isKing = false;
    }
    // Calculate center
    x = xSum / totalPoints;
    y = ySum / totalPoints;
    isValid = true;
    return true;
}

void getPointsInImage(cv::Mat& img, std::vector<std::vector<Point>>& pointsList) {
    // Split RGB channels
    cv::Mat bgr[3];
    cv::split(img, bgr);
    cv::Mat blueChannel, greenChannel, redChannel;
    bgr[0].convertTo(blueChannel, CV_32SC1);
    bgr[1].convertTo(greenChannel, CV_32SC1);
    bgr[2].convertTo(redChannel, CV_32SC1);
    // Filter
    cv::Mat blueFilter = blueChannel - redChannel - greenChannel;
    cv::Mat redFilter = redChannel - blueChannel - greenChannel;
    cv::Mat yellowFilter = 0.5 * (redChannel + greenChannel) - 2*blueChannel;
    // Find points
    int kernelSize = KERNEL_SIZE;
    int kernelArea = kernelSize * kernelSize;
    double filterCutoff = 160 * kernelArea;
    std::vector<Point> bluePoints;
    std::vector<Point> redPoints;
    for(int i = 0; i < 1088; i += kernelSize) {
        for(int j = 0; j < 1920; j += kernelSize) {
            cv::Mat blueSquare = blueFilter(cv::Rect(j, i, kernelSize, kernelSize));
            cv::Mat redSquare = redFilter(cv::Rect(j, i, kernelSize, kernelSize));
            cv::Mat yellowSquare = yellowFilter(cv::Rect(j, i, kernelSize, kernelSize));
            // Check for blue
            if(cv::sum(blueSquare)[0] > filterCutoff) {
                Point p;
                p.x = i + kernelSize/2;
                p.y = j + kernelSize/2;
                p.type = BLUE;
                bluePoints.push_back(p);
            }
            // Check for red
            else if(cv::sum(redSquare)[0] > filterCutoff) {
                Point p;
                p.x = i + kernelSize/2;
                p.y = j + kernelSize/2;
                p.type = RED;
                redPoints.push_back(p);
            }
            // Check for yellow
            else if(cv::sum(yellowSquare)[0] > filterCutoff) {
                Point p;
                p.x = i + kernelSize/2;
                p.y = j + kernelSize/2;
                p.type = YELLOW;
                bluePoints.push_back(p);
                redPoints.push_back(p);
            }
        }
    }
    // Add to vector and return
    pointsList.push_back(redPoints);
    pointsList.push_back(bluePoints);
    return;
}

void clusterize(std::vector<Point>& pList, bool isBlue, std::vector<Cluster>& finalClusters) {
    std::vector<Cluster> clusters;
    for(Point& p : pList) {
        // Go through all clusters and stop after finding one
        bool found = false;
        for(Cluster& c : clusters) {
            if(c.checkRange(p)) {
                c.addPoint(p);
                found = true;
                break;
            }
        }
        if(!found) {
            Cluster newCluster;
            newCluster.addPoint(p);
            clusters.push_back(newCluster);
        }
    }
    // Finish cluster calculation
    for(Cluster& c : clusters) {
        c.finalize();
        if(c.isValid && c.isBlue == isBlue) {
            finalClusters.push_back(c);
        }
    }
    return;
}
