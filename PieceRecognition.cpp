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

//////////////////////////////// Cluster Definitions /////////////////////////////////////
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

//////////////////////////////// ImageState Definitions /////////////////////////////////////
int ImageState::countRedKingsOnBoard() {
    int counter = 0;
    for(CheckersPiece& piece : redPiecesOnBoard) {
        if(piece.isKing) {
            counter++;
        }
    }
    return counter;
}

int ImageState::countBlueKingsOnBoard() {
    int counter = 0;
    for(CheckersPiece& piece : bluePiecesOnBoard) {
        if(piece.isKing) {
            counter++;
        }
    }
    return counter;
}

int ImageState::countRedKingsOffBoard() {
    int counter = 0;
    for(CheckersPiece& piece : redPiecesOffBoard) {
        if(piece.isKing) {
            counter++;
        }
    }
    return counter;
}

int ImageState::countBlueKingsOffBoard() {
    int counter = 0;
    for(CheckersPiece& piece : bluePiecesOffBoard) {
        if(piece.isKing) {
            counter++;
        }
    }
    return counter;
}

bool ImageState::generateBoardstate(cv::Mat& img) {
    // Get points
    std::vector<std::vector<Point>> points;
    getPointsInImage(img, points);
    std::vector<Point> bluePoints = points.back();
    points.pop_back();
    std::vector<Point> redPoints = points.back();
    points.pop_back();
    // Clusterize
    std::vector<Cluster> blueClusters;
    clusterize(bluePoints, true, blueClusters);
    std::vector<Cluster> redClusters;
    clusterize(redPoints, false, redClusters);
    bool valid = generateBoardState(redClusters, blueClusters);
    isValidState = valid;
    // check if the move was legal here
    bool wasLegalMove = true;
    if(valid && wasLegalMove) {
        lastValidBoardState = boardState;
        return true;
    }
    else {
        return false;
    }
}

bool ImageState::generateBoardState(std::vector<Cluster>& redClusters, std::vector<Cluster>& blueClusters) {
    bool success = true;
    // Red Pieces
    for(Cluster& c : redClusters) {
        CheckersPiece cp;
        cp.isBlue = false;
        cp.isKing = c.isKing;
        cp.x = c.x;
        cp.y = c.y;
        cp.onBoard = edgeX[0] < cp.x && cp.x < edgeX[1] && edgeY[0] < cp.y && cp.y < edgeY[1];
        if(cp.onBoard)
            redPiecesOnBoard.push_back(cp);
        else
            redPiecesOffBoard.push_back(cp);
    }
    // Blue Pieces
    for(Cluster& c : blueClusters) {
        CheckersPiece cp;
        cp.isBlue = true;
        cp.isKing = c.isKing;
        cp.x = c.x;
        cp.y = c.y;
        cp.onBoard = edgeX[0] < cp.x && cp.x < edgeX[1] && edgeY[0] < cp.y && cp.y < edgeY[1];
        if(cp.onBoard)
            bluePiecesOnBoard.push_back(cp);
        else
            bluePiecesOffBoard.push_back(cp);
    }
    // Check positions on board
    char pos[8][8];
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            pos[i][j] = '.';
        }
    }
    for(CheckersPiece& p : redPiecesOnBoard) {
        std::cout << p.x << ", " << p.y << "\n";
        cv::Point2i coord = getBoardPos(p);
        if(coord.x == -1) {
            // Invaloid coordinate
            success = false;
        }
        else if(pos[coord.x][coord.y] == '.') {
            if(p.isKing) {
                pos[coord.x][coord.y] = 'R';
            } 
            else {
                pos[coord.x][coord.y] = 'r';
            }
        } 
        else {
            // Two pieces on same spot
            success = false;
        }
    }
    for(CheckersPiece& p : bluePiecesOnBoard) {
        cv::Point2i coord = getBoardPos(p);
        if(coord.x == -1) {
            // Invalid coordinate
            success = false;
        }
        if(pos[coord.x][coord.y] == '.') {
            if(p.isKing) {
                pos[coord.x][coord.y] = 'B';
            } 
            else {
                pos[coord.x][coord.y] = 'b';
            }
        } 
        else {
            // Two pieces on same spot
            success = false;
        }
    }
    // update boardsate string
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            boardState = boardState + pos[i][j];
        }
        boardState = boardState + "\n";
    }
    return success;
}

