/*********************
 * Paul Warnes
 * CS 645
 * Programming Assignment Five
 * Due 28 Nov 2012
 * 
 * A simple raytracer which models reflections and not refractions.
 * In addition the raytraced scene will be framed by spline patches on
 * either side modeling sine curves.
 * 
 * The scene to be depicted shows several spheres reflecting off eachother,
 * and the floor which is a mirror. Note the scene is rendered in a
 * parallel view volume. Double buffering is used to show the scene.
 * 
 * The spline curves are rendered with normal Phong Illumination and
 * do not interact with the ray traced portion of the scene.
 * 
 * Key commands:
 *      'G' - render the scene
 *      'L' - toggle the light '20' units to the right
 *      'X' - exit the program
 * 
 * Notable functions and structures:
 * 
 * keyboard_input - the keyboard callback hander to accept user input
 * display_func - display callback function
 * 
 * phong_sphere - used to apply Phong Illumination to a sphere
 * cast_ray - apply the raycasting algorithm
 * compute_scene - computes the scene and stores in an intermediate buffer
 *                      scene
 * find_intersection - finds the intersection of a sphere and a ray
 * 
 * add_sphere - used to add a sphere to the linked list
 * init_light - initializes a light in openGL
 * draw_spline_surface - uses openGL commands to draw a spline patch
 * draw_splines - draws a sin approximation spline patch at the given
 *                  offset, enables Phong lighting
 * 
 * 
 * 
 * scene - an array that holds the computed color values for the scene from
 *              the ray tracing algorithm
 * sphere - structure to represent a sphere
 * sphere_list - structure to represent a list of spheres and their
 *                  material properties (linked list)
 * light - holds information about a light
 * spline_material_* - material components for splines
 * 
 */
#include "colors.h"
#include "geometry.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>

#include "my_setup.h"

#define CANVAS_WIDTH 300
#define CANVAS_HEIGHT 300

#define SCENE_WIDTH 100
#define SCENE_HEIGHT 100

#define SPLINE_WIDTH 250

bool show_message = true;
unsigned int max_ray_depth = 4;

double scene_floor = -SCENE_HEIGHT/2;

color canvas[CANVAS_WIDTH*CANVAS_HEIGHT];

/*spline material components*/
float spline_material_a[4] = {0.1,0.6,0.1, 1.0};
float spline_material_d[4] = {0.4,0.8,0.4, 1.0};
float spline_material_s[4] = {0.3,0.75,0.3, 1.0};


/*a structure to represent a list of spheres and their properties*/
typedef struct sphere_list_struct {
    sphere s;
    color ambient, diffuse, specular;
    double s_exp, reflectivity;
    struct sphere_list_struct * next;
} sphere_list;


/*represents a light by location and coloration*/
typedef struct light_struct {
    point location;
    color ambient, diffuse, specular;
} light;


/*the single light for the scene*/
light light0;
/*list of spheres in the scene*/
sphere_list * list;


/*finds the Phong Illumination at the given point on the sphere at center sphere_center
 * with the given material properties*/
color phong_sphere(point sphere_center, point p, point viewer, color ambient, color diffuse, 
                    color specular, double specular_exp, light lght){
    vector l = normalize_vector(points_to_vector(p,lght.location));
    vector v = normalize_vector(points_to_vector(p, viewer));
    vector n = normalize_vector(points_to_vector( sphere_center, p));
    vector h = scale_vector(0.5, add_vectors(l, v));
    
    color ambient_r = multiply_colors(ambient, lght.ambient);
    
    color diffuse_r = scale_color(dot_vector(l, n), 
                        multiply_colors(diffuse, lght.diffuse));
    double spec_scale1 = fabs(dot_vector(h,n));
    double spec_scale2 = specular_exp;
    color specular_r = scale_color(pow(fabs(dot_vector(h,n)), specular_exp), 
                          multiply_colors(specular, lght.specular));
                          
    return add_colors3(ambient_r, diffuse_r, specular_r); 
    
}

