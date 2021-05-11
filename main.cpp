#include <iostream>
#include "parser.h"
#include <GL/glew.h>
#include <GL/gl.h>   // The GL Header File

#include <GLFW/glfw3.h>
#include <GL/freeglut.h> // The GL Utility Toolkit (Glut) Header


//////-------- Global Variables -------/////////

GLuint gpuVertexBuffer;
GLuint gpuNormalBuffer;
GLuint gpuIndexBuffer;

// Sample usage for reading an XML scene file
parser::Scene scene;
static GLFWwindow* win = NULL;

double previousTime;
int frameCount;

int gWidth, gHeight;


std::vector<parser::Vec3f> vertexNormals;

parser::Mesh mesh;
parser::Material material;



static void errorCallback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}



parser::Vec3f crossProduct(parser::Vec3f v1 , parser::Vec3f v2) {
    parser::Vec3f result;
    result.x = (v1.y * v2.z - v1.z * v2.y);
    result.y = (v1.z * v2.x - v1.x * v2.z);
    result.z = (v1.x * v2.y - v1.y * v2.x);

    return result;
}

parser::Vec3f add(parser::Vec3f v1, parser::Vec3f v2) {

    parser::Vec3f result;
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;

    return result;
    //result.x = (x - v.x);//, y - v.y, z - v.z);
}

parser::Vec3f minus(parser::Vec3f v1, parser::Vec3f v2 ) {

    parser::Vec3f result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;

    return result;
    //result.x = (x - v.x);//, y - v.y, z - v.z);
}

parser::Vec3f normalize(parser::Vec3f v) {

    float norm = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);

    parser::Vec3f result;

    result.x = (v.x / norm);
    result.y = (v.y / norm);
    result.z = (v.z / norm);

    return result;
}

parser::Vec3f vertexNormal(parser::Vec3f v1, parser::Vec3f v2, parser::Vec3f v3) {
      
    parser::Vec3f result;
    result = (crossProduct(minus(v2,v1), minus(v3,v1)));
    //result.y = (v1.z * v2.x - v1.x * v2.z);
    //result.z = (v1.x * v2.y - v1.y * v2.x);

    return result;
}



std::vector<parser::Vec3f> ComputeVertexNormals() {

    int v0, v1, v2;

    std::vector<parser::Vec3f> vertexNormals(scene.vertex_data.size());

    for (size_t i = 0; i < scene.meshes.size(); i++)
    {
        for (size_t j = 0; j < scene.meshes[i].faces.size(); j++)
        {

            v0 = scene.meshes[i].faces[j].v0_id - 1;
            v1 = scene.meshes[i].faces[j].v1_id - 1;;
            v2 = scene.meshes[i].faces[j].v2_id - 1;;


            vertexNormals[v0] = (add((vertexNormal(scene.vertex_data[v0], scene.vertex_data[v1], scene.vertex_data[v2])), vertexNormals[v0]));

            vertexNormals[v1] = (add((vertexNormal(scene.vertex_data[v1], scene.vertex_data[v2], scene.vertex_data[v0])), vertexNormals[v1]));

            vertexNormals[v2] = (add((vertexNormal(scene.vertex_data[v2], scene.vertex_data[v0], scene.vertex_data[v1])), vertexNormals[v2]));


        }
    }

    for (size_t i = 0; i < vertexNormals.size(); i++)
    {
        vertexNormals[i] = normalize(vertexNormals[i]);
    }
    

    return vertexNormals;
}





void drawMesh(int i) { // i -> mesh index

    int v0,v1,v2;
    parser::Vec3f n0, n1, n2;

    mesh = scene.meshes[i];
    material = scene.materials[mesh.material_id - 1];

    GLfloat ambColor[4] = { material.ambient.x, material.ambient.y, material.ambient.z, 1.0 };
    GLfloat diffColor[4] = { material.diffuse.x, material.diffuse.y, material.diffuse.z, 1.0 };
    GLfloat specColor[4] = { material.specular.x, material.specular.y, material.specular.z, 1.0 };
    GLfloat specExp[1] = { material.phong_exponent };

    //fprintf(stdout, "Diffuse: %f %f %f %f\n", diffColor[0], diffColor[1], diffColor[2], diffColor[3]);

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambColor);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffColor);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specColor);
    glMaterialfv(GL_FRONT, GL_SHININESS, specExp);



    for (size_t j = 0; j < mesh.faces.size(); j++)
    {
        v0 = mesh.faces[j].v0_id - 1;
        v1 = mesh.faces[j].v1_id - 1;;
        v2 = mesh.faces[j].v2_id - 1;;

        //n0 = normalize(divide(vertexNormals[v0], vertexCounts[v0]));
        n0 = ((vertexNormals[v0]));
        n1 = ((vertexNormals[v1]));
        n2 = ((vertexNormals[v2]));

        glBegin(GL_TRIANGLES);

        //glColor3f(colors[0], colors[1], colors[2]);
        glNormal3f(n0.x, n0.y, n0.z);
        glVertex3f(scene.vertex_data[v0].x, scene.vertex_data[v0].y, scene.vertex_data[v0].z);

        glNormal3f(n1.x, n1.y, n1.z);
        glVertex3f(scene.vertex_data[v1].x, scene.vertex_data[v1].y, scene.vertex_data[v1].z);

        glNormal3f(n2.x, n2.y, n2.z);
        glVertex3f(scene.vertex_data[v2].x, scene.vertex_data[v2].y, scene.vertex_data[v2].z);
        glEnd();
    }


}

