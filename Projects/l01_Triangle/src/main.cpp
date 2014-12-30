/**notes from 11/24
Very similar to the ones we did in assignment 3. Scene is a collection of objects. introduce draw function in Objects. Using opengl-quad to do the param surface
glBegin(GL_QUADS);
for(int i = 0; i<NTESS;i++)
	for (int j = 0; j<NTESS;j++){
	VertexOGL((float)i/NTESS, (float)j/NTESS);
	then just do either i+1 or j+1... for the rest of the four vertices
	VertexOGL((float)(i+1)/NTESS, (float)j/NTESS);
	VertexOGL((float)i/NTESS, (float)(j+1)/NTESS);
	}

	Using glutSolidSphere(R.rows,cols) for the sphere drawModel()
		  glutSolidTeapot(size) for drawModel in Teapot

	World to screen:

	camera transormation: u dot u = 1 because u is a unit vector, u dot v = 0 because they are perpendicular to each other



	Dec. 1:
	Review from last lecture: we need to keep fp as big as possible and bp as small as possible
	PPT18:
		for each pixel,t = infinity
		//trace a ray
		for eah object: t_new = intersect(pixel, object)
		if(tnew<t){+=tnew, objectcolor}

		t[] = infinite
		what if we swap them?
		for each object:
			for each pixel
				t_new = intersect(object, pixel)
				if t_new <t[pixel]{t[pixel]= t_new, object color} just swap the color
				//We replace t with z to figure out the z buffer
		The bigger z value is, the farther the layer is from the eye, so if two objects intersects, choose the one that has the smaller z value

		PPT20: have an equation of Z(X,Y) = aX+bY+c and we have three vertices, so we can solve a,b and c.
		Linear regression already takes too much time... for several nano seconds So we use incremental method: scanning each line-> Z(X+1,Y) = Z(X,Y)+a
		Ineteger addition would be so much faster than floating point addition

		PPT21: The same for shading, the method we used in computing shades is too slow:
				Alernatively, we use per-vertex shading 

		PPT22: Gouraud shading
				Use intersect method to compute the normal transform. Opengl takes care of it
				Viewing direction will be the negative direction of position vector
				Use linear interpolation to color with R,G,B components. Using the same incremental method

		PPT23: Opengl can help, but parameters are needed, need to enable glEnable(GL_LIGHTING) (If we do not enable the glEnable, then glColor is forgortten, 
				is the color we use to color the vertices in glVertex)
				Color = ... where Id means diffuse intensity
				*backface can never be visible, but it still needs color so when we turn the object we can see it
				glMaterialfv where fv means floating value, vector

				position with 0 at the end means it is infinitely far
				with normal position, we just set the fourth element to 1 to transform from the cartesian coordinates to homogeneous coordinates

		PPT24: Texture mapping
				



		Practical: physics of the game
**/

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
// Needed on MsWindows
#include <windows.h>
#endif // Win32 platform
 
#include <GL/gl.h>
#include <GL/glu.h>
// Download glut from: http://www.opengl.org/resources/libraries/glut/
#include <GL/glut.h>
#include "float2.h"
#include "float3.h"
#include <vector>
#include <map>
#include "Mesh.h"
#include "stb_image.c"
extern "C" unsigned char* stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp);



class LightSource
{
public:
	virtual float3 getpowerDensityAt  ( float3 x )=0;
	virtual float3 getLightDirAt  ( float3 x )=0;
	virtual float  getDistanceFrom( float3 x )=0;
	virtual void   apply( GLenum openglLightName )=0;
};

