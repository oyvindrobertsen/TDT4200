#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/opencl.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <tgmath.h>

#include "lodepng.h"

typedef struct Pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Pixel;

struct Color{
    float angle;
    float intensity;
};

struct CircleInfo{
    float x;
    float y;
    float radius;
    struct Color color;
};

struct LineInfo{
    float x1,y1;
    float x2,y2;
    float thickness;
    struct Color color;
};

char * readText( const char * filename){
    FILE * file = fopen( filename, "r");
    fseek( file, 0, SEEK_END);
    size_t length = ftell(file);
    (void) fseek( file, 0L, SEEK_SET);
    char * content = calloc( length+1, sizeof(char));
    int itemsread = fread( content, sizeof(char), length, file);
    if ( itemsread != length ) {
        printf("Error, reeadText(const char *), failed to read file");
        exit(1);
    }
    return content;
}


void parseLine(char * line, struct LineInfo li[], cl_int *lines){
    float x1,x2,y1,y2,thickness, angle, intensity;
    int items = sscanf(line, "line %f,%f %f,%f %f %f,%f", &x1, &y1, &x2, &y2, &thickness, &angle, &intensity);
    if ( 7 == items ){
        li[*lines].x1 = x1;
        li[*lines].x2 = x2;
        li[*lines].y1 = y1;
        li[*lines].y2 = y2;
        li[*lines].thickness = thickness;
        li[*lines].color.angle = angle;
        li[*lines].color.intensity = intensity;
        (*lines)++;
    }
}


void parseCircle(char * line, struct CircleInfo ci[], cl_int *circles){
    float x,y,radius;
    struct Color c;
    int items = sscanf(line, "circle %f,%f %f %f,%f", &x,&y,&radius, &c.angle, &c.intensity);
    if ( 5==items){
        ci[*circles].x = x;
        ci[*circles].y = y;
        ci[*circles].radius = radius;
        ci[*circles].color.angle = c.angle;
        ci[*circles].color.intensity = c.intensity;
        (*circles)++;
    }
}


void printLines(struct LineInfo li[], cl_int lines){
    for ( int i = 0 ; i < lines ; i++){
        printf("line:	from:%f,%f to:%f,%f thick:%f,	%f,%f\n", li[i].x1, li[i].y1, li[i].x2, li[i].y2, li[i].thickness,li[i].color.angle, li[i].color.intensity);
    }
}


void printCircles(struct CircleInfo ci[], cl_int circles){
    for ( int i = 0 ; i < circles ; i++){
        printf("circle %f,%f %f %f,%f\n", ci[i].x,ci[i].y,ci[i].radius, ci[i].color.angle, ci[i].color.intensity);
    }
}


int main(){

    // Parse input
    int numberOfInstructions;
    char* *instructions = NULL;
    size_t *instructionLengths;

    struct CircleInfo *circleinfo;
    cl_int circles = 0;
    struct LineInfo *lineinfo;
    cl_int lines = 0;

    char *line = NULL;
    size_t linelen = 0;
    int width=0, height = 0;

    // Read size of canvas
    ssize_t read = getline( & line, &linelen, stdin);
    sscanf( line, "%d,%d" , &width,&height);

    // Read amount of primitives
    read = getline( & line, &linelen, stdin);
    sscanf( line, "%d" , & numberOfInstructions);

    // Allocate memory for primitives
    instructions = calloc(sizeof(char*),numberOfInstructions);
    instructionLengths = calloc( sizeof(size_t), numberOfInstructions);
    circleinfo = calloc( sizeof(struct CircleInfo), numberOfInstructions);
    lineinfo = calloc( sizeof(struct LineInfo), numberOfInstructions);

    // Read in each primitive
    for ( int i =0 ; i < numberOfInstructions; i++) {
        ssize_t read = getline( &instructions[i] , &instructionLengths[i] , stdin);
        if (instructions[i][0] == 'l') {
            parseLine(instructions[i], lineinfo, &lines);
        } else {
            // We assume circle
            parseCircle(instructions[i], circleinfo, &circles);
        }
    }

    // Output setup
    Pixel* image = calloc(sizeof(Pixel), width*height);

    // Connect to a compute device
    int err;
    cl_device_id device_id;
    err = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
    if (err != CL_SUCCESS) {
        printf("Error: Failed to create a device group!\n");
        return EXIT_FAILURE;
    }

    // Create context
    cl_context context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
    if (!context || err != CL_SUCCESS) {
        printf("Error: Failed to create a compute context!\n");
        return EXIT_FAILURE;
    }


    // Create command queue
    cl_command_queue commands = clCreateCommandQueue(context, device_id, 0, &err);
    if (!commands || err != CL_SUCCESS) {
        printf("Error: Failed to create command queue!\n");
        return EXIT_FAILURE;
    }

    // Create program from source file
    char * source = readText("kernel.cl");
    cl_program program = clCreateProgramWithSource(
            context, 1,
            (const char **) &source,
            NULL, &err);

    if (!program || err != CL_SUCCESS) {
        printf("Error: Failed to create program!\n");
    }

    // Build the program
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        size_t len;
        printf("Error: Failed to build program executable!\n");
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, NULL, NULL, &len);
        char* log = malloc(sizeof(char)*len);
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, len, log, NULL);
        printf("%s\n", log);
        return EXIT_FAILURE;
    }

    // Create Kernel / transfer data to device
    cl_kernel kernel = clCreateKernel(program, "blue_kernel", &err);
    if (!kernel || err != CL_SUCCESS) {
        printf("Error: Failed to create kernel!\n");
        printf("Error code: %d", err);
        return EXIT_FAILURE;
    }

    cl_mem dev_image = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(Pixel) * width*height, NULL, NULL);
    if (!dev_image) {
        printf("Error: could not allocate memory on device!\n");
        return EXIT_FAILURE;
    }

    err = 0;
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &dev_image);

    // Query for maximum number of work items per workgroup
    size_t group_size;
    err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(group_size), &group_size, NULL);

    // Execute Kernel / transfer result back from device
    const size_t work_items = (size_t) width*height;
    err = clEnqueueNDRangeKernel(
            commands,
            kernel,
            1,
            NULL,
            &work_items,
            &group_size,
            0,
            NULL,
            NULL);

    // Wait for kernels to finish
    clFinish(commands);

    Pixel* out_image = calloc(sizeof(Pixel), width*height);
    err = clEnqueueReadBuffer(commands, dev_image, CL_TRUE, 0, sizeof(Pixel) * width * height, out_image, 0, NULL, NULL );

    uint8_t* image_output_buffer = calloc(sizeof(uint8_t), width*height*3);
    int j = 0;
    for (int i = 0; i < width*height; i++) {
        Pixel p = out_image[i];
        image_output_buffer[j] = p.r;
        image_output_buffer[j+1] = p.g;
        image_output_buffer[j+2] = p.b;
        j = j + 3;
    }
    size_t memfile_length = 0;
    unsigned char * memfile = NULL;
    lodepng_encode24(
            &memfile,
            &memfile_length,
            image_output_buffer,
            width,
            height);

    // KEEP THIS LINE. Or make damn sure you replace it with something equivalent.
    // This "prints" your png to stdout, permitting I/O redirection
    fwrite( memfile, sizeof(unsigned char), memfile_length, stdout);
    return 0;
}
