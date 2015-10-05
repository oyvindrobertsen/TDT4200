
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

__kernel void blue_kernel(__global Pixel* image) {
    int i = get_global_id(0);
    Pixel p = image[i];
    p.r = 0;
    p.g = 255;
    p.b = 0;
    image[i] = p;
}