void Render()
{

    static float rotationAngle = 0;
    static float translation = 0;

    //glClearColor(1.0, 1.0, 1.0, 1.0);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glClearColor(0.20, 0.15, 0.25, 1); // purple
    glClearColor(scene.background_color.x, scene.background_color.y, scene.background_color.z, 1); // background color

    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Anchient method to draw primitives;
    // The order of vertices are also important!!!

    //glLoadIdentity();
    //
    //glTranslatef(0.00, 0.00, -translation);
    //glRotatef(rotationAngle,0,1,0);
    //rotationAngle += 0.002;
    //translation += 0.04;

    //fprintf(stdout, "Number of Faces: %d\n", scene.meshes[1].faces.size());
    
    

    for (size_t i = 0; i < scene.meshes.size(); i++)
    {
        mesh = scene.meshes[i];

        if (mesh.mesh_type.compare("Solid") == 0)
        {
            //fprintf(stdout, "Mesh Type SOLID!\n");

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        }
        else if (mesh.mesh_type.compare("Wireframe") == 0)
        {
            //fprintf(stdout, "Mesh Type Wireframe!\n");

            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        }
        else {
            fprintf(stdout, "Mesh Type Error!");
        }
        

        glPushMatrix();
        
        for (int j = mesh.transformations.size()-1; j >= 0 ; j--)
        {
            if (mesh.transformations[j].transformation_type.compare("Translation") == 0)
            {
                float tX = scene.translations[mesh.transformations[j].id - 1].x;
                float tY = scene.translations[mesh.transformations[j].id - 1].y;
                float tZ = scene.translations[mesh.transformations[j].id - 1].z;

                //fprintf(stdout, "TRANSLATION: %f %f %f \n", tX, tY, tZ);
                //glLoadIdentity();

                glTranslatef(tX, tY, tZ);
                //glLoadIdentity();
                //glTranslatef(-tX, -tY, -tZ);

            }
            else if (mesh.transformations[j].transformation_type.compare("Rotation") == 0)
            {
                float rA = scene.rotations[mesh.transformations[j].id - 1].x;
                float rX = scene.rotations[mesh.transformations[j].id - 1].y;
                float rY = scene.rotations[mesh.transformations[j].id - 1].z;
                float rZ = scene.rotations[mesh.transformations[j].id - 1].w;

                //fprintf(stdout, "ROTATION: %f %f %f %f \n" , rA, rX, rY, rZ);
                //glLoadIdentity();

                glRotatef(rA ,rX, rY, rZ);
                //glLoadIdentity();
                //glTranslatef(-tX, -tY, -tZ);
            }else if (mesh.transformations[j].transformation_type.compare("Scaling") == 0)
            {
                float sX = scene.scalings[mesh.transformations[j].id - 1].x;
                float sY = scene.scalings[mesh.transformations[j].id - 1].y;
                float sZ = scene.scalings[mesh.transformations[j].id - 1].z;

                //fprintf(stdout, "SCALING: %f %f %f \n", sX, sY, sZ);
                //glLoadIdentity();

                glScalef(sX, sY, sZ);
                //glLoadIdentity();
                //glTranslatef(-tX, -tY, -tZ);
            }
            else {
                fprintf(stdout, "TRANSFORMATION ERROR\n");

            }
        }

        drawMesh(i); // i-> mesh index
        glPopMatrix();

    }



}

void turnOnLights() {

    glEnable(GL_LIGHTING);

    glEnable(GL_LIGHT0);

    GLfloat ambient[] = { scene.ambient_light.x, scene.ambient_light.y, scene.ambient_light.z, 1.0f };
    GLfloat diffuse[] = { 0,0,0 ,1.0f};
    GLfloat specular[] = { 0,0,0 ,1.0f};
    //GLfloat positions[] = { 0,0,0 ,1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    //glLightfv(GL_LIGHT0, GL_POSITION, positions);



    for (int i = 0; i < scene.point_lights.size(); i++) {

        glEnable(GL_LIGHT0 + i+1);
        GLfloat intensities[] = { scene.point_lights[i].intensity.x, scene.point_lights[i].intensity.y, scene.point_lights[i].intensity.z, 1.0f };
        GLfloat positions[] = { scene.point_lights[i].position.x, scene.point_lights[i].position.y, scene.point_lights[i].position.z, 1.0f };
        GLfloat ambient[] = { 0,0,0,1.0f };

        glLightfv(GL_LIGHT0 + i+1, GL_POSITION, positions);
        glLightfv(GL_LIGHT0 + i+1, GL_AMBIENT, ambient);
        glLightfv(GL_LIGHT0 + i+1, GL_DIFFUSE, intensities);
        glLightfv(GL_LIGHT0 + i+1, GL_SPECULAR, intensities);
    }


}


