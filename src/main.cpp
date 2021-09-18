#include <unistd.h>
#include <stdio.h>
#include <string>

#include "./headers/stitcher.hpp"

#include <opencv2/imgcodecs.hpp>

int main(int argc, char** argv) {
    // Parse inputs
    int opt;
    std::vector<cv::Mat> inputImages;
    std::string outputImagePath = "";

    while ((opt = getopt(argc, argv, "i:o:")) != -1) {
        switch (opt) {
            case 'i': {
                cv::Mat image = cv::imread(optarg, 1);
                if (!image.data) {
                    printf("No image data for %s.\n", optarg);
                    return -1;
                }
                inputImages.push_back(image);
                break;
            }
            case 'o': {
                outputImagePath = optarg;
                break;
            }
            default: {
                break;
            }
        }
    }

    // Check mandatory options
    if (inputImages.size() < 2 || outputImagePath == "") {
        printf("Usage: <inputImagePaths> <outpathImagePath>");
        return -1;
    }

    // Create stitcher
    Stitcher stitcher = Stitcher(inputImages);

    // call stitcher

    // save image to output file
}