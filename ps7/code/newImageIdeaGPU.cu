#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ppmCU.h"

// Image from:
// http://7-themes.com/6971875-funny-flowers-pictures.html

// TODO: You must implement this
// The handout code is much simpler than the MPI/OpenMP versions
//__global__ void performNewIdeaIterationGPU( ... ) { ... }

// TODO: You should implement this
//__global__ void performNewIdeaFinalizationGPU( ... ) { ... }

// Perhaps some extra kernels will be practical as well?
//__global__ void ...GPU( ... ) { ... }

typedef struct {
     float red,green,blue;
} AccuratePixel;

typedef struct {
     int x, y;
     AccuratePixel *data;
} AccurateImage;

__global__ void convertImageToNewFormatGPU(PPMPixel* image, AccuratePixel* imageAccurate) {
    // Calculate data-index
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int w = gridDim.x * blockDim.x;
    int i = y * w + x;

    imageAccurate->data[i].red = (float) image->data[i].red;
    imageAccurate->data[i].green = (float) image->data[i].green;
    imageAccurate->data[i].blue = (float) image->data[i].blue;
}

// Convert a PPM image to a high-precision format 
AccurateImage *convertImageToNewFormat(PPMImage *image) {
    // Allocate space for PPM-image on device
    PPMPixel* devImageData;
    cudaMalloc((void**) &devImageData, sizeof(PPMPixel) * image->x * image->y);
    // Copy image to device
    cudaMemcpy(devImageData, image->data, image->x * image->y * sizeof(PPMPixel), cudaMemcpyHostToDevice);
	// Make a copy
	AccurateImage *imageAccurate = createEmptyImage(image);
    // Allocate space for copy on device
    AccuratePixel* devAccurateImageData;
    cudaMalloc((void**) devAccurateImageData, sizeof(AccuratePixel) * image->x * image->y);

    // Invoke kernel
    dim3 blocks(16, 16);
    dim3 threadsPerBlock(image->x/blocks.x, image->y/blocks.y);

    convertImageToNewFormatGPU<<<blocks, threadsPerBlock>>>(devImageData, devAccurateImageData);
    
    // Retrieve image from device
    cudaMemcpy(imageAccurate->data, devAccurateImageData, image->x * image->y * sizeof(AccuratePixel), cudaMemcpyDeviceToHost);
    // Cleanup
    cudaFree(devImageData);
    cudaFree(devAccurateImageData);

	imageAccurate->x = image->x;
	imageAccurate->y = image->y;
	
	return imageAccurate;
}

// Convert a high-precision format to a PPM image
PPMImage *convertNewFormatToPPM(AccurateImage *image) {
	// Make a copy
	PPMImage *imagePPM;
	imagePPM = (PPMImage *)malloc(sizeof(PPMImage));
	imagePPM->data = (PPMPixel*)malloc(image->x * image->y * sizeof(PPMPixel));
	for(int i = 0; i < image->x * image->y; i++) {
		imagePPM->data[i].red   = (unsigned char) image->data[i].red;
		imagePPM->data[i].green = (unsigned char) image->data[i].green;
		imagePPM->data[i].blue  = (unsigned char) image->data[i].blue;
	}
	imagePPM->x = image->x;
	imagePPM->y = image->y;
	
	return imagePPM;
}

AccurateImage *createEmptyImage(PPMImage *image){
	AccurateImage *imageAccurate;
	imageAccurate = (AccurateImage *)malloc(sizeof(AccurateImage));
	imageAccurate->data = (AccuratePixel*)malloc(image->x * image->y * sizeof(AccuratePixel));
	imageAccurate->x = image->x;
	imageAccurate->y = image->y;
	
	return imageAccurate;
}

// free memory of an AccurateImage
void freeImage(AccurateImage *image){
	free(image->data);
	free(image);
}

