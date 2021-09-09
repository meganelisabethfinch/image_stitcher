#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/gapi/own/types.hpp>

using namespace cv;

int main(int argc, char** argv )
{
    if ( argc != 3 )
    {
        printf("usage: ./image-stitching <Img1_Path> <Img2_Path> \n");
        return -1;
    }
    cv::Mat img1;
    img1 = imread(argv[1], 0);
    if ( !img1.data )
    {
        printf("No image data for image 1 \n");
        return -1;
    }

    cv::Mat img2;
    img2 = imread(argv[2], 0);
    if ( !img2.data )
    {
        printf("No image data for image 2 \n");
        return -1;
    }
    std::vector<cv::Mat> images;
    images.push_back(img1);
    images.push_back(img2);
   
    // Set-up SIFT
    cv::Ptr<cv::SIFT> detector = cv::SIFT::create();

    std::vector<std::vector<cv::KeyPoint>> allKeypoints;
    std::vector<cv::Mat> allDescriptors;

    for (size_t i = 0; i < images.size(); i++) {
        std::vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;

        detector->detect(images[i], keypoints);
        detector->compute(images[i], keypoints, descriptors);

        allKeypoints.push_back(keypoints);
        allDescriptors.push_back(descriptors);
    }

    // Find matches
    cv::Ptr<cv::BFMatcher> matcher = cv::BFMatcher::create();
    std::vector<std::vector<cv::DMatch>> rawMatches;
    matcher->knnMatch(allDescriptors[0], allDescriptors[1], rawMatches, 2);

   std::vector<cv::DMatch> goodMatches;

   // Collect keypoints and coords for good matches
   std::vector<KeyPoint> kps1;
   std::vector<KeyPoint> kps2;
   std::vector<Point2f> pts1;
   std::vector<Point2f> pts2; 

   for (int i = 0; i < rawMatches.size(); i++) {
       std::vector<cv::DMatch> m = rawMatches[i];
       if (m.size() == 2 && (m[0].distance < m[1].distance * 0.6)) {
           // Lowe's Ratio Test: If d1/d2 < 0.6, this is a good match
           goodMatches.push_back(m[0]);
           kps1.push_back(allKeypoints[0][m[0].queryIdx]);
           kps2.push_back(allKeypoints[1][m[0].trainIdx]);
           pts1.push_back(allKeypoints[0][m[0].queryIdx].pt);
           pts2.push_back(allKeypoints[1][m[0].trainIdx].pt);
       }
   }

   // check we have enough matches for homography
   if (goodMatches.size() < 4) {
       printf("Not enough matches. 4+ matches are needed to compute a homography.\n");
       return -1;
   }

   // find homography

   cv::Mat H;
   H = cv::findHomography(pts1, pts2, cv::RANSAC);

   // warp perspective to stitch the images together
   cv::Mat result = cv::Mat(img1.rows, img1.cols + img2.cols, 0);
   cv::warpPerspective(img1, result, H, cv::Size(img1.cols + img2.cols, img1.rows));

    namedWindow("Panorama", WINDOW_AUTOSIZE);
    imshow("Panorama", result);
    waitKey(0);
   /*
    // Draw good matches
    cv::Mat output;
    cv::drawMatches(images[0], allKeypoints[0], images[1], allKeypoints[1], goodMatches, output, Scalar::all(-1), Scalar::all(-1), std::vector< char >(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

    namedWindow("Display Matches", WINDOW_AUTOSIZE );
    imshow("Display Matches", output);
    waitKey(0); // Any keypress to exit
    */

    return 0;
}