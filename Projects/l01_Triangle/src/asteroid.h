#include "Object.h"


class Asteroid : public Object
{
    void drawModel(){
		glColor3d(1.0, 0.5, 0.0);
        glBegin(GL_TRIANGLE_FAN);
             glVertex2d(0.0, 0.0);
             for(double phi=0.0; phi<6.5; phi+=0.32)
               glVertex2d( 
                   cos(phi)*(0.8 + 0.2*sin(phi*7) ),
                   sin(phi)*(0.8 + 0.2*sin(phi*7) ));
        glEnd();
	}
};
