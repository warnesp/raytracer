/*************************************************************

    This file has initialization function calls and set-ups
for CS 445/545 Open GL programs that use the GLUT.  (The initializations
will work for Mesa as well as Open GL.)  The initializations involve
defining a callback handler (my_reshape_function) that sets vewing
parameters for 2D display. The x and y coordinates of the drawing canvas
upper corner are determined by the STRT_X_POS and STRT_Y_POS variables, resp.

    TSN 01/2010

***************************************************************/
/* reshape callback handler - defines viewing parameters */

int my_width;
int my_height;

void my_reshape_handler(int width, int height)
    {
        my_width = width;
        my_height = height;
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
    
        //setting viewing parameters to simple 2D viewing
        glLoadIdentity();
        gluOrtho2D((GLdouble) -width/2, (GLdouble) width/2, (GLdouble) -height/2, (GLdouble) height/2);
        
        glMatrixMode(GL_MODELVIEW);
    }
    
#define STRT_X_POS 25
#define STRT_Y_POS 25

void my_setup(int width, int height, char *window_name_str)
{
    //initialization
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glPolygonMode(GL_FRONT, GL_FILL);
    
    glutInitWindowSize(width, height);
    glutInitWindowPosition(STRT_X_POS, STRT_Y_POS);
    
    glutCreateWindow(window_name_str);
    
    //one callback handler - viewing parameters set upone resize event
    glutReshapeFunc(my_reshape_handler);
}