void init() {
    turnOnLights();

    glEnable(GL_DEPTH_TEST);

    if (scene.culling_enabled)
    {
        glEnable(GL_CULL_FACE);
        if (scene.culling_face == 0)
        {
            glCullFace(GL_BACK);
        }
        else {
            glCullFace(GL_FRONT);
        }
    }

    glShadeModel(GL_SMOOTH);


    float fovy = (scene.camera.near_plane.w) / scene.camera.near_distance; 
    
    fovy = 2 * (double)atan(fovy) * (180 / 3.1415);

    float aspectRatio = (float) scene.camera.image_width / (float) scene.camera.image_height;

    glViewport(0, 0, scene.camera.image_width, scene.camera.image_height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective( fovy, aspectRatio , scene.camera.near_distance,scene.camera.far_distance);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //glTranslatef( - scene.camera.position.x , - scene.camera.position.y , - scene.camera.position.z );
    float camX = scene.camera.position.x;
    float camY = scene.camera.position.y;
    float camZ = scene.camera.position.z;

    float gazeX = scene.camera.gaze.x;
    float gazeY = scene.camera.gaze.y;
    float gazeZ = scene.camera.gaze.z;

    float upX = scene.camera.up.x;
    float upY = scene.camera.up.y;
    float upZ = scene.camera.up.z;

    gluLookAt(0,0,0, gazeX, gazeY, gazeZ, upX, upY, upZ); //first set the up vector and look at the point the gaze vector is pointing at 
    glTranslatef(-camX,-camY,-camZ); // then move the scene according to the camera position

    //gluLookAt(camX,camY,camZ, gazeX, gazeY, gazeZ, upX, upY, upZ);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


}

//void reshape(int w, int h)   // Create The Reshape Function (the viewport)
//{
//    w = w < 1 ? 1 : w;
//    h = h < 1 ? 1 : h;
//
//    gWidth = w;
//    gHeight = h;
//
//    glViewport(0, 0, w, h);
//
//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    //glOrtho(-10, 10, -10, 10, -10, 10);
//    gluPerspective(45, (float)w / h, 1, 500);
//
//    glMatrixMode(GL_MODELVIEW);
//    glLoadIdentity();
//
//
//}



void window_size_callback(GLFWwindow* window, int width, int height)
{
    //fprintf(stdout, "RESIZE CALLBACK");
    //int width, height;
    glfwGetWindowSize(window, &width, &height);


}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void UpdateTitle(GLFWwindow* win)
{
    // Measure speed
    double currentTime = glfwGetTime();
    double deltaTime = currentTime - previousTime;
    char ss[500] = {};
    frameCount++;
    if (deltaTime >= 1.0) { // If last cout was more than 1 sec ago
        //cout << 1000.0/double(frameCount) << endl;

        double fps = ((double)(frameCount)) / deltaTime;

        sprintf(ss, "CENG477 - HW3 [%3.2lf FPS]", fps);

        glfwSetWindowTitle(win, ss);

        frameCount = 0;
        previousTime = currentTime;
    }
}

int main(int argc, char* argv[]) {
    //fprintf(stdout, "Start of Loading\n");

    scene.loadFromXml(argv[1]);

    //fprintf(stdout, "End of Loading\n");


    glfwSetErrorCallback(errorCallback);
    
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    
    win = glfwCreateWindow(scene.camera.image_width, scene.camera.image_height, "CENG477 - HW3", NULL, NULL);
    if (!win) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(win);
    
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    
    glfwSetKeyCallback(win, keyCallback);
    
    vertexNormals = ComputeVertexNormals();
    
    init();
    
    //glutInit(&argc, argv);
    ////
    //glutReshapeFunc(reshape);

    glfwSetWindowSizeCallback(win, window_size_callback); // initialized after context
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
    
    previousTime = glfwGetTime();
    frameCount = 0;

    while(!glfwWindowShouldClose(win)) {
        //glfwWaitEvents();

        UpdateTitle(win);

        Render();

        glfwSwapBuffers(win);
        glfwPollEvents();
    }
    
    glfwDestroyWindow(win);
    glfwTerminate();
    
    exit(EXIT_SUCCESS);
    
    return 0;
}

