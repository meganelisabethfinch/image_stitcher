#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgcodecs.hpp>

using namespace cv;

int main(int argc, char** argv )
{
    if ( argc != 3 )
    {
        printf("usage: ./image-stitching <Img1Path> <Img2Path> \n");
        return -1;
    }
        
    std::vector<cv::Mat> images;

    // IMPORTANT: Flipped the order of images, the array goes right to left
    for (int i = 2; i > 0; i--) {
        cv::Mat img = imread(argv[i], 0);
        if (!img.data) {
            printf("No image data.\n");
            return -1;
        }
        images.push_back(img);
    }
    printf("Read images.\n");

    // Set-up SIFT
    cv::Ptr<cv::SIFT> detector = cv::SIFT::create();

    std::vector<std::vector<cv::KeyPoint>> allKeypoints;
    std::vector<cv::Mat> allDescriptors;

    for (size_t i = 0; i < images.size(); i++) {
        std::vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;

        detector->detect(images[i], keypoints);
        printf("Found keypoints.\n");
        detector->compute(images[i], keypoints, descriptors);
        printf("Computed descriptors.\n");

        allKeypoints.push_back(keypoints);
        allDescriptors.push_back(descriptors);
    }

    // Find matches
    cv::Ptr<cv::BFMatcher> matcher = cv::BFMatcher::create();
    std::vector<std::vector<cv::DMatch>> rawMatches;
    matcher->knnMatch(allDescriptors[0], allDescriptors[1], rawMatches, 2);
    printf("Found raw matches.\n");

    std::vector<cv::DMatch> goodMatches;

    for (int i = 0; i < rawMatches.size(); i++) {
        std::vector<cv::DMatch> m = rawMatches[i];
        if (m.size() == 2 && (m[0].distance < m[1].distance * 0.6)) {
            // Lowe's Ratio Test: If d1/d2 < 0.6, this is a good match
            goodMatches.push_back(m[0]);
        }
    }

    printf("Found good matches.\n");

    // Collect keypoints coords for the good matches
    std::vector<cv::Point2f> pts1;
    std::vector<cv::Point2f> pts2; 

    for (int i = 0; i < goodMatches.size(); i++) {
        int idx = goodMatches[i].queryIdx;
        pts1.push_back(allKeypoints[0][idx].pt);
    }

    for (int i = 0; i < goodMatches.size(); i++) {
        int idx = goodMatches[i].trainIdx;
        pts2.push_back(allKeypoints[1][idx].pt);
    }

    // Check we have enough matches for homography
    if (goodMatches.size() < 4) {
       printf("Not enough matches. 4+ matches are needed to compute a homography.\n");
       return -1;
    }

    // Find homography
    cv::Mat H;
    H = cv::findHomography(pts1, pts2, cv::RANSAC);

    // Warp perspective on the right image
    int nrows = images[0].rows;
    int ncols = images[0].cols + images[1].cols;
    cv::Mat result = cv::Mat(nrows, ncols, 0);
    cv::warpPerspective(images[0], result, H, cv::Size(ncols,nrows));
    
    // Add in the left image
    for (int i = 0; i < images[1].rows; i++) {
        for (int j = 0; j < images[1].cols; j++) {
            result.at<uchar>(i,j) = images[1].at<uchar>(i,j);
        }
    }
    

    namedWindow("Panorama", WINDOW_AUTOSIZE);
    imshow("Panorama", result);
    waitKey(0);

    imwrite("./images/result.png", result);

    /*
    // Collect keypoints for the good matches
    std::vector<cv::KeyPoint> kps0;
    std::vector<cv::KeyPoint> kps1; 

    for (int i = 0; i < goodMatches.size(); i++) {
        int idx = goodMatches[i].queryIdx;
        kps0.push_back(allKeypoints[0][idx]);
    }

    for (int i = 0; i < goodMatches.size(); i++) {
        int idx = goodMatches[i].trainIdx;
        kps1.push_back(allKeypoints[1][idx]);
    }

    cv::Mat matchesVis;
    cv::drawMatches(images[1], kps1, images[0], kps0, goodMatches, matchesVis);
    
    namedWindow("Display Matches", WINDOW_AUTOSIZE );
    imshow("Display Matches", matchesVis);
    waitKey(0); // Any keypress to exit
    */

    return 0;
}