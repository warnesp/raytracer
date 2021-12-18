/********************************
 * Defines some basic color definitions
 ********************************/
#ifndef COLORS_H
#define COLORS_H

#include <math.h>

#define WHITE 1.0, 1.0, 1.0, 1.0
#define BLACK 0.0, 0.0, 0.0, 1.0

#define DARK_GRAY 0.2, 0.2, 0.2, 1.0

#define RED 1.0, 0.0, 0.0, 1.0
#define GREEN 0.0, 1.0, 0.0, 1.0
#define BLUE 0.0, 0.0, 1.0, 1.0

#define CYAN 0.0, 1.0, 1.0, 1.0
#define MAGENTA 1.0, 0.0, 1.0, 1.0
#define YELLOW 1.0, 1.0, 0.0, 1.0
#define ORANGE 1.0, 0.7, 0.0, 1.0

#define INDIGO 75.0/255.0, 0.0, 130.0/255.0, 1.0
#define VIOLET 143.0/255.0, 0.0, 1.0, 1.0

#define YELLOW_ORANGE 1.0, 0.88, 0.0, 1.0
#define OFF_WHITE 0.99, 0.99, 0.95, 1.0


typedef struct color_struct {
	float r, g, b, a;
} color; 

color multiply_colors(color one, color two);
color add_colors(color one, color two);
color scale_color(double scale, color one);

color multiply_colors(color one, color two){
    color result;
    result.r = one.r * two.r;
    result.g = one.g * two.g;
    result.b = one.b * two.b;
    result.a = one.a * two.a;
    return result;
}

/*add two colors together (clamps colors at 0)*/
color add_colors(color one, color two){
    color result;
    result.r = fmax(0, one.r) + fmax(0, two.r);
    result.g = fmax(0, one.g) + fmax(0, two.g);
    result.b = fmax(0, one.b) + fmax(0, two.b);
    result.a = fmax(0, one.a) + fmax(0, two.a);
    return result;
}


color scale_color(double scale, color one){
    color result;
    result.r = one.r * scale;
    result.g = one.g * scale;
    result.b = one.b * scale;
    result.a = one.a * scale;
    return result;
}

#define add_colors3(a, b, c) add_colors( add_colors(a, b), c)


#endif