__global__ void performNewIdeaIterationGPU(AccuratePixel* imageInData, AccuratePixel* imageOutData, int size) {
    int centerX = blockIdx.x * blockDim.x + threadIdx.x;
    int centerY = blockIdx.y * blockDim.y + threadIdx.y;
    int w = gridDim.x * blockDim.x;
    int h = gridDim.y * blockDim.y;

    float sumR = 0;
    float sumG = 0;
    float sumB = 0;
    int countIncluded = 0;
    for (int x = -size; x <= size; x++) {
        int currentX = centerX + x;
        if (currentX < 0 || currentX >= w) {
            continue;
        };
        for (int y = -size; y <= size; y++) {
            int currentY = centerY + y;
            if(currentY < 0 || currentY >= h) {
                continue;
            }

            // Calculate index
            int i = currentY * w + currentX;
            // Accumulate
            sumR += imageInData[i].red;
            sumG += imageInData[i].green;
            sumB += imageInData[i].blue;
            countIncluded++;
        }
    }

    // Average color values
    float valueR = sumR / countIncluded;
    float valueG = sumG / countIncluded;
    float valueB = sumB / countIncluded;

    // Update outputimage
    int i = centerY * w + centerX;
    imageOutData[i].red = valueR;
	imageOutData[i].green = valueG;
	imageOutData[i].blue = valueB;
}

void performNewIdeaIteration(AccurateImage *imageOut, AccurateImage *imageIn, int size) {
	// Allocate space for imagedata on device
    AccuratePixel* devImageInData, devImageOutData;
    int* devSize;
    cudaMalloc((void**) &devImageInData, sizeof(AccuratePixel) * imageIn->x * imageIn->y);
    cudaMalloc((void**) &devImageOutData, sizeof(AccuratePixel) * imageIn->x * imageIn->y);
    cudaMalloc((void**) &devSize, sizeof(int));
    // Copy image data to device
    cudaMemcpy(devImageInData, imageIn->data, sizeof(AccuratePixel) * imageIn->x * imageIn->y, cudaMemcpyHostToDevice);
    cudaMemcpy(devSize, &size, sizeof(int), cudaMemcpyHostToDevice);
	
    // Invoke kernel
    dim3 blocks(16, 16);
    dim3 threadsPerBlock(imageIn->x/blocks.x, imageIn->y/blocks.y);
    performNewIdeaIterationGPU<<<blocks, threadsPerBlock>>>(devImageInData, devImageOutData, devSize);

    // Retrieve image
    cudaMemcpy(imageOut->data, devImageOutData, sizeof(AccuratePixel) * imageIn->x * imageIn->y, cudaMemcpyDeviceToHost);
    cudaFree(devImageInData);
    cudaFree(devImageOutData);
    cudaFree(devSize);
}

__device__ float threshold(float value) {
    if(value > 255.0f)
		return 255;
	else if (value < -1.0f) {
		value = 257.0f+value;
	    if(value > 255.0f)
			return 255;
		else
			return floorf(value);
	} else if (value > -1.0f && value < 0.0f) {
	    return 0;
	} else {
		return floorf(value);
	}
}

__global__ void performNewIdeaFinalizationGPU(
        AccuratePixel* imageInSmallData, AccuratPixel* imageInLargeData, PPMPixel* imageOutData) {
    // Calculate data-index
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int w = gridDim.x * blockDim.x;
    int i = y * w + x;

    float value = (imageInLarge->data[i].red - imageInSmall->data[i].red);
	imageOutData[i].red = threshold(value);

    float value = (imageInLarge->data[i].green - imageInSmall->data[i].green);
	imageOutData[i].green = threshold(value);

    float value = (imageInLarge->data[i].blue - imageInSmall->data[i].blue);
	imageOutData[i].blue = threshold(value);
}

// Perform the final step, and save it as a ppm in imageOut
void performNewIdeaFinalization(AccurateImage *imageInSmall, AccurateImage *imageInLarge, PPMImage *imageOut) {

	
	imageOut->x = imageInSmall->x;
	imageOut->y = imageInSmall->y;
	
    // Allocate device copies of image data
    AccuratePixel* devImageInSmallData, devImageInLargeData;
    PPMPixel* devImageOutData;
    cudaMalloc((void**) devImageInSmallData, sizeof(AccuratePixel) * imageOut->x * imageOut->y);
    cudaMalloc((void**) devImageInLargeData, sizeof(AccuratePixel) * imageOut->x * imageOut->y);
    cudaMalloc((void**) devImageOutData, sizeof(PPMPixel) * imageOut->x * imageOut->y);
    // Copy image data to device
    cudaMemcpy(devImageInSmallData, imageInSmall->data, sizeof(AccuratePixel) * imageOut->x * imageOut->y, cudaMemcpyHostToDevice);
    cudaMemcpy(devImageInLargeData, imageInLarge->data, sizeof(AccuratePixel) * imageOut->x * imageOut->y, cudaMemcpyHostToDevice);

    // Invoke kernel
    dim3 blocks(16, 16);
    dim3 threadsPerBlock(imageOut->x/blocks.x, imageOut->y/blocks.y);
    performNewIdeaFinalizationGPU<<<blocks, threadsPerBlock>>>(devImageInSmallData, devImageInLargeData, devImageOutData);

    // Retrieve result
    cudaMemcpy(imageOut->data, devImageOutData, sizeof(PPMPixel) * imagOut->x * imageOut->y, cudaMemcpyDeviceToHost);
    cudaFree(devImageInSmallData);
    cudaFree(devImageInLargeData);
    cudaFree(devImageOutData);
}

