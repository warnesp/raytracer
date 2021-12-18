
#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <math.h>
#include <stdbool.h>

/*structure to hold a point (vertex)*/
typedef struct point_str {
    double x, y, z;
} point;

/*a representation of a vector*/
typedef struct vector_str {
    double x, y, z;
} vector;

/* a means to switch between point and vector views*/
union point_vector {
    point p;
    vector v;
};

typedef struct ray_str {
    point orgin, at;
} ray;

typedef struct sphere_str {
    point center;
    double radius;
} sphere;

point scale_point(double scale, point p){
    p.x *= scale;
    p.y *= scale;
    p.z *= scale;
    return p;
}

point add_points(point one, point two){
    point result;
    result.x = one.x + two.x;
    result.y = one.y + two.y;
    result.z = one.z + two.z;
    return result;
}

point subtract_points(point one, point two){
    point result;
    result.x = one.x - two.x;
    result.y = one.y - two.y;
    result.z = one.z - two.z;
    return result;
}

vector subtract_vectors(vector one, vector two){
    vector result;
    result.x = one.x - two.x;
    result.y = one.y - two.y;
    result.z = one.z - two.z;
    return result;
}

/*get a vector from two points*/
vector points_to_vector(point one, point two){
    union point_vector pv;
    pv.p = subtract_points(two, one);
    
    return  pv.v;
}

/*creates a ray from a point and a vector*/
ray point_vector_to_ray(point p, vector v){
    ray result;
    result.orgin = p;
    result.at.x = p.x + v.x; 
    result.at.y = p.y + v.y; 
    result.at.z = p.z + v.z;
    return result;
}
/*add two vectors together*/
vector add_vectors(vector one, vector two){
    vector result;
    result.x = one.x + two.x;
    result.y = one.y + two.y;
    result.z = one.z + two.z;
    return result;
}
/*scales the vector by the given amount*/
vector scale_vector(double scale, vector v){
    v.x *= scale;
    v.y *= scale;
    v.z *= scale;
    return v;
}
/*normalizes the given vector*/
vector normalize_vector(vector v){
    double length = sqrt(v.x * v.x + v.y*v.y + v.z*v.z);
    if(length == 0){
        return v;
    }
    v.x /= length;
    v.y /= length;
    v.z /= length;
    return v;
}

/*dot product of two vectors*/
double dot_vector(vector one, vector two){
    return one.x*two.x + one.y * two.y + one.z* two.z;
}

/*returns the distance squared between two points*/
double distance_sq(point one, point two){
    double dx = (two.x - one.x), dy = (two.y - one.y), dz = (two.z - one.z);
    return dx*dx + dy*dy + dz*dz;
}

/*finds a point along a ray*/
point parametric_ray(ray r, double t){
    double dx = (r.at.x - r.orgin.x), 
            dy = (r.at.y - r.orgin.y), 
            dz = (r.at.z - r.orgin.z);
    point p;
    p.x = r.orgin.x + t * (dx);
    p.y = r.orgin.y + t * (dy);
    p.z = r.orgin.z + t * (dz);
    return p;
}

/*normalizes the ray*/
ray normalize_ray(ray r){
    ray result;
    double length = sqrt((r.at.x - r.orgin.x) * (r.at.x - r.orgin.x) 
        + (r.at.y - r.orgin.y) * (r.at.y - r.orgin.y) 
        + (r.at.z - r.orgin.z) * (r.at.z - r.orgin.z) );
    result.orgin = r.orgin;
    result.at.x = (r.at.x - r.orgin.x) / length + r.orgin.x;
    result.at.y = (r.at.y - r.orgin.y) / length + r.orgin.y;
    result.at.z = (r.at.z - r.orgin.z) / length + r.orgin.z;
    
    return result;
    
}

/* get a vector for the given ray*/
vector ray_to_vector(ray r){
    vector result = {r.at.x - r.orgin.x, r.at.y - r.orgin.y, 
                        r.at.z - r.orgin.z};
                        
    return result;
}

/*finds the intersection between a sphere s and a ray r.
 * s - the sphere
 * r - the ray
 * found - used to return if the point was found 
 * return - the point found */
point find_intersection(sphere s, ray r, bool * found){
    float t0, t1;
    point p;
    ray unit = normalize_ray(r);
    double sqrt_disc, radius_sq = s.radius * s.radius;
    double dx = r.at.x - r.orgin.x, 
            dy = r.at.y - r.orgin.y, 
            dz = r.at.z - r.orgin.z;
    double dxs = r.orgin.x - s.center.x,
            dys = r.orgin.y - s.center.y,
            dzs = r.orgin.z - s.center.z;
    double dist_sq = distance_sq(s.center, r.orgin);
    if(dist_sq < radius_sq ){
        //no intercetion, ray inside sphere
        *found = false;
        return p;
    }
    
    //Calculating the coefficients of the quadratic equation
    double a = dx*dx + dy*dy + dz*dz;
    double b = 2.0f * ( dxs * dx  + dys * dy + dzs * dz); 
    double c = dxs * dxs + dys * dys + dzs * dzs 
                    - radius_sq;
    
    double disc = (b*b)-(4.0f*a*c);
    
    if(disc <= 0.00001){
        //no intersection
        *found = false;
    } else {
        sqrt_disc = sqrt(disc);
        t0 = (-b - sqrt_disc) / (2 * a);
        t1 = (-b + sqrt_disc) / (2 * a);
        
        if(t0 < 0.00001 && t1 < 0.00001){
            *found == false;
        } else {
             if(t0 <= 0.00001){
                //intersect at t1, t0 is behind or at start
                p = parametric_ray(r, t1);
            } else {
                //intersect at t0
                p = parametric_ray(r, t0 < t1 ? t0 : t1);
            }
            
            *found = true;
        }
    }
    
    return p;
}

/*finds the intersection of a ray and the y plane at the given y coordinate,
 * assumes there is such an intersection*/
point find_y_plane_intersection(ray r, double plane_y){
    double t = (plane_y - r.orgin.y) /(r.at.y - r.orgin.y);
    return parametric_ray(r, t);
}

#endif

