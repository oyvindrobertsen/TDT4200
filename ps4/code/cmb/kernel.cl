
// Write any additional support functions as well as the actual kernel function in this file

typedef struct Color{
    float angle;
    float intensity;
} Color;

typedef struct CircleInfo{
    float x;
    float y;
    float radius;
    Color color;
} CircleInfo;

typedef struct LineInfo{
    float x1,y1;
    float x2,y2;
    float thickness;
    Color color;
} LineInfo;

typedef struct Pixel {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} Pixel;

float red( float deg ) {
    float a1 = 1.f/60;
    float b1 = 2;
    float a2 = -1.f/60;
    float b2 = 2;
    float asc = deg*a2+b2;
    float desc = deg*a1+b1;
    return fmax( .0f , fmin( 1.f, fmin(asc,desc)));
}

float green( float deg ) {
    float a1 = 1.f/60;
    float b1 = 0;
    float a2 = -1.f/60;
    float b2 = 4;
    float asc = deg*a2+b2;
    float desc = deg*a1+b1;
    return fmax( .0f , fmin( 1.f, fmin(asc,desc)));
}

float blue( float deg ) {
    float a1 = 1.f/60;
    float b1 = -2;
    float a2 = -1.f/60;
    float b2 = 6;
    float asc = deg*a2+b2;
    float desc = deg*a1+b1;
    return fmax( .0f , fmin( 1.f, fmin(asc,desc)));
}


float my_distance(float x1, float y1, float x2, float y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

__kernel void blue_kernel(__global Pixel* image) {
    int i = get_global_id(0);
    Pixel p = image[i];
    p.r = 0;
    p.g = 0;
    p.b = 255;
    image[i] = p;
}

__kernel void render_pixel(__global Pixel* image,
        __global CircleInfo* circle_list,
        __global LineInfo* line_list,
        const int circle_count,
        const int line_count,
        const int width,
        const int height) {
    int i = get_global_id(0);
    float x, y;
    x = (i % width) / (float) width;
    y = (i / width) / (float) height;
    Pixel p = image[i];

    // Circles
    for (int j = 0; j < circle_count; j++) {
        struct CircleInfo c = circle_list[j];
        if (my_distance(c.x, c.y, x, y) <= c.radius) {
            p.r += (int) (c.color.intensity * red(c.color.angle));
            p.g += (int) (c.color.intensity * green(c.color.angle));
            p.b += (int) (c.color.intensity * blue(c.color.angle));
        }
    }

    // Lines
    for (int k = 0; k < line_count; k++) {
        struct LineInfo l = line_list[k];
        if (fabs((my_distance(l.x1, l.y1, x, y) + my_distance(l.x2, l.y2, x, y)) - my_distance(l.x1, l.y1, l.x2, l.y2)) < 0.0001) {
            p.r += (int) (l.color.intensity * red(l.color.angle));
            p.g += (int) (l.color.intensity * green(l.color.angle));
            p.b += (int) (l.color.intensity * blue(l.color.angle));
        }
    }
    image[i] = p;
}
