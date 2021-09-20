#include "stitcher.hpp"

#include <stdio.h>

#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>

Stitcher::Stitcher(double ratio) : ratio(ratio) {}

int Stitcher::stitchAdjacent(cv::Mat leftImg, cv::Mat rightImg, cv::Mat &out)
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

    cv::Mat leftDescriptors;
    cv::Mat rightDescriptors;
    detector->compute(leftGrey, leftKP, leftDescriptors);
    detector->compute(rightGrey, rightKP, rightDescriptors);

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
    cv::Mat result = cv::Mat(nrows, ncols, 0);
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

int Stitcher::stitch(std::vector<cv::Mat> images, cv::Mat &out)
{
    if (images.size() == 0)
    {
        printf("Input image array is empty.");
        return -1;
    }

    while (images.size() > 1)
    {
        // Take greyscale copies of images[0..1]
        /*
        std::vector<cv::Mat> greyImages;
        cv::Mat grey1;
        cv::cvtColor(images[0], grey1, cv::COLOR_BGR2GRAY);
        cv::Mat grey2;
        cv::cvtColor(images[1], grey2, cv::COLOR_BGR2GRAY);

        greyImages.push_back(grey1);
        greyImages.push_back(grey2);

        // Set-up SIFT
        cv::Ptr<cv::SIFT> detector = cv::SIFT::create();

        std::vector<std::vector<cv::KeyPoint>> allKeypoints;
        std::vector<cv::Mat> allDescriptors;

        for (size_t i = 0; i < greyImages.size(); i++)
        {
            std::vector<cv::KeyPoint> keypoints;
            cv::Mat descriptors;

            detector->detect(greyImages[i], keypoints);
            printf("Found keypoints.\n");
            detector->compute(greyImages[i], keypoints, descriptors);
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
        std::vector<cv::Point2f> pts1;
        std::vector<cv::Point2f> pts2;

        for (int i = 0; i < goodMatches.size(); i++)
        {
            int idx = goodMatches[i].queryIdx;
            pts1.push_back(allKeypoints[0][idx].pt);
        }

        for (int i = 0; i < goodMatches.size(); i++)
        {
            int idx = goodMatches[i].trainIdx;
            pts2.push_back(allKeypoints[1][idx].pt);
        }

        // Check we have enough matches for homography
        if (goodMatches.size() < 4)
        {
            printf("Not enough matches. 4+ matches are needed to compute a homography.\n");
            return -1;
        }

        // Find homography
        cv::Mat H;
        H = cv::findHomography(pts1, pts2, cv::RANSAC);

        // Warp perspective on the right image
        int nrows = greyImages[0].rows;
        int ncols = greyImages[0].cols + greyImages[1].cols;
        cv::Mat result = cv::Mat(nrows, ncols, 0);
        cv::warpPerspective(greyImages[0], result, H, cv::Size(ncols, nrows));

        // Add in the left image
        for (int i = 0; i < greyImages[1].rows; i++)
        {
            for (int j = 0; j < greyImages[1].cols; j++)
            {
                result.at<uchar>(i, j) = greyImages[1].at<uchar>(i, j);
            }
        }
        */

       cv::Mat result;
       stitchAdjacent(images[0], images[1], result);

        // TODO: replace vector with queue
        // Pop images[0..1] from front
        images.erase(images.begin());
        images.erase(images.begin());

        // Push result to images[0]
        images.insert(images.begin(), result);
    }

    out = cv::Mat(images.front());
    return 0;
}