int main(int argc, char** argv) {
	
	PPMImage *image;
        
	if(argc > 1) {
		image = readPPM("flower.ppm");
	} else {
		image = readStreamPPM(stdin);
	}

    //int* GPUDevice;
    //cudaGetDevice(GPUDevice);
    //struct cudaDeviceProp* props;
    //cudaGetDeviceProperties(props, *GPUDevice);
    //int maxThreadsPerBlock = props->maxThreadsPerBlock;

	AccurateImage *imageUnchanged = convertImageToNewFormat(image); // save the unchanged image from input image
	AccurateImage *imageBuffer = createEmptyImage(image);
	AccurateImage *imageSmall = createEmptyImage(image);
	AccurateImage *imageBig = createEmptyImage(image);
	
	PPMImage *imageOut;
	imageOut = (PPMImage *)malloc(sizeof(PPMImage));
	imageOut->data = (PPMPixel*)malloc(image->x * image->y * sizeof(PPMPixel));

	// Process the tiny case:
	performNewIdeaIteration(imageSmall, imageUnchanged, 2);
	performNewIdeaIteration(imageBuffer, imageSmall, 2);
	performNewIdeaIteration(imageSmall, imageBuffer, 2);
	performNewIdeaIteration(imageBuffer, imageSmall, 2);
	performNewIdeaIteration(imageSmall, imageBuffer, 2);
	
	// Process the small case:
	performNewIdeaIteration(imageBig, imageUnchanged,3);
	performNewIdeaIteration(imageBuffer, imageBig,3);
	performNewIdeaIteration(imageBig, imageBuffer,3);
	performNewIdeaIteration(imageBuffer, imageBig,3);
	performNewIdeaIteration(imageBig, imageBuffer,3);
	
	// save tiny case result
	performNewIdeaFinalization(imageSmall,  imageBig, imageOut);
	if(argc > 1) {
		writePPM("flower_tiny.ppm", imageOut);
	} else {
		writeStreamPPM(stdout, imageOut);
	}

	
	// Process the medium case:
	performNewIdeaIteration(imageSmall, imageUnchanged, 5);
	performNewIdeaIteration(imageBuffer, imageSmall, 5);
	performNewIdeaIteration(imageSmall, imageBuffer, 5);
	performNewIdeaIteration(imageBuffer, imageSmall, 5);
	performNewIdeaIteration(imageSmall, imageBuffer, 5);
	
	// save small case
	performNewIdeaFinalization(imageBig,  imageSmall,imageOut);
	if(argc > 1) {
		writePPM("flower_small.ppm", imageOut);
	} else {
		writeStreamPPM(stdout, imageOut);
	}

	// process the large case
	performNewIdeaIteration(imageBig, imageUnchanged, 8);
	performNewIdeaIteration(imageBuffer, imageBig, 8);
	performNewIdeaIteration(imageBig, imageBuffer, 8);
	performNewIdeaIteration(imageBuffer, imageBig, 8);
	performNewIdeaIteration(imageBig, imageBuffer, 8);

	// save the medium case
	performNewIdeaFinalization(imageSmall,  imageBig, imageOut);
	if(argc > 1) {
		writePPM("flower_medium.ppm", imageOut);
	} else {
		writeStreamPPM(stdout, imageOut);
	}
	
	// free all memory structures
	freeImage(imageUnchanged);
	freeImage(imageBuffer);
	freeImage(imageSmall);
	freeImage(imageBig);
	free(imageOut->data);
	free(imageOut);
	free(image->data);
	free(image);
	
	return 0;
}

