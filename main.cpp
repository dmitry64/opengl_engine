#include "tiny_obj_loader.cpp"
#include "tiny_obj_loader.h"
#include <GL/glew.h>
#include <GL/glut.h>
#include <SOIL/SOIL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

struct custom_mesh {
    tinyobj::mesh_t model;
    std::vector<float> diffusecolors;
    std::vector<float> ambientcolors;
    std::vector<float> specularcolors;
    GLuint vertexbuffer;
    GLuint uvbuffer;
    GLuint normalbuffer;
    GLuint indexbuffer;
    GLuint ambientbuffer;
    GLuint diffusebuffer;
    GLuint specularbuffer;
};

struct custom_texture {
    GLuint id;
    unsigned char *image_ptr;
    std::string name;
};

std::vector<custom_texture> textures;
std::vector<tinyobj::shape_t> shapes;
std::vector<tinyobj::material_t> materials;
std::vector<custom_mesh> meshes;

float angle;
GLuint texture[1];

GLuint shaderProgram;
GLchar *vertexSource, *fragmentSource;
GLuint vertexShader, fragmentShader;

char *filetobuf(char *file) {
    FILE *fptr;
    long length;
    char *buf;
    fptr = fopen(file, "rb");
    if (!fptr)
        return NULL;
    fseek(fptr, 0, SEEK_END);
    length = ftell(fptr);
    buf = new char[length + 1];
    fseek(fptr, 0, SEEK_SET);
    fread(buf, length, 1, fptr);
    fclose(fptr);
    buf[length] = 0;
    return buf;
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
}

GLuint load_texture(const char *c) {
    GLuint temp = SOIL_load_OGL_texture(
            c,
            SOIL_LOAD_AUTO,
            SOIL_CREATE_NEW_ID,
            SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
    );

    if (0 == temp) {
        printf("SOIL loading error: '%s'\n", SOIL_last_result());
    }

    glBindTexture(GL_TEXTURE_2D, temp);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return temp;
}

int getTextureId(std::string name) {
    std::vector<custom_texture>::iterator it = textures.begin();
    while (it != textures.end()) {
        if (it.operator*().name.compare(name) == 0)
            return it.operator*().id;
        it++;
    }
    std::cout << "Texture " << name << " not found!" << std::endl;
    return -1;
}