class DirectionalLight : public LightSource
{
	float3 dir;
	float3 powerDensity;
public:
	DirectionalLight(float3 dir, float3 powerDensity)
		:dir(dir), powerDensity(powerDensity){}
	float3 getpowerDensityAt  ( float3 x ){return powerDensity;}
	float3 getLightDirAt  ( float3 x ){return dir;}
	float  getDistanceFrom( float3 x ){return 900000000;}
	void   apply( GLenum openglLightName )
	{
		float aglPos[] = {dir.x, dir.y, dir.z, 0.0f};
        glLightfv(openglLightName, GL_POSITION, aglPos);
		float aglZero[] = {0.0f, 0.0f, 0.0f, 0.0f};
        glLightfv(openglLightName, GL_AMBIENT, aglZero);
		float aglIntensity[] = {powerDensity.x, powerDensity.y, powerDensity.z, 1.0f};
        glLightfv(openglLightName, GL_DIFFUSE, aglIntensity);
        glLightfv(openglLightName, GL_SPECULAR, aglIntensity);
        glLightf(openglLightName, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(openglLightName, GL_LINEAR_ATTENUATION, 0.0f);
        glLightf(openglLightName, GL_QUADRATIC_ATTENUATION, 0.0f);                
	}
};

class PointLight : public LightSource
{
	float3 pos;
	float3 power;
public:
	PointLight(float3 pos, float3 power)
		:pos(pos), power(power){}
	float3 getpowerDensityAt  ( float3 x ){return power*(1/(x-pos).norm2()*4*3.14);}
	float3 getLightDirAt  ( float3 x ){return (pos-x).normalize();}
	float  getDistanceFrom( float3 x ){return (pos-x).norm();}
	void   apply( GLenum openglLightName )
	{
		float aglPos[] = {pos.x, pos.y, pos.z, 1.0f};
        glLightfv(openglLightName, GL_POSITION, aglPos);
		float aglZero[] = {0.0f, 0.0f, 0.0f, 0.0f};
        glLightfv(openglLightName, GL_AMBIENT, aglZero);
		float aglIntensity[] = {power.x, power.y, power.z, 1.0f};
        glLightfv(openglLightName, GL_DIFFUSE, aglIntensity);
        glLightfv(openglLightName, GL_SPECULAR, aglIntensity);
        glLightf(openglLightName, GL_CONSTANT_ATTENUATION, 0.0f);
        glLightf(openglLightName, GL_LINEAR_ATTENUATION, 0.0f);
        glLightf(openglLightName, GL_QUADRATIC_ATTENUATION, 0.25f / 3.14f);
	}
};

class Material
{
public:
	float3 kd;			// diffuse reflection coefficient
	float3 ks;			// specular reflection coefficient
	float shininess;	// specular exponent
	Material()
	{
		kd = float3(0.5, 0.5, 0.5) + kd * 0.5;
		ks = float3(1, 1, 1);
		shininess = 15;
	}
	virtual void apply()
	{
		float aglDiffuse[] = {kd.x, kd.y, kd.z, 1.0f};
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, aglDiffuse);
		float aglSpecular[] = {kd.x, kd.y, kd.z, 1.0f};
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, aglSpecular);
		if(shininess <= 128)
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
		else
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 128.0f);
		glDisable(GL_TEXTURE_2D);
	}
};

// Skeletal Camera class. Feel free to add custom initialization, set aspect ratio to fit viewport dimensions, or animation.
class Camera
{
public:
	float3 eye;

	float3 ahead;
	float3 right;
	float3 up;

	float2 lastMousePos;
    float2 mouseDelta;

	float fov;
	float aspect;
	float3 lookAt;
public:
	void setAspectRatio(float ar)  { 
		aspect = ar;
	}

	float3 getEye()
	{
		return eye;
	}
	Camera()
	{
		eye = float3(0, 1, 50);
		ahead = float3(0, 0, -1);
		right = float3(1, 0, 0);
		up = float3(0, 1, 0);
		fov = 1.5;
		aspect = 1;
	}

