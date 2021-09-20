#include "stitcher.hpp"

#include <stdio.h>

#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>

Stitcher::Stitcher(double ratio) : ratio(ratio) {}

int Stitcher::stitch(cv::Mat leftImg, cv::Mat rightImg, cv::Mat &out)
{
    // Take greyscale copies
    cv::Mat leftGrey, rightGrey;
    cv::cvtColor(leftImg, leftGrey, cv::COLOR_BGR2GRAY);
    cv::cvtColor(rightImg, rightGrey, cv::COLOR_BGR2GRAY);

    // Set-up SIFT
    cv::Ptr<cv::SIFT> detector = cv::SIFT::create();

    std::vector<std::vector<cv::KeyPoint>> allKeypoints;
    std::vector<cv::Mat> allDescriptors;

    std::vector<cv::KeyPoint> leftKP;
    std::vector<cv::KeyPoint> rightKP;
    detector->detect(leftGrey, leftKP);
    detector->detect(rightGrey, rightKP);
    printf("Found keypoints.\n");

    cv::Mat leftDescriptors;
    cv::Mat rightDescriptors;
    detector->compute(leftGrey, leftKP, leftDescriptors);
    detector->compute(rightGrey, rightKP, rightDescriptors);
    printf("Computed descriptors.\n");

    // Find matches
    cv::Ptr<cv::BFMatcher> matcher = cv::BFMatcher::create();
    std::vector<std::vector<cv::DMatch>> rawMatches;
    matcher->knnMatch(rightDescriptors, leftDescriptors, rawMatches, 2);
    printf("Found raw matches.\n");

    std::vector<cv::DMatch> goodMatches;

    for (int i = 0; i < rawMatches.size(); i++)
    {
        std::vector<cv::DMatch> m = rawMatches[i];
        if (m.size() == 2 && (m[0].distance < m[1].distance * 0.6))
        {
            // Lowe's Ratio Test: If d1/d2 < 0.6, this is a good match
            goodMatches.push_back(m[0]);
        }
    }

    printf("Found good matches.\n");

    // Collect keypoints coords for the good matches
    std::vector<cv::Point2f> rightPts;
    std::vector<cv::Point2f> leftPts;

    for (int i = 0; i < goodMatches.size(); i++)
    {
        int idx = goodMatches[i].queryIdx;
        rightPts.push_back(rightKP[idx].pt);
    }

    for (int i = 0; i < goodMatches.size(); i++)
    {
        int idx = goodMatches[i].trainIdx;
        leftPts.push_back(leftKP[idx].pt);
    }

    // Check we have enough matches for homography
    if (goodMatches.size() < 4)
    {
        printf("Not enough matches. 4+ matches are needed to compute a homography.\n");
        return -1;
    }

    // Find homography
    cv::Mat H;
    H = cv::findHomography(rightPts, leftPts, cv::RANSAC);

    // Warp perspective on the right image
    int nrows = rightGrey.rows;
    int ncols = rightGrey.cols + leftGrey.cols;
    cv::Mat result = cv::Mat(nrows, ncols, rightGrey.type());
    cv::warpPerspective(rightGrey, result, H, cv::Size(ncols, nrows));

    // Add in the left image
    for (int i = 0; i < leftGrey.rows; i++)
    {
        for (int j = 0; j < leftGrey.cols; j++)
        {
            result.at<uchar>(i, j) = leftGrey.at<uchar>(i, j);
        }
    }

    out = cv::Mat(result);
    return 0;
}

