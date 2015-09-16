#include <iostream>
#include <stdio.h>
#include <time.h>
#include "lodepng.h"

__global__ void invert_character(char* image) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    image[x] = ~image[x];
}

int main( int argc, char ** argv){

    size_t pngsize;
    unsigned char *png;
    const char * filename = "lenna512x512_inv.png";
    /* Read in the image */
    lodepng_load_file(&png, &pngsize, filename);

    unsigned char *image;
    unsigned int width, height;
    /* Decode it into a RGB 8-bit per channel vector */
    unsigned int error = lodepng_decode24(&image, &width, &height, png, pngsize);

    /* Check if read and decode of .png went well */
    if(error != 0){
        std::cout << "error " << error << ": " << lodepng_error_text(error) << std::endl;
    }

    // Setup timing
    float there, back_again;
    cudaEvent_t start_1, start_2, end_1, end_2;
    cudaEventCreate(&start_1);
    cudaEventCreate(&start_2);
    cudaEventCreate(&end_1);
    cudaEventCreate(&end_2);

    char* dev_img;
    cudaMalloc((void**)&dev_img, sizeof(char)*width*height*3);

    cudaEventRecord(start_1, 0);
    cudaMemcpy(dev_img, image, width*height*3*sizeof(char), cudaMemcpyHostToDevice);
    cudaEventRecord(end_1, 0);
    cudaEventSynchronize(end_1);
    cudaEventElapsedTime(&there, start_1, end_1);

    invert_character<<<512*3, 512>>>(dev_img);

    cudaEventRecord(start_2, 0);
    cudaMemcpy(image, dev_img, sizeof(char)*width*height*3, cudaMemcpyDeviceToHost);
    cudaEventRecord(end_2, 0);
    cudaEventSynchronize(end_2);
    cudaEventElapsedTime(&back_again, start_2, end_2);

    cudaEventDestroy(start_1);
    cudaEventDestroy(start_2);
    cudaEventDestroy(end_1);
    cudaEventDestroy(end_2);

    cudaFree(dev_img);
    /* Save the result to a new .png file */
    lodepng_encode24_file("lenna512x512_orig.png", image , width,height);
    free(image);

    float total_mem_time = there + back_again;
    printf("Total time spent transferring data: %f\n", total_mem_time);

    return 0;
}