	void apply()
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(fov / 3.14 * 180, aspect, 0.1, 200);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		//this two are the exactly the same thing.Define lookAt for no reason:(
		gluLookAt(eye.x, eye.y, eye.z, eye.x+ahead.x, eye.y+ahead.y, eye.z+ahead.z, 0.0, 1.0, 0.0);
		//gluLookAt(eye.x,eye.y, eye.z, lookAt.x, lookAt.y,lookAt.z,0.0,1.0,0.0);
	}

	void startDrag(int x, int y) {
        lastMousePos = float2(x, y);
    }
    void drag(int x, int y){
        float2 mousePos(x, y);
        mouseDelta = mousePos - lastMousePos;
        lastMousePos = mousePos;
    }
    void endDrag(){
        mouseDelta = float2(0, 0);
    }
	void move(float dt, std::vector<bool>& keysPressed) {
		if(keysPressed.at('w'))
			eye += ahead * dt * 20;
		if(keysPressed.at('s'))
			eye -= ahead * dt * 20;
		if(keysPressed.at('a'))
			eye -= right * dt * 20;
		if(keysPressed.at('d'))
			eye += right * dt * 20;
		if(keysPressed.at('q'))
			eye -= float3(0,1,0) * dt * 20;
		if(keysPressed.at('e'))
			eye += float3(0,1,0) * dt * 20;
  //to be continued here
		float yaw = atan2f( ahead.x, ahead.z );
	    float pitch = -atan2f( ahead.y, 
		sqrtf(ahead.x * ahead.x + ahead.z * ahead.z) );

		 yaw -= mouseDelta.x * 0.02f;
		pitch += mouseDelta.y * 0.02f;
		if(pitch > 3.14/2) pitch = 3.14/2;
		if(pitch < -3.14/2) pitch = -3.14/2;

   	    mouseDelta = float2(0, 0);

		ahead = float3(sin(yaw)*cos(pitch), -sin(pitch),
                                 cos(yaw)*cos(pitch) );
		right = ahead.cross(float3(0, 1, 0)).normalize();
		up = right.cross(ahead);
		lookAt = eye + ahead;

}


};


// Object abstract base class.
class Object
{
public:
	Material* material;
	float3 scaleFactor;
	float3 position;
	float3 orientationAxis;
	float orientationAngle;
public:
	Object(Material* material):material(material),position(0, 0, 0),orientationAngle(0.0f),scaleFactor(1.0,1.0,1.0),orientationAxis(0.0,1.0,0.0){}
    virtual ~Object(){}
    Object* translate(float3 offset){
        position += offset; return this;
    }
    Object* scale(float3 factor){
        scaleFactor *= factor; return this;
    }
    Object* rotate(float angle){
        orientationAngle += angle; return this;
    }
    virtual void draw()
    {
		material->apply();
        // apply scaling, translation and orientation
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
			glTranslatef(position.x, position.y, position.z);
			glRotatef(orientationAngle, orientationAxis.x, orientationAxis.y, orientationAxis.z);
			glScalef(scaleFactor.x, scaleFactor.y, scaleFactor.z);
			drawModel();
		glPopMatrix();
    }
    virtual void drawModel()=0;
    virtual void move(double t, double dt){}
    virtual bool control(std::vector<bool>& keysPressed, std::vector<Object*>& spawn, std::vector<Object*>& objects){return false;}
};


class Teapot : public Object
{
public:
	Teapot(Material* material):Object(material){}
	void drawModel()
	{
		glutSolidTeapot(1.0f);
	}
};

class Spawn: public Object
{
public:
	float3 dir;
public:

	Spawn(Material* material, float3 direction):Object(material){
		dir = direction.normalize();
		direction.x = direction.x*10;
		direction.z = direction.z*10;
	}
	void drawModel(){

		glutSolidSphere(1,50,50);
			
		//glutSolidTeapot(1.0f);
	}
    void move(double t, double dt){
		dir*= pow(0.93, dt);
		if(position.y>2)
		{
			translate(dir+float3(0,-0.8,0));
		}
		
	}
    bool control(std::vector<bool>& keysPressed, std::vector<Object*>& spawn, std::vector<Object*>& objects){return false;}


};

class MeshInstance: public Object
{
	Mesh* mesh1;
public:
	MeshInstance(Mesh* mesh, Material* material):Object(material){
		mesh1 = mesh;
	}
	void drawModel()
	{
		mesh1->draw();
	}

};