/*cast a ray into the scene, depth is used to stop the recursion*/
color cast_ray(ray r, int depth){
    color tmp1, tmp2, tmp3, tmp4;
    
    vector normal, incident, reflect;
    double cosi;
    color result = {BLACK}, reflect_color;
    point p, p_saved;
    bool found = false, any_found = false;
    sphere_list * sl = list, sl_closest;
    
    if(depth++ >= max_ray_depth){
        return result;
    }
    //find intersection
    while(sl != NULL){
        p = find_intersection(sl->s, r, &found);
        /*only update if intersection was found*/
        if(found){
            /*use if closer*/
            if(!any_found 
               || (distance_sq(r.orgin, p) < distance_sq(p_saved, r.orgin))){ 
                sl_closest = (*sl);
                p_saved = p;
                any_found = true;
            }
            found = false;
        }
        sl = (sl->next);
    }
    
    if(!any_found){
        //test for intersection with bottom
        if(r.at.y - r.orgin.y < 0){
            p_saved = find_y_plane_intersection(r, scene_floor);
            incident =  normalize_vector(ray_to_vector(r));
            normal.x = 0;
            normal.y = 1;
            normal.z = 0;
            // use bottom as mirror
            cosi = dot_vector(scale_vector(-1, incident), normal);
            reflect = normalize_vector(add_vectors(incident,scale_vector(2*cosi, normal)));
            reflect_color = cast_ray(point_vector_to_ray(p_saved, reflect), depth);
            result = add_colors(result, reflect_color);
        }
        /*no intersections means the light has left the scene*/
        return result;
    }
    
    //do Phong
    result = phong_sphere(sl_closest.s.center, p_saved, r.orgin, 
        sl_closest.ambient, sl_closest.diffuse, sl_closest.specular, sl_closest.s_exp,
        light0);
    
    //cast reflection
    incident =  normalize_vector(ray_to_vector(r));
    normal = normalize_vector(points_to_vector(sl_closest.s.center, p_saved));
    cosi = dot_vector(scale_vector(-1, incident), normal);
    reflect = normalize_vector(add_vectors(incident,scale_vector(2*cosi, normal)));
    
    
    reflect_color = cast_ray(point_vector_to_ray(p_saved, reflect), depth);
    result.r += sl_closest.reflectivity * reflect_color.r;
    result.g += sl_closest.reflectivity * reflect_color.g;
    result.b += sl_closest.reflectivity * reflect_color.b;
    

    //cast refraction here if desired
    
    
    return result;
}

/*colors the given pixel with the given color*/
void color_pixel(double x, double y, color c){
    
    glColor3f(c.r, c.g, c.b);
    glVertex2d(x, y);
    
    
}

/*cast rays out of every pixel in the given square*/
void compute_scene(int x1, int y1, int x2, int y2){
    int x, y;
    double height_ratio = (SCENE_HEIGHT/(double)CANVAS_HEIGHT),
            width_ratio = (SCENE_WIDTH/(double)CANVAS_WIDTH);
    int count = 0;
    ray r;
    
    r.orgin.z = 0.0;
    r.at.z = -1.0;
    
    for(y = 0; y < CANVAS_HEIGHT; ++y){
        for(x = 0; x < CANVAS_WIDTH; ++x){
            
            r.orgin.x = r.at.x = ((double)x)*width_ratio + x1; 
            r.orgin.y = r.at.y = ((double)y)*height_ratio + y1;
            
            canvas[y*CANVAS_WIDTH + x] = cast_ray(r, 0);
        }
    }
    
}

/* draws the given null terminated string str to the string 
 * at position (x, y) */
void drawString(int x, int y, char str[]) {
    
    int i;
    glRasterPos2i(x,y);
    for(i = 0; str[i] != '\0'; ++i) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, str[i]);
    }
}

/*initializes a light in openGL*/
void init_light(GLenum l_enum, light l){
    float pos[4] = {l.location.x, l.location.y, l.location.z, 1};
    float amb[4] = {l.ambient.r, l.ambient.g, l.ambient.b, l.ambient.a};
    float dif[4] = {l.diffuse.r, l.diffuse.g, l.diffuse.b, l.diffuse.a};
    float spe[4] = {l.specular.r, l.specular.g, l.specular.b, l.specular.a};
    
    glLightfv(l_enum, GL_POSITION, pos);
    glLightfv(l_enum, GL_AMBIENT, amb);
    glLightfv(l_enum, GL_DIFFUSE, dif);
    glLightfv(l_enum, GL_SPECULAR, spe);
}

/*turns the given control points into a visible spline pathch*/
void draw_spline_surface(float * ctrl_points){
    glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4,
           0, 1, 12, 4, ctrl_points);
    
    glEnable(GL_MAP2_VERTEX_3);
    
           
    glMapGrid2f(20, 0, 1, 10, 0, 1);
    glEvalMesh2(GL_FILL, 0, 20, 0, 10);
    
    glMapGrid2f(20, 0.0, 1.0, 10, 0.0, 1.0);
}

