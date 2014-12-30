
#include <stdlib.h>
#include <vector>
#include "Object.h"
class Scene
{
    std::vector<Object*> objects;
public:
    void addObject(Object* object) {
        objects.push_back(object);
    }
    ~Scene() {
        for(unsigned int i=0; i<objects.size(); i++)
            delete objects.at(i); 
    }
    void draw() {
        for(unsigned int i=0; i<objects.size(); i++)
            objects.at(i)->draw();
    }
    void move(double t, double dt) {
        for(unsigned int i=0; i<objects.size(); i++)
            objects.at(i)->move(t, dt);
    }
};