//global variable


class TexturedMaterial: public Material{
	GLuint textureName;
	//GLuint *textures = new GLuint[n];

public:
	TexturedMaterial(const char* filename, 
	GLint filtering = GL_LINEAR_MIPMAP_LINEAR
){
unsigned char* data;
int width;
int height;
int nComponents = 4;

data = stbi_load(filename, &width, &height, &nComponents, 0);

if(data == NULL) return;

// opengl texture creation comes here
glGenTextures(1, &textureName);  // id generation
glBindTexture(GL_TEXTURE_2D, textureName);      // binding



if(nComponents == 4)
  gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
else if(nComponents == 3)
  gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);


delete data;
}


	void apply(){
		Material::apply();
		glBindTexture(GL_TEXTURE_2D, textureName); 
		glEnable(GL_BLEND);
		
        glEnable(GL_LINEAR_MIPMAP_LINEAR);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_TEXTURE_2D);
    }


};


class RotatingPart: public MeshInstance
{
public:
	float angularVelocity;
	float angularAccel;
	RotatingPart(Mesh* mesh, Material* material):MeshInstance(mesh, material)
	{
		angularVelocity = 40;
		angularAccel = 0;
		orientationAxis = float3(1.0,0.0,0.0);
	}

	void move(double t, double dt)
	{
		rotate(angularVelocity*dt);
	}

	bool control(std::vector<bool>& keysPressed, std::vector<Object*>& spawn, std::vector<Object*>& objects)
	{
		if(keysPressed.at('p')){
			angularAccel+=1;
		}
		if(keysPressed.at('o'))
		{
			angularAccel-=1;
		}
		if(!keysPressed.at('p')&&!keysPressed.at('o'))
		{
			angularAccel = 0;
		}
		return true;
	}

};

class Bouncer: public MeshInstance
{
public:

	float3 velocity;
	float angularVelocity; // rotation only around Y
	float angularAcceleration;
	float restitution;  //ratio of energy conserved on bounce
	float3 accel;
	float3 direction;
	int i;
	Bouncer(Mesh* mesh, Material* material):MeshInstance(mesh, material)
	{
		i = 0;
		accel= float3(0,-10, 0);
		restitution = 1;
		velocity = float3(0,15,0);
		angularVelocity= 0;
		angularAcceleration = 0;
		direction = float3(0,0,1);
		orientationAngle = 90;
	}
	//MeshInstance(Mesh* mesh, Material* material):Object(material)

	void move(double t, double dt)
	{
	
		velocity = velocity+accel*dt;
		position = position + velocity*dt;

		if(position.y < 0) 
		{
			velocity.y *= -restitution;
			position.y= 0;
		}
			
		angularVelocity = angularVelocity+ angularAcceleration*dt;
		//this is the drag
		angularVelocity *= pow(0.8, dt);
		velocity *= pow(0.9, dt);
		rotate(angularVelocity*dt);
		direction.x = direction.x*cos((angularVelocity*dt/180)*3.14159)+direction.z*sin((angularVelocity*dt/180)*3.14159);
		direction.z = -sin((angularVelocity*dt/180)*3.14159)*direction.x+direction.z*cos((angularVelocity*dt/180)*3.14159);

	}

	bool control(std::vector<bool>& keysPressed, std::vector<Object*>& spawn, std::vector<Object*>& objects)
	{
		if(keysPressed.at('h')){
			angularAcceleration+=1;
		}
		if(keysPressed.at('k'))
		{
			angularAcceleration-=1;
		}
		if(!keysPressed.at('h')&&!keysPressed.at('k'))
		{
			angularAcceleration = 0;
		}
		if(keysPressed.at('u'))
		{
			accel = float3(-cos(orientationAngle/180*3.14159)*10, -10, sin(orientationAngle/180*3.14159) *10);
		}
		if(keysPressed.at('j'))
		{
			accel = float3(cos(orientationAngle/180*3.14159) *10, -10, -sin(orientationAngle/180*3.14159) *10);
		}
		if(!keysPressed.at('u')&&!keysPressed.at('j'))
		{
			accel = float3(0,-10,0);
		}
		if(keysPressed.at('d'))
		{
			restitution = 1.5;
		}
		else
		{
			restitution = 1;
		}
		return true;
	}
};

