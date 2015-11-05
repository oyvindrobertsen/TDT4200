#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include "ppm.h"


typedef struct {
    float red,green,blue;
} AccuratePixel;

typedef struct {
    int x, y;
    AccuratePixel *data;
} AccurateImage;

// Convert ppm to high precision format.
AccurateImage *convertImageToNewFormat(PPMImage *image) {
    // Make a copy
    AccurateImage *imageAccurate;
    imageAccurate = (AccurateImage *)malloc(sizeof(AccurateImage));
    imageAccurate->data = (AccuratePixel*)malloc(image->x * image->y * sizeof(AccuratePixel));
    for(int i = 0; i < image->x * image->y; i++) {
        imageAccurate->data[i].red   = (float) image->data[i].red;
        imageAccurate->data[i].green = (float) image->data[i].green;
        imageAccurate->data[i].blue  = (float) image->data[i].blue;
    }
    imageAccurate->x = image->x;
    imageAccurate->y = image->y;

    return imageAccurate;
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

// Perform the new idea:
// The code in this function should run in parallel
// Try to find a good strategy for dividing the problem into individual parts.
// Using OpenMP inside this function itself might be avoided
// You may be able to do this only with a single OpenMP directive
void performNewIdeaIteration(AccurateImage *imageOut, AccurateImage *imageIn, int size, int startY, int stopY) {
    int img_w = imageIn->x;

    // line buffer that will save the sum of some pixel in the column
    AccuratePixel *line_buffer = (AccuratePixel*) malloc(imageIn->x*sizeof(AccuratePixel));
    memset(line_buffer, 0, imageIn->x * sizeof(AccuratePixel));

    //  Non-first threads need to read from the preceeding rows as well as its startY/stopY range
    int preY
    if (startY > 0) {
        preY = startY - size;
    }

    // Iterate over each line of pixelx.
    for(int centerY = preY; centerY < stopY; centerY++) {
        // first and last line considered  by the computation of the pixel in the line centerY
        int starty = centerY - size;
        int endy = centerY + size;

        // Initialize and update the line_buffer.
        // For OpenMP this might cause problems
        // Separating out the initialization part might help
        if (starty <= preY) {
            starty = preY;
            if (centerY == preY) {
                // for all pixel in the first line, we sum all pixel of the column (until the line endy)
                // we save the result in the array line_buffer
                for (int line_y = starty; line_y < endy; line_y++) {
                    for (int i = 0; i < imageIn->x; i++) {
                        int add = img_w * line_y + i;
                        line_buffer[i].blue += imageIn->data[add].blue;
                        line_buffer[i].red += imageIn->data[add].red;
                        line_buffer[i].green += imageIn->data[add].green;
                    }
                }
            }
            for (int i = 0; i < imageIn->x; i++) {
                // add the next pixel of the next line in the column x
                int add = img_w * endy + i;
                line_buffer[i].blue += imageIn->data[add].blue;
                line_buffer[i].red += imageIn->data[add].red;
                line_buffer[i].green += imageIn->data[add].green;
            }
        } else if (endy >= imageIn->y) {
            // for the last lines, we just need to subtract the first added line
            endy = imageIn->y - 1;
            for (int i = 0; i < imageIn->x; i++) {
                int sub = img_w * (starty - 1) + i;
                line_buffer[i].blue -= imageIn->data[sub].blue;
                line_buffer[i].red -= imageIn->data[sub].red;
                line_buffer[i].green -= imageIn->data[sub].green;
            }
        } else {
            // general case
            // add the next line and remove the first added line
            for (int i = 0; i < imageIn->x; i++) {
                int add = img_w * endy + i;
                int sub = img_w * (starty - 1) + i;
                line_buffer[i].blue += imageIn->data[add].blue - imageIn->data[sub].blue;
                line_buffer[i].red += imageIn->data[add].red - imageIn->data[sub].red;
                line_buffer[i].green += imageIn->data[add].green - imageIn->data[sub].green;
            }
        }
        // End of line_buffer initialisation.

        if (centerY >= startY) {
            float sum_green = 0;
            float sum_red = 0;
            float sum_blue = 0;
            for (int centerX = 0; centerX < imageIn->x; centerX++) {
                // in this loop, we do exactly the same thing as before but only with the array line_buffer

                int startx = centerX - size;
                int endx = centerX + size;

                if (startx <= 0) {
                    startx = 0;
                    if (centerX == 0) {
                        for (int x = startx; x < endx; x++) {
                            sum_red += line_buffer[x].red;
                            sum_green += line_buffer[x].green;
                            sum_blue += line_buffer[x].blue;
                        }
                    }
                    sum_red += line_buffer[endx].red;
                    sum_green += line_buffer[endx].green;
                    sum_blue += line_buffer[endx].blue;
                } else if (endx >= imageIn->x) {
                    endx = imageIn->x - 1;
                    sum_red -= line_buffer[startx - 1].red;
                    sum_green -= line_buffer[startx - 1].green;
                    sum_blue -= line_buffer[startx - 1].blue;
                } else {
                    sum_red += line_buffer[endx].red - line_buffer[startx - 1].red;
                    sum_green += line_buffer[endx].green - line_buffer[startx - 1].green;
                    sum_blue += line_buffer[endx].blue - line_buffer[startx - 1].blue;
                }

                // we save each pixel in the output image
                int i = img_w * centerY + centerX;
                int count = (endx - startx + 1) * (endy - starty + 1);

                imageOut->data[i].red = sum_red / count;
                imageOut->data[i].green = sum_green / count;
                imageOut->data[i].blue = sum_blue / count;
            }
        }
    }

    free(line_buffer);	
}

unsigned char threshold(float value) {
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

void performNewIdeaFinalization(AccurateImage *imageInSmall, AccurateImage *imageInLarge, PPMImage *imageOut) {
    imageOut->x = imageInSmall->x;
    imageOut->y = imageInSmall->y;

    for(int i = 0; i < imageInSmall->x * imageInSmall->y; i++) {
        float value = imageInLarge->data[i].red - imageInSmall->data[i].red;
        imageOut->data[i].red = threshold(value);
        value = imageInLarge->data[i].green - imageInSmall->data[i].green;
        imageOut->data[i].green = threshold(value);
        value = imageInLarge->data[i].blue - imageInSmall->data[i].blue;
        imageOut->data[i].blue = threshold(value);
    }
}

void performNewIdeaIterationOMP(AccurateImage *imageOut, AccurateImage *imageIn, int size) {
#pragma omp parallel
    {
        int d = imageIn->y / omp_get_num_threads();
        int startY = d * omp_get_thread_num();
        int stopY;
        if (startY + d > imageIn->y) {
            stopY = imageIn->y;
        } else {
            stopY = startY + d;
        }
        performNewIdeaIteration(imageOut, imageIn, size, startY, stopY);
    }
}

int main(int argc, char** argv) {
    PPMImage *image;

    if(argc > 1) {
        image = readPPM("flower.ppm");
    } else {
        image = readStreamPPM(stdin);
    }

    AccurateImage *imageUnchanged = convertImageToNewFormat(image); // save the unchanged image from input image
    AccurateImage *imageBuffer = createEmptyImage(image);
    AccurateImage *imageSmall = createEmptyImage(image);
    AccurateImage *imageBig = createEmptyImage(image);

    PPMImage *imageOut;
    imageOut = (PPMImage *)malloc(sizeof(PPMImage));
    imageOut->data = (PPMPixel*)malloc(image->x * image->y * sizeof(PPMPixel));

    // Process the tiny case:
    performNewIdeaIterationOMP(imageSmall, imageUnchanged, 2);
    performNewIdeaIterationOMP(imageBuffer, imageSmall, 2);
    performNewIdeaIterationOMP(imageSmall, imageBuffer, 2);
    performNewIdeaIterationOMP(imageBuffer, imageSmall, 2);
    performNewIdeaIterationOMP(imageSmall, imageBuffer, 2);

    // Process the small case:
    performNewIdeaIterationOMP(imageBig, imageUnchanged,3);
    performNewIdeaIterationOMP(imageBuffer, imageBig,3);
    performNewIdeaIterationOMP(imageBig, imageBuffer,3);
    performNewIdeaIterationOMP(imageBuffer, imageBig,3);
    performNewIdeaIterationOMP(imageBig, imageBuffer,3);

    // save tiny case result
    performNewIdeaFinalization(imageSmall,  imageBig, imageOut);
    if(argc > 1) {
        writePPM("flower_tiny.ppm", imageOut);
    } else {
        writeStreamPPM(stdout, imageOut);
    }


    // Process the medium case:
    performNewIdeaIterationOMP(imageSmall, imageUnchanged, 5);
    performNewIdeaIterationOMP(imageBuffer, imageSmall, 5);
    performNewIdeaIterationOMP(imageSmall, imageBuffer, 5);
    performNewIdeaIterationOMP(imageBuffer, imageSmall, 5);
    performNewIdeaIterationOMP(imageSmall, imageBuffer, 5);

    // save small case
    performNewIdeaFinalization(imageBig,  imageSmall,imageOut);
    if(argc > 1) {
        writePPM("flower_small.ppm", imageOut);
    } else {
        writeStreamPPM(stdout, imageOut);
    }

    // process the large case
    performNewIdeaIterationOMP(imageBig, imageUnchanged, 8);
    performNewIdeaIterationOMP(imageBuffer, imageBig, 8);
    performNewIdeaIterationOMP(imageBig, imageBuffer, 8);
    performNewIdeaIterationOMP(imageBuffer, imageBig, 8);
    performNewIdeaIterationOMP(imageBig, imageBuffer, 8);

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

