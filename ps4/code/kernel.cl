
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
    int i_x = get_global_id(0);
    int i_y = get_global_id(1);
    float x = i_x / (float) width;
    float y = i_y / (float) height;
    Pixel p = image[i_y*width + i_x];

    p.r = 0;
    p.g = 0;
    p.b = 0;

    // Circles
    for (int j = 0; j < circle_count; j++) {
        struct CircleInfo c = circle_list[j];
        if (my_distance(c.x, c.y, x, y) <= c.radius) {
            p.r = min(255, (int)(p.r + (c.color.intensity * red(c.color.angle))));
            p.g = min(255, (int)(p.g + (c.color.intensity * green(c.color.angle))));
            p.b = min(255, (int)(p.b + (c.color.intensity * blue(c.color.angle))));
        }
    }

    // Lines
    for (int k = 0; k < line_count; k++) {
        struct LineInfo l = line_list[k];
        float point_to_line = fabs((l.y2 - l.y1) * x - (l.x2 - l.x1) * y + l.x2 * l.y1 - l.y2 * l.x1)/ my_distance(l.x1, l.y1, l.x2, l.y2);
        if (point_to_line < l.thickness && x > min(l.x1, l.x2) - l.thickness/2 && x < max(l.x1, l.x2) + l.thickness/2 && y > min(l.y1, l.y2) - l.thickness && y < max(l.y1, l.y2) + l.thickness) {
            p.r = min(255, (int)(p.r + (l.color.intensity * red(l.color.angle))));
            p.g = min(255, (int)(p.g + (l.color.intensity * green(l.color.angle))));
            p.b = min(255, (int)(p.b + (l.color.intensity * blue(l.color.angle))));
        }
    }
    image[i_y*width + i_x] = p;
}