class Ground: public Object{
public:	
	Ground(Material* material):Object(material){
	}
	void drawModel()
	{	
		/**glBegin(GL_QUADS);
		glVertex3d(1250,0,1250);
		glVertex3d(12500,0,-1250);
		glVertex3d(-1250,0,-1250);
		glVertex3d(-1250,0,1250);
		glEnd();**/

		glEnable(GL_TEXTURE_2D);

		glBegin(GL_QUADS);
				glTexCoord2d( 1, 1);
				glVertex3d(1250, 0, 1250);
				
				glTexCoord2d( 1,0);
				glVertex3d(1250, 0, -1250);
				
				glTexCoord2d( 0, 0);
				glVertex3d(-1250, 0, -1250);
				
				glTexCoord2d( 0, 1);
				glVertex3d(-1250, 0, 1250);
		glEnd();
		
		glDisable(GL_TEXTURE_2D);

	}


};

class Scene
{
public:
	Camera camera;
	std::vector<LightSource*> lightSources;
	std::vector<Object*> objects;
	std::vector<Material*> materials;
	std::vector<Mesh*> meshs;
	std::vector<Object*> spawns;
	Bouncer* avatar;
public:
	

	Scene()
	{

		// BUILD YOUR SCENE HERE
		lightSources.push_back(
			new DirectionalLight(
				float3(0, 1, 0), 
				float3(1, 1, 1)));
		lightSources.push_back(
			new DirectionalLight(
				float3(0, 0, 1), 
				float3(1, 1, 1)));
		lightSources.push_back(
			new PointLight(
				float3(-1, -1, 1), 
				float3(1, 1, 1)));
		lightSources.push_back(
			new PointLight(
				float3(5, 5, 5), 
				float3(1, 1, 1)));
	
		Material* yellowDiffuseMaterial = new Material();
		materials.push_back(yellowDiffuseMaterial);
		yellowDiffuseMaterial->kd = float3(1, 1, 1);
		materials.push_back(new Material());
		
		materials.push_back(new Material());
		materials.push_back(new Material());
		materials.push_back(new Material());
		materials.push_back(new Material());
		materials.push_back(new Material());

		//objects.push_back( (new Teapot( yellowDiffuseMaterial))->translate(float3(2, 0, 0)) );
		objects.push_back( (new Teapot( materials.at(2) )     )->translate(float3(4, 3, 2))->scale(float3(0.6, 0.6, 0.6)) );
		objects.push_back( (new Teapot( materials.at(2) )     )->translate(float3(0, 3, 0.5))->scale(float3(1.3, 1.3, 1.3)) );
		objects.push_back( (new Teapot( materials.at(1) )     )->translate(float3(-4, 3, 2))->scale(float3(0.6, 0.6, 0.6)) );


		
	}
	~Scene()
	{
		for (std::vector<LightSource*>::iterator iLightSource = lightSources.begin(); iLightSource != lightSources.end(); ++iLightSource)
			delete *iLightSource;
		for (std::vector<Material*>::iterator iMaterial = materials.begin(); iMaterial != materials.end(); ++iMaterial)
			delete *iMaterial;
		for (std::vector<Object*>::iterator iObject = objects.begin(); iObject != objects.end(); ++iObject)
			delete *iObject;
	}

public:
	