/*draws a spline patch at the given x offset*/
void draw_splines(double left){
    float div = SPLINE_WIDTH/12;
    float low_z = -60, mid_z = -30, high_z = 0.0;
    float y1 = 0, y2 = CANVAS_HEIGHT/3, y3 = CANVAS_HEIGHT*2/3, y4 = CANVAS_HEIGHT;
    float ctrl_points1[4][4][3] = {
   { {left, y1,   mid_z},    {div+left, y1, high_z},    {div*2+left, y1, low_z},   {div*3+left, y1, mid_z}}, 
   { {left, y2, mid_z},  {div+left, y2, high_z},  {div*2+left, y2, low_z},  {div*3+left, y2, mid_z}}, 
   { {left, y3, mid_z},  {div+left, y3, high_z},  {div*2+left, y3, low_z},  {div*3+left, y3, mid_z}}, 
   { {left, y4, mid_z}, {div+left, y4, high_z}, {div*2+left, y4, low_z},  {div*3+left, y4, mid_z}}
   };/**/
   float ctrl_points2[4][4][3] = {
   { {left+div*3, y1, mid_z},    {div*4+left, y1, high_z},    {div*5+left, y1, low_z},   {div*6+left, y1, mid_z}}, 
   { {left+div*3, y2, mid_z},  {div*4+left, y2, high_z},  {div*5+left, y2, low_z},  {div*6+left, y2, mid_z}}, 
   { {left+div*3, y3, mid_z},  {div*4+left, y3, high_z},  {div*5+left, y3, low_z},  {div*6+left, y3, mid_z}}, 
   { {left+div*3, y4, mid_z}, {div*4+left, y4, high_z}, {div*5+left, y4, low_z},  {div*6+left, y4, mid_z}}
   };/**/
   float ctrl_points3[4][4][3] = {
   { {left+div*6, y1, mid_z},    {div*7+left, y1, high_z},    {div*8+left, y1, low_z},   {div*9+left, y1, mid_z}}, 
   { {left+div*6, y2, mid_z},  {div*7+left, y2, high_z},  {div*8+left, y2, low_z},  {div*9+left, y2, mid_z}}, 
   { {left+div*6, y3, mid_z},  {div*7+left, y3, high_z},  {div*8+left, y3, low_z},  {div*9+left, y3, mid_z}}, 
   { {left+div*6, y4, mid_z}, {div*7+left, y4, high_z}, {div*8+left, y4, low_z},  {div*9+left, y4, mid_z}}
   };/**/
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho((GLdouble) left, (GLdouble) SPLINE_WIDTH/2 + left, 
            (GLdouble) 0, (GLdouble) SCENE_HEIGHT, -100, 100);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    /*Turn on phong*/
    glEnable(GL_LIGHTING);
    
    glMaterialf(GL_FRONT, GL_SHININESS, 5.0);
    glMaterialfv(GL_FRONT, GL_AMBIENT, spline_material_a);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, spline_material_a);
    glMaterialfv(GL_FRONT, GL_SPECULAR, spline_material_s);
    draw_spline_surface(&ctrl_points1[0][0][0]);
    draw_spline_surface(&ctrl_points2[0][0][0]);
    draw_spline_surface(&ctrl_points3[0][0][0]);
    
    /*set everything back to normal*/
    glDisable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    
}

void setup_raytrace_camera(){
    double view_port_start = SPLINE_WIDTH/2;
    glViewport(view_port_start, 0, CANVAS_WIDTH, CANVAS_HEIGHT);
    glMatrixMode(GL_PROJECTION);

    //setting viewing parameters to simple 2D viewing
    glLoadIdentity();
    gluOrtho2D((GLdouble) -CANVAS_WIDTH/2 - view_port_start, 
                (GLdouble) CANVAS_WIDTH/2 - view_port_start, 
                (GLdouble) -CANVAS_HEIGHT/2, (GLdouble) CANVAS_HEIGHT/2);
    
    glMatrixMode(GL_MODELVIEW);
}

/*display callback handler*/
void display_func(){
    int x, y;
    double view_port_start = SPLINE_WIDTH/2;
    
    
    glViewport(0, 0, view_port_start, CANVAS_HEIGHT);
    draw_splines(0);
    
    glViewport(CANVAS_WIDTH + view_port_start, 0, 
                view_port_start, CANVAS_HEIGHT);
    draw_splines(SPLINE_WIDTH/2 + CANVAS_WIDTH);
    
    
    setup_raytrace_camera();
    
    /*use points to draw each pixel*/
    glBegin(GL_POINTS);
    
    /*color every pixel from the computed scene*/
    for(y = 0; y < CANVAS_HEIGHT; ++y){
        for(x = 0; x < CANVAS_WIDTH; ++x) {
            color_pixel(x - my_width/2, 
                        y - my_height/2, canvas[y*CANVAS_WIDTH + x]);
        }
    }
    glEnd();
    
    
    if(show_message){
        glViewport(view_port_start, 0, 
                    CANVAS_WIDTH+view_port_start, CANVAS_HEIGHT);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D((GLdouble) 0, (GLdouble) SCENE_WIDTH, 
                    (GLdouble) 0, (GLdouble) SCENE_HEIGHT);
        glColor4f(GREEN);
        drawString(1,20, "Press G to begin simulation." );
        drawString(1,15, "Press X to exit when you are finished viewing the image.");
        
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }
    
    
    /*swap the buffer and request a draw*/
    glutSwapBuffers();
    glFinish();
    
}


