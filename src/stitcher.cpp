#include "stitcher.hpp"

#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>

cv::Mat Stitcher::stitch(std::vector<cv::Mat> images)
{
    if (images.size() == 0)
    {
        // TODO: throw error
    }

    while (images.size() > 1)
    {
        // Take greyscale copies of images[0..1]
        std::vector<cv::Mat> greyImages;

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
            // TODO: throw error
        }

        // Find homography
        cv::Mat H;
        H = cv::findHomography(pts1, pts2, cv::RANSAC);

        // Warp perspective on the right image
        int nrows = images[0].rows;
        int ncols = images[0].cols + images[1].cols;
        cv::Mat result = cv::Mat(nrows, ncols, 0);
        cv::warpPerspective(images[0], result, H, cv::Size(ncols, nrows));

        // Add in the left image
        for (int i = 0; i < images[1].rows; i++)
        {
            for (int j = 0; j < images[1].cols; j++)
            {
                result.at<uchar>(i, j) = images[1].at<uchar>(i, j);
            }
        }

        // Pop images[0..1]

        // Push result to images[0]
    }

    return images.front();
}