	void initialize(){

		//THE UPPER ONE IS A GLOBAL PATH AND THE LOWER ONE IS A RELATIVE PATH. THEY BOTH WORK FOR ME
		//meshs.push_back(new Mesh("C:\\Users\\xinyunxing\\Desktop\\AitGraphics-Hello\\Projects\\l01_Triangle\\src\\tigger.obj"));
		meshs.push_back(new Mesh("tigger.obj"));
		avatar = new Bouncer(meshs[0],new TexturedMaterial("tigger.png"));
		objects.push_back(avatar);
		//objects.push_back(new MeshInstance(meshs[0],materials[1]));
		
		//objects.push_back(new Ground(materials[1]));
		objects.push_back(new Ground(new TexturedMaterial("Syrian_Desert.png")));

		meshs.push_back(new Mesh("tree.obj"));
		objects.push_back((new MeshInstance(meshs[1],new TexturedMaterial("tree.png")))->translate(float3(-10,0,30)));

		meshs.push_back(new Mesh("wheel.obj"));
		//Bouncer* wheel1= (new Bouncer(meshs[2],new TexturedMaterial("C:\\Users\\xinyunxing\\Desktop\\AitGraphics-Hello\\Projects\\l01_Triangle\\src\\chevy.png")))->translate(float3(45,5,44))->scale(float3(1.2,1.2,1.2));
		//objects.push_back(wheel1);

		objects.push_back((new RotatingPart(meshs[2],new TexturedMaterial("chevy.png")))->translate(float3(45,5,44))->scale(float3(1.2,1.2,1.2)));
		objects.push_back((new RotatingPart(meshs[2],new TexturedMaterial("chevy.png")))->translate(float3(56,5,44))->scale(float3(1.2,1.2,1.2)));
		objects.push_back((new RotatingPart(meshs[2],new TexturedMaterial("chevy.png")))->translate(float3(45,5,19))->scale(float3(1.2,1.2,1.2)));
		objects.push_back((new RotatingPart(meshs[2],new TexturedMaterial("chevy.png")))->translate(float3(56,5,19))->scale(float3(1.2,1.2,1.2)));

		meshs.push_back(new Mesh("chassis.obj"));
		objects.push_back((new MeshInstance(meshs[3],new TexturedMaterial("chevy.png")))->translate(float3(50,10,30)));

		for (int i=0; i< 5; i++)
		{
			objects.push_back((new MeshInstance(meshs[1],new TexturedMaterial("tree.png")))->translate(float3(-10+80*i,0,30+80*i)));
			objects.push_back((new MeshInstance(meshs[1],new TexturedMaterial("tree.png")))->translate(float3(-10-80*i,0,30-80*i)));
			objects.push_back((new MeshInstance(meshs[1],new TexturedMaterial("tree.png")))->translate(float3(-10-80*i,0,30+80*i)));
			objects.push_back((new MeshInstance(meshs[1],new TexturedMaterial("tree.png")))->translate(float3(-10+80*i,0,30-80*i)));
			
		}
		
	}
	Camera& getCamera()
	{
		return camera;
	}

	Bouncer* getAvatar()
	{
		return avatar;
	}
	void draw()
	{
		camera.apply();
		unsigned int iLightSource=0;
		for (; iLightSource<lightSources.size(); iLightSource++)
		{
			glEnable(GL_LIGHT0 + iLightSource);
			lightSources.at(iLightSource)->apply(GL_LIGHT0 + iLightSource);
		}
		for (; iLightSource<GL_MAX_LIGHTS; iLightSource++)
			glDisable(GL_LIGHT0 + iLightSource);

		for (unsigned int iObject=0; iObject<objects.size(); iObject++)
			objects.at(iObject)->draw();
		
		for (unsigned int iSpawn=0; iSpawn<spawns.size(); iSpawn++)
			spawns.at(iSpawn)->draw();

	}

	void move(double t, double dt)
	{
		for (unsigned int iObject=0; iObject<objects.size(); iObject++)
			objects.at(iObject)->move(t,dt);
	
		for (unsigned int iSpawn=0; iSpawn<spawns.size(); iSpawn++)
			spawns.at(iSpawn)->move(t,dt);

	}


