/*
 * Implementation of the Pytorch C++ native image classifier using a model like MobileNet-V3
 */

// see https://github.com/pytorch/vision/issues/2691
// On how to use CUDA with libtorch
// https://github.com/pytorch/pytorch/issues/47590
#include "strawberry.h"
#include <sys/time.h>
#include <torch/script.h> // One-stop header.
#include <torch/cuda.h> 

#include <stb/stb.h>
#include <stb/stb_image.h>

#include <iostream>
#include <memory>
#include <string>
#include <string.h>

#include <jpeglib.h>
#include <jerror.h>
// Note that in order ot use jpeg buffer plugin for CImg a macro for cimg_plugin needs to be defined.
#define cimg_plugin "plugins/jpeg_buffer.h" 
#include <CImg.h>
#include <cstdio>

// Flag to enable image labeling
static bool patchImageWithText = true;

/**
 * A function to convert RGB image containing data from 0..255 for each Red Green Blue colors
 * to a torch compatible tensor
 */
static torch::Tensor convertRGBintoTensor(uint8_t *rgb, int width, int height){
    // I wonder if the rgb data should be translated to +-2? or specifically to the values that are referred
    // in python as: 
    //  mean=[0.485, 0.456, 0.406],
    //  std=[0.229, 0.224, 0.225] ????
    float* rgbf = new float[height*width*3];
    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            rgbf[y*width*3+x*3+0] = (rgb[y*width*3+x*3+0] / 256.0 - 0.5) * 4.0;
            rgbf[y*width*3+x*3+1] = (rgb[y*width*3+x*3+1] / 256.0 - 0.5) * 4.0;
            rgbf[y*width*3+x*3+2] = (rgb[y*width*3+x*3+2] / 256.0 - 0.5) * 4.0;
        }
    }
    torch::Tensor tensor = torch::from_blob(rgbf, {height, width, 3}, torch::kFloat).clone();  // copy
    tensor = tensor.permute({2, 0, 1}).to(torch::kFloat);  // {H,W,C} ===> {C,H,W}
    tensor = tensor.unsqueeze(0);
    delete rgbf;
    return tensor;
}

// TODO: move these to inference_stream structure
static torch::jit::script::Module module;
static std::string currentModelFileName = "";
static std::vector<std::string> labels;


/**
 * Initialize the image labeling code using a specific cuda pre-trained model and related label file.
 */
int init_image_torch(stream_inference_t *stream, const char* modelFileName, const char* labelFileName) {
    if (currentModelFileName == modelFileName) {
        std::cout << "Skipping loading torch model as it is already loaded\n";
        return 0;
    }
    if (torch::cuda::is_available()) {
        std::cout << "Cuda available\n";
    } else {
        std::cout << "Cuda not available\n";
    }
    try {
        // Deserialize the ScriptModule from a file using torch::jit::load().
        module = torch::jit::load(modelFileName);
        module.to(c10::kCUDA);
    } catch (const c10::Error& e) {
        std::cerr << "error loading the model\n";
        return -1;
    }
    
    std::cout << "pytorch model loaded ok\n";

    // Load the labels
    std::ifstream labelFile(labelFileName);
    std::string line;
    while (std::getline(labelFile, line)) {
        labels.push_back(line);
    }
    std::cout << "pytorch labels loaded count=" << labels.size() << "\n";
    currentModelFileName = modelFileName;
    return 0;
}

/**
 * De-initialize the torch model
 */
void deinit_image_torch() {
    if (torch::cuda::is_available()) {
        // module.reset(); // There is no module.reset() API in the Torch C++?
    }
}

/**
 * A routing to get current time in milliseconds for debug timing
 */
static double current_time_millis() {
    static struct timeval start = {0,0};
    if (start.tv_sec == 0) gettimeofday(&start, NULL);
    struct timeval now;
    gettimeofday(&now, NULL);
    return ((now.tv_sec - start.tv_sec + now.tv_usec*1e-6))*1000.0;
}

/**
 * Process the image from the packet. Note that this function updates the packet frame image with inference processed data.
 */
void step(stream_inference_t *stream, packet_t* packet) {
    int width, height, bpp;
    double t0 = current_time_millis();
    // Convert the JPEG to RGB image
    uint8_t* rgb_image = stbi_load_from_memory(packet->frame.data, packet->frame.size_of_data, &width, &height, &bpp, 3);
    if (rgb_image == nullptr) {
        printf("ERROR: Failed to decode JPEG image\n");
        return;
    }
    // Create a vector of inputs.
    std::vector<torch::jit::IValue> inputs;
    double t1 = current_time_millis();

    // Note when using CUDA the tensor needs to be moved to CUDA
    inputs.push_back(convertRGBintoTensor(rgb_image, width, height).to(c10::kCUDA));

    // Execute the model and turn its output into a tensor and back to CPU
    at::Tensor output = module.forward(inputs).toTensor().to(c10::kCPU);
    //std::cout << output.slice(/*dim=*/1, /*start=*/0, /*end=*/1000) << '\n';
    double t2 = current_time_millis();

    const float* p = output.data_ptr<float>();
    float maxf = -100;
    int maxi = -1;
    // if max is at 949 it's a strawberry :)!!!!!
    for (int i=0; i<1000; i++) {
        if (p[i]  > maxf) {
            maxf = p[i];
            maxi = i;
        }
        //printf("%d: %f %f\n", i, p[i], maxf);
    }
    double t3 = current_time_millis();
    //printf("inference detect: %s TIMING=%f msec\n", labels[maxi].c_str(), (t3-t0));

    if (patchImageWithText) {
#if 1
        const float white[] = { 255, 255, 255 }, black[] = { 0, 0, 0 };
        // This does not work since CImg uses RRRR.. GGGG... BBBBB... format instead of RGBRGBRGB
        //cimg_library::CImg<uint8_t> img(rgb_image, width, height, 1, 3); 
        cimg_library::CImg<uint8_t> img;
        img.load_jpeg_buffer(packet->frame.data, packet->frame.size_of_data);
        img.draw_text(16, 16, labels[maxi].c_str(), white, black, 1, 13);
        uint8_t *buffer_output = (JOCTET*)malloc(2*packet->frame.size_of_data);
        unsigned int buf_size = 2*packet->frame.size_of_data;
        img.save_jpeg_buffer(buffer_output, buf_size, 92);
        static int i = 0;
        i++;
        if (i == 30) {
            printf("Input Image Resolution %dx%d size %lu\n",height,width,packet->frame.size_of_data);
            printf("Output Image Resolution %dx%d size %u\n",img.height(),img.width(),buf_size);
            i = 0;
        }
        free(packet->frame.data); // free the old data as we overwrite it with the new one
        packet->frame.data = buffer_output;
        packet->frame.size_of_data = buf_size;
#endif
    } else {
        free(packet->frame.data); // free the old data as we overwrite it with the new one
        // TODO: Add content-type to the packet? So we know if we return text of image or video frame?
        packet->frame.data = (uint8_t *)strdup((char *)labels[maxi].c_str());
        packet->frame.size_of_data = strlen((char *)packet->frame.data);
    }
    stbi_image_free(rgb_image);
    packet->frame.processTime = (uint64_t)(t3-t0);
}