bool light_toogle = false;
/*keyboard callback handler*/
void keyboard_input(unsigned char key, int x, int y){
    
    if(key == 'L' || key == 'l'){
        
        light0.location.x = light0.location.x > 10 ? 0 : 20 ;
        glDisable(light_toogle ? GL_LIGHT1 : GL_LIGHT0);
        glEnable(light_toogle ? GL_LIGHT0 : GL_LIGHT1);
        light_toogle = !light_toogle;
        
        if(!show_message){
            compute_scene(-SCENE_WIDTH/2,-SCENE_HEIGHT/2,
                            SCENE_WIDTH/2,SCENE_HEIGHT/2);
        }
        
        glutPostRedisplay();
    } else if(key == 'X' || key == 'x'){
        exit(0);
    } else if(key == 'G' || key == 'g'){
        compute_scene(-SCENE_WIDTH/2,-SCENE_HEIGHT/2,
                        SCENE_WIDTH/2,SCENE_HEIGHT/2);
        show_message = false;
        glutPostRedisplay();
    }
    
}

/* Add a sphere to the head of the list */
void add_sphere(double x, double y, double z, double r, color c, 
        double reflectivity, double spec_exp){
    sphere_list * newNode = (sphere_list *) malloc(sizeof(sphere_list));
    newNode->s.center.x = x;
    newNode->s.center.y = y;
    newNode->s.center.z = z;
    newNode->s.radius = r;
    newNode->ambient = c;
    newNode->diffuse = c;
    newNode->specular = c;
    newNode->s_exp = spec_exp;
    newNode->reflectivity = reflectivity;
    newNode->next = list;
    list = newNode;
    
}

#define canvas_Width SCENE_WIDTH
#define canvas_Height SCENE_HEIGHT
#define canvas_Name "Programming Assignment 5 - Paul Warnes"


int main(int argc, char ** argv){
    int y, x;
    /*setup some colors*/
    color green = {GREEN};
    color silver = {.4,.4,.4, 1.0};
    color white = {.95,.95,.95, 1.0};
    color orange = {ORANGE};
    color red = {RED};
    color yellow = {YELLOW};
    color blue = {BLUE};
    glutInit(&argc, argv);
    
    glEnable(GL_AUTO_NORMAL);
    glShadeModel(GL_SMOOTH);
    
    /*create the light*/
    light0.location.x = 0;
    light0.location.y = 0;
    light0.location.z = 10;

    light0.ambient.r = .12;
    light0.ambient.g = .12;
    light0.ambient.b = .12;
    
    light0.diffuse.r = .32;
    light0.diffuse.g = .32;
    light0.diffuse.b = .32;
    
    light0.specular.r = .4;
    light0.specular.g = .4;
    light0.specular.b = .4;
    
    add_sphere(0,0,-20,6, silver, 0.7, 9);
    add_sphere(15,15,-20,7, white, 0.5, 1.4);
    add_sphere(78,52,-70,10, green, 0.5, 1.2);
    add_sphere(48,51,-68,10, red, 0.5, 1.1);
    add_sphere(50,50,-40,4, yellow, 0.5, 1.2);
    
    add_sphere(-9,11,-11,10, orange, 0.5, 1.2);
    
    add_sphere(3,11,-11,2, red, 0.5, 1.2);
    add_sphere(-50,0,-50,25, yellow, 0.5, 1.2);
    add_sphere(45,5,20,18, blue, 0.5, 1.2);
    
    my_setup(CANVAS_WIDTH + SPLINE_WIDTH, CANVAS_HEIGHT, canvas_Name);
    
    glutKeyboardFunc(keyboard_input);
    
    glutDisplayFunc(display_func);
    
    /*setup the light for splines*/
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D((GLdouble) -(CANVAS_WIDTH + SPLINE_WIDTH)/2, 
               (GLdouble) (CANVAS_WIDTH + SPLINE_WIDTH)/2, 
               (GLdouble) -CANVAS_HEIGHT/2, (GLdouble) CANVAS_HEIGHT/2);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    light0.location.y = SCENE_HEIGHT/2;
    light0.location.x = CANVAS_WIDTH/2 + SPLINE_WIDTH/2;
    init_light(GL_LIGHT0, light0);
    light0.location.x += 20;
    init_light(GL_LIGHT1, light0);
    glEnable(GL_LIGHT0);
    light0.location.x = 0;
    light0.location.y = 0;
    
    glutMainLoop();
    
    return 0;
}

