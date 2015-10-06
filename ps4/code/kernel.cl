
// Write any additional support functions as well as the actual kernel function in this file

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

float distance(int x1, int y1, int x2, int y2) {
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
        __global size_t circle_count,
        __global size_t line_count,
        __global size_t width) {
    int i = get_global_id(0);
    int x, y;
    x = i % width;
    y = i / width;
    Pixel p = image[i];

    // Circles
    for (int j = 0; j < circle_count; j++) {
        CircleInfo c = circle_list[j];
        if (distance(c.x, c.y, x, y) <= c.radius) {
            p.r += red(c.color)
        }
    }

    // Lines
    for (int k = 0; k < line_count; k++) {
        LineInfo l = line_list[k];
        // TODO: Point P is along the line between A and B
        // if distance(A, P) + distance(P, B) == distance(A,B)
    }
}
