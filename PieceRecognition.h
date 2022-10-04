/**
 * @file PieceRecognition.h
 * @author EMNEM
 * @brief 
 * @version 0.1
 * @date 2022-10-02
 * 
 * Header file for c++ checkers piece recognition
 */

#ifndef PIECE_RECOGNITION_H
#define PIECE_RECOGNITION_H

#include <opencv2/opencv.hpp>
#include <vector>

// Max distance in pixels squared
#define CLUSTER_MAX_DISTANCE_SQUARE 400
#define CLUSTER_MIN_POINTS 10
#define CLUSTER_KING_MIN_POINTS 10
#define KERNEL_SIZE 8

enum PointType {RED, BLUE, YELLOW};

struct Point {
    int x;
    int y;
    enum PointType type;
};

class Cluster {
    public:
        // Initialize object
        Cluster();
        // Contains info about group of pixels and how to add new ones
        // point is struct of int x, int y, int type), where 0 = blue, 1 = red, 2 = yellow
        void addPoint(Point& p);
        // Checks if point is within range
        // returns True or False depending on whether close point found
        bool checkRange(Point& p1);
        // Finalizes cluster and returns true or false depending
        // on whether cluster is likely a piece
        bool finalize();
        // Variables
        int red;
        int blue;
        int yellow;
        bool isBlue;
        bool isKing;
        bool isValid;
        std::vector<Point> points;
        int xSum;
        int ySum;
        int x;
        int y;
};

// Returns list of points for both red and blue pieces
void getPointsInImage(cv::Mat& img, std::vector<std::vector<Point>>& pointsList);
// Turns list of Points into list of clusters
void clusterize(std::vector<Point>& pList, bool isBlue, std::vector<Cluster>& finalClusters);

#endif