	void control(std::vector<bool>& keysPressed)
	{
		std::vector<Object*> spawn;
		for (unsigned int iObject=0; 
                 iObject<objects.size(); iObject++)
		{
			objects.at(iObject)->control(
             keysPressed, spawn, objects);
		}

 
	//camera.eye = avatar->position - avatar->direction.normalize()*20+float3(0,30,0);
	//camera.ahead = avatar->direction;
		float3 angle(-cos(avatar->orientationAngle/180*3.14),0,sin(avatar->orientationAngle/180*3.14));
		camera.eye = avatar->position- float3(-cos(avatar->orientationAngle/180*3.14),0,sin(avatar->orientationAngle/180*3.14))*20+float3(0,30,0);
		camera.ahead = float3(-cos(avatar->orientationAngle/180*3.14),0,sin(avatar->orientationAngle/180*3.14));
	
	if(keysPressed.at('m'))
	{
		spawns.push_back((new Spawn(materials[1], angle+float3(0,6,0)))->translate(avatar->position+float3(0,30,0))->scale(float3(3,3,3)));
		//spawns.push_back( (new Teapot( materials.at(1) )     )->scale(float3(20, 20, 20)) );
		
	}
	if(keysPressed.at('n'))
	{
		if(spawns.size()>0)
		{
			spawns.erase((spawns.begin()));
		}
	}
}

	std::vector<Object*>& getObjects()
	{
		return objects;
	}

};

// global application data

// screen resolution
const int screenWidth = 600;
const int screenHeight = 600;

//scene object
Scene scene;

std::vector<bool> keysPressed;

void onKeyboard(unsigned char key, int x, int y) {
    keysPressed.at(key) = true;
}

void onKeyboardUp(unsigned char key, int x, int y) {
    keysPressed.at(key) = false;
}

void onMouse(int button, int state, int x, int y) {
  if(button == GLUT_LEFT_BUTTON)
    if(state == GLUT_DOWN)
      scene.getCamera().startDrag(x, y);
    else
      scene.getCamera().endDrag();
}

void onMouseMotion(int x, int y) {
  scene.getCamera().drag(x, y);
}

// Displays the image. 
void onDisplay( ) {
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear screen

	scene.draw();

    glutSwapBuffers(); // drawing finished
}

void onIdle()
{
	double t = glutGet(GLUT_ELAPSED_TIME) * 0.001;
    static double lastTime = 0.0;
    double dt = t - lastTime;
    lastTime = t;
	scene.control(keysPressed);
	scene.move(t,dt);
    //scene.getCamera().move(dt, keysPressed);
	
	//scene.getCamera(). = 15*scene.getAvatar()->orientationAngle;
	//scene.getCamera().ahead = float3(-cos(scene.getAvatar()->orientationAngle) *15, 40, sin(scene.getAvatar()->orientationAngle) *15);
    glutPostRedisplay();

}

void onReshape(int winWidth, int winHeight) {
  glViewport(0, 0, winWidth, winHeight);
  scene.getCamera().setAspectRatio(
                    (float)winWidth/winHeight);
}


int main(int argc, char **argv) {
	//scene.getCamera()
    glutInit(&argc, argv);						// initialize GLUT
	
    glutInitWindowSize(screenWidth, screenHeight);				// startup window size 
    glutInitWindowPosition(100, 100);           // where to put window on screen
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);    // 8 bit R,G,B,A + double buffer + depth buffer
	
    glutCreateWindow("OpenGL teapots");				// application window is created and displayed
 
    glViewport(0, 0, screenWidth, screenHeight);

    glutDisplayFunc(onDisplay);					// register callback
    glutIdleFunc(onIdle);						// register callback
	glutKeyboardFunc(onKeyboard);
    glutKeyboardUpFunc(onKeyboardUp);
    for(int i=0; i<256; i++)
        keysPressed.push_back(false);
	
	glutMouseFunc(onMouse);
    glutMotionFunc(onMouseMotion);
	glutReshapeFunc(onReshape);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	scene.initialize();
    glutMainLoop();								// launch event handling loop
    
    return 0;
}