void display(void) {
    while (true) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        float aspectRatio = (float) 800 / 600, fieldOfView = 60.0f;
        glm::mat4 ProjectionMatrix = glm::perspective(fieldOfView, aspectRatio, 1.0f, 1000.0f);
        glm::mat4 ViewMatrix = glm::lookAt(glm::vec3(15.0f * sin(angle / 8), 5.0f, 15.0f * cos(angle / 8)),
                                           glm::vec3(0.0f, 2.3f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 ModelMatrix = glm::mat4();
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

        GLuint MatrixID = glGetUniformLocation(shaderProgram, "MVP");
        GLuint ViewMatrixID = glGetUniformLocation(shaderProgram, "V");
        GLuint ModelMatrixID = glGetUniformLocation(shaderProgram, "M");
        GLuint LightID = glGetUniformLocation(shaderProgram, "LightPosition_worldspace");
        GLuint hasTextureID = glGetUniformLocation(shaderProgram, "hasTexture");

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

        glm::vec3 lightPos = glm::vec3(20, 19, 17);
        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

        glBegin(GL_POLYGON);
        glNormal3i(0, 1, 0);
        glVertex3i(-40, 0, 40);
        glVertex3i(40, 0, 40);
        glVertex3i(40, 0, -40);
        glVertex3i(-40, 0, -40);
        glEnd();

        std::vector<custom_mesh>::iterator it = meshes.begin();
        GLint vbpos = glGetAttribLocation(shaderProgram, "vertexPosition_modelspace");
        GLint normpos = glGetAttribLocation(shaderProgram, "vertexNormal_modelspace");
        GLint uvpos = glGetAttribLocation(shaderProgram, "vertexUV");
        GLint ambientpos = glGetAttribLocation(shaderProgram, "vertexAmbientColor");
        GLint diffusepos = glGetAttribLocation(shaderProgram, "vertexDiffuseColor");
        GLint specularpos = glGetAttribLocation(shaderProgram, "vertexSpecularColor");

        glEnableVertexAttribArray(normpos);
        glEnableVertexAttribArray(vbpos);
        glEnableVertexAttribArray(uvpos);
        glEnableVertexAttribArray(ambientpos);
        glEnableVertexAttribArray(diffusepos);
        glEnableVertexAttribArray(specularpos);

        while (it != meshes.end()) {
            glBindBuffer(GL_ARRAY_BUFFER, it.operator*().vertexbuffer);
            glVertexAttribPointer(vbpos, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
            glBindBuffer(GL_ARRAY_BUFFER, it.operator*().normalbuffer);
            glVertexAttribPointer(normpos, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);

            glBindBuffer(GL_ARRAY_BUFFER, it.operator*().uvbuffer);
            glVertexAttribPointer(uvpos, 2, GL_FLOAT, GL_FALSE, 0, (void *) 0);

            glBindBuffer(GL_ARRAY_BUFFER, it.operator*().diffusebuffer);
            glVertexAttribPointer(diffusepos, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
            glBindBuffer(GL_ARRAY_BUFFER, it.operator*().ambientbuffer);
            glVertexAttribPointer(ambientpos, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
            glBindBuffer(GL_ARRAY_BUFFER, it.operator*().specularbuffer);
            glVertexAttribPointer(specularpos, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, it.operator*().indexbuffer);

            int material = it.operator*().model.material_ids.front();
            std::string text_name = materials.at(material).diffuse_texname;
            glUniform1i(hasTextureID, 0);
            if (text_name.compare("") != 0) {
                GLuint textureid = getTextureId(text_name);
                GLuint texturepos = glGetUniformLocation(shaderProgram, "texture_sample");
                if (textureid != -1) {
                    glActiveTexture(GL_TEXTURE0);
                    glUniform1i(texturepos, 0);
                    glBindTexture(GL_TEXTURE_2D, textureid);
                    glUniform1i(hasTextureID, 1);
                }
            }
            glDrawElements(GL_TRIANGLES, it.operator*().model.indices.size(), GL_UNSIGNED_INT, nullptr);
            it++;
        }

        glDisableVertexAttribArray(specularpos);
        glDisableVertexAttribArray(diffusepos);
        glDisableVertexAttribArray(ambientpos);
        glDisableVertexAttribArray(uvpos);
        glDisableVertexAttribArray(vbpos);
        glDisableVertexAttribArray(normpos);
        glutSwapBuffers();
        angle += 0.01f;
    }
}

int main(int argc, char **argv) {
    std::cout << "Loading model..." << std::endl;

    std::string errstr;
    tinyobj::LoadObj(shapes, materials, errstr, "car.obj", "./", true);

    std::cout << "Model loaded." << std::endl;
    std::cout << "Meshes count = " << shapes.size() << std::endl;
    std::cout << "Materials count = " << shapes.front().mesh.material_ids.size() << std::endl;

    std::vector<tinyobj::shape_t>::iterator iter = shapes.begin();
    while (iter != shapes.end()) {
        std::cout << "new shape: " << iter.operator*().name << std::endl;
        std::cout << "indices: " << iter.operator*().mesh.indices.size() << std::endl;
        std::cout << "verts: " << iter.operator*().mesh.positions.size() << std::endl;
        std::cout << "materials: " << iter.operator*().mesh.material_ids.size() << std::endl;
        std::cout << std::endl << "====================================" << std::endl;
        iter++;
    }

    angle = 0;

    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInit(&argc, argv);

    glutCreateWindow("OpenGL sample");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    glClearColor(0.1f, 0.1f, 0.1f, 1.f);

    glewExperimental = GL_TRUE;
    GLenum error = glewInit();
    if (error != GLEW_OK) {
        std::cout << "glewInit failed, aborting." << std::endl;
    }
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    std::cout << "v: " << glGetString(GL_VERSION) << std::endl;

    vertexSource = filetobuf("vertex.vert");
    fragmentSource = filetobuf("fragment.frag");
    std::cout << "Shaders read!" << std::endl;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    std::cout << "Shaders created!" << std::endl;
    glShaderSource(vertexShader, 1, (const GLchar **) &vertexSource, 0);
    glShaderSource(fragmentShader, 1, (const GLchar **) &fragmentSource, 0);

    free(vertexSource);
    free(fragmentSource);

    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);

    GLint success = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    std::cout << "Vertex shader compile status: " << success << std::endl;
    success = 0;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    std::cout << "Fragment shader compile status: " << success << std::endl;

    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    std::cout << "Program link status: " << success << std::endl;

    glUseProgram(shaderProgram);

    std::cout << "Shaders loaded!" << std::endl;

    std::vector<tinyobj::shape_t>::iterator it = shapes.begin();
    while (it != shapes.end()) {
        custom_mesh mm;
        mm.model = it.operator*().mesh;

        GLuint vertexbuffer;
        glGenBuffers(1, &vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, mm.model.positions.size() * sizeof(GL_FLOAT), mm.model.positions.data(),
                     GL_STATIC_DRAW);

        GLuint normalbuffer;
        glGenBuffers(1, &normalbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
        glBufferData(GL_ARRAY_BUFFER, mm.model.normals.size() * sizeof(GL_FLOAT), mm.model.normals.data(),
                     GL_STATIC_DRAW);

        GLuint uvbuffer;
        glGenBuffers(1, &uvbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glBufferData(GL_ARRAY_BUFFER, mm.model.texcoords.size() * sizeof(GL_FLOAT), mm.model.texcoords.data(),
                     GL_STATIC_DRAW);


        std::vector<int>::iterator it2 = it.operator*().mesh.material_ids.begin();
        while (it2 != it.operator*().mesh.material_ids.end()) {
            int id = it2.operator*();
            float *diffuse = materials.at(id).diffuse;
            float *ambient = materials.at(id).ambient;
            float *specular = materials.at(id).specular;
            if (materials.at(id).diffuse_texname.compare("") != 0) {
                std::string texture_name = materials.at(id).diffuse_texname;
                if (getTextureId(texture_name) == -1) {
                    std::cout << "New texture: " << texture_name << std::endl;

                    custom_texture mytexture1;
                    mytexture1.name = texture_name;
                    mytexture1.id = load_texture(texture_name.c_str());

                    textures.push_back(mytexture1);
                }
            }

            mm.diffusecolors.push_back(diffuse[0]);
            mm.diffusecolors.push_back(diffuse[1]);
            mm.diffusecolors.push_back(diffuse[2]);
            mm.diffusecolors.push_back(diffuse[0]);
            mm.diffusecolors.push_back(diffuse[1]);
            mm.diffusecolors.push_back(diffuse[2]);
            mm.diffusecolors.push_back(diffuse[0]);
            mm.diffusecolors.push_back(diffuse[1]);
            mm.diffusecolors.push_back(diffuse[2]);

            mm.ambientcolors.push_back(ambient[0]);
            mm.ambientcolors.push_back(ambient[1]);
            mm.ambientcolors.push_back(ambient[2]);
            mm.ambientcolors.push_back(ambient[0]);
            mm.ambientcolors.push_back(ambient[1]);
            mm.ambientcolors.push_back(ambient[2]);
            mm.ambientcolors.push_back(ambient[0]);
            mm.ambientcolors.push_back(ambient[1]);
            mm.ambientcolors.push_back(ambient[2]);

            mm.specularcolors.push_back(specular[0]);
            mm.specularcolors.push_back(specular[1]);
            mm.specularcolors.push_back(specular[2]);
            mm.specularcolors.push_back(specular[0]);
            mm.specularcolors.push_back(specular[1]);
            mm.specularcolors.push_back(specular[2]);
            mm.specularcolors.push_back(specular[0]);
            mm.specularcolors.push_back(specular[1]);
            mm.specularcolors.push_back(specular[2]);

            it2++;
        }

        GLuint diffusebuffer;
        glGenBuffers(1, &diffusebuffer);
        glBindBuffer(GL_ARRAY_BUFFER, diffusebuffer);
        glBufferData(GL_ARRAY_BUFFER, mm.diffusecolors.size() * sizeof(GL_FLOAT), mm.diffusecolors.data(),
                     GL_STATIC_DRAW);

        GLuint ambientbuffer;
        glGenBuffers(1, &ambientbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, ambientbuffer);
        glBufferData(GL_ARRAY_BUFFER, mm.ambientcolors.size() * sizeof(GL_FLOAT), mm.ambientcolors.data(),
                     GL_STATIC_DRAW);

        GLuint specularbuffer;
        glGenBuffers(1, &specularbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, specularbuffer);
        glBufferData(GL_ARRAY_BUFFER, mm.specularcolors.size() * sizeof(GL_FLOAT), mm.specularcolors.data(),
                     GL_STATIC_DRAW);

        GLuint indexbuffer;
        glGenBuffers(1, &indexbuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mm.model.indices.size() * sizeof(unsigned int), mm.model.indices.data(),
                     GL_STATIC_DRAW);

        mm.vertexbuffer = vertexbuffer;
        mm.indexbuffer = indexbuffer;
        mm.normalbuffer = normalbuffer;
        mm.uvbuffer = uvbuffer;
        mm.diffusebuffer = diffusebuffer;
        mm.ambientbuffer = ambientbuffer;
        mm.specularbuffer = specularbuffer;

        meshes.push_back(mm);

        it++;
    }
    std::cout << "Meshes loaded!" << std::endl;

    glutMainLoop();

    std::cout << "End." << std::endl;

    return 0;
}