bool ImageState::alignCamera(cv::Mat& img) {
    int cornerWidth = 7;
    int cornerHeight = 7;
    cv::Size boardSize(cornerWidth, cornerHeight);
    std::vector<cv::Point2f> corners;
    bool found = cv::findChessboardCorners(img, boardSize, corners, cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_FAST_CHECK);
    if(!found) {
        // Could not find checkers board
        std::cout << "Did not find grid\n";
        return false;
    }
    /*
    // Undistort
    std::vector<cv::Point3f> objPoints;
    std::vector<std::vector<cv::Point3f>> objPointsOuter;
    std::vector<std::vector<cv::Point2f>> cornersOuter;
    for(int i = 0; i < 7; i++) {
        for(int j = 0; j < 7; j++) {
            objPoints.push_back(cv::Point3f((float)j*10, (float)i*10, 0.0f));
        }
    }
    objPointsOuter.push_back(objPoints);
    cornersOuter.push_back(corners);
    cv::Mat k, d;
    std::vector<cv::Mat> rvecs, tvecs;
    //cv::calibrateCamera(objPointsOuter, cornersOuter, img.size(), k, d, rvecs, tvecs);
    cv::Mat newCameraMat = cv::getOptimalNewCameraMatrix(k, d, img.size(), 1, img.size());
    cv::Mat newImage;
    cv::undistort(img, newImage, k, d);
    */
    // Find grid size
    // Calculate average board width and height:
    float widthSum = 0;
    float heightSum = 0;
    float topSum = 0;
    float botSum = 0;
    float leftSum = 0;
    float rightSum = 0;
    for(int i = 0; i < boardSize.area(); i++) {
        // Check if corner is not on the right end
        if(i % 7 != 6) {
            widthSum += corners[i+1].x - corners[i].x;
        }
        else {
            rightSum += corners[i].x;
        }
        // Check if corner is on the left
        if(i % cornerWidth == 0) {
            leftSum += corners[i].x;
        }
        // Check if corner is not on the bottom
        if(i < boardSize.area() - cornerWidth) {
            heightSum += corners[i+cornerWidth].y - corners[i].y;     
        }
        else {
            botSum += corners[i].y;
        }
        // Check if corner is on the top
        if(i < cornerWidth) {
            topSum += corners[i].y;
        }
    }
    // The number of heights and widths is 1 less than the number of corners found per edge
    float avgWidth = widthSum / 42;
    float avgHeight = heightSum / 42;
    avgSquareHeight = (int)avgWidth;
    avgSquareWidth = (int)avgHeight;
    // Extrapolate out to board edges, x and y from coordinates to board are switched
    edgeY[0] = (int)(leftSum / cornerHeight - avgWidth);
    edgeY[1] = (int)(rightSum / cornerHeight + avgWidth);
    edgeX[0] = (int)(topSum / cornerWidth - avgHeight);
    edgeX[1] = (int)(botSum / cornerWidth + avgHeight);
    return true;
}

cv::Point2i ImageState::getBoardPos(CheckersPiece& p) {
    // Keep moving right or down until you get past the point
    cv::Point2i pos(0, 0);
    while(p.x > edgeX[0] + (pos.x+1)*avgSquareWidth) {
        pos.x++;
    }
    while(p.y > edgeY[0] + (pos.y+1)*avgSquareHeight) {
        pos.y++;
    }
    // Check distance, if too far, return -1
    // The calculated value is always larger than p.x or p.y
    // Must be in the middle 3/5 of the board
    int minWidth = avgSquareWidth / 5;
    int xDist = edgeX[0] + (pos.x+1)*avgSquareWidth - p.x;
    int yDist = edgeY[0] + (pos.y+1)*avgSquareHeight - p.y;
    if(xDist < minWidth || xDist > minWidth * 4) {
        // Piece is close to a border
        pos.x = -1;
        pos.y = -1;
    }
    return pos;
}

//////////////////////////////// Global Method Definitions //////////////////////////////////
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
