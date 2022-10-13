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

int testClusterizing(int argc, char** argv) {
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

int testBoardAligner(int argc, char** argv) {
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
        return 0;
    }
    // Align board
    ImageState boardState;
    boardState.alignCamera(img);
    return 0;
}

int testBoardString(int argc, char** argv) {
    // Check arguments
    if(argc != 3) {
        std::cout << "Must use exactly 2 arguments, the align file path and the image file path\n";
        return 0;
    }
    // Read images
    std::string alignFilename = argv[1];
    std::string stateFilename = argv[2];
    cv::Mat alignImg = cv::imread(alignFilename);
    if(alignImg.empty()) {
        std::cout << "Could not read file: " << alignFilename << "\n"; 
        return 0;
    }
    cv::Mat stateImg = cv::imread(stateFilename);
    if(stateImg.empty()) {
        std::cout << "Could not read file: " << stateFilename << "\n"; 
        return 0;
    }
    // Align board
    ImageState boardState;
    boardState.alignCamera(alignImg);
    // Get board state
    bool yay = boardState.generateBoardstate(stateImg);
    if(yay) {
        std::cout << "Successfully imaged board: \n";
        std::cout << boardState.boardState << "\n";
    }
    else {
        std::cout << "Detected inavlid board\n";
    }
    std::cout << "Board left and right side: " << boardState.edgeX[0] << ", " << boardState.edgeX[1] << "\n";
    std::cout << "Board top and bottom side: " << boardState.edgeY[0] << ", " << boardState.edgeY[1] << "\n";
    std::cout << "Average Square Size (widthxheight): " << boardState.avgSquareWidth << " x ";
    std::cout << boardState.avgSquareHeight << "\n";
    std::cout << "Red Pieces: " << boardState.redPiecesOnBoard.size() << "\n";
    std::cout << "Blue Pieces: " << boardState.bluePiecesOnBoard.size() << "\n";
    return 0;
}

int main(int argc, char** argv) {
    //return testBoardAligner(argc, argv);
    //return testClusterizing(argc, argv);
    return testBoardString(argc, argv);
}
