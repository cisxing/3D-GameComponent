#include "float2.h"
class Object
{
protected:
    float2 scaleFactor;
    float2 position;
    float orientation;
public:
    Object():orientation(0.0f), scaleFactor(1.0,1.0){}
    virtual ~Object(){}
    virtual void draw()
    {
        // apply scaling, translation and orientation
        drawModel();
    }
    virtual void drawModel()=0;
    virtual void move(double t, double dt){}
};
