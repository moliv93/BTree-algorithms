#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb-master/stb_image.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shaderReader.hpp"
#include "textureLoader.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const float SCR_WIDTH = 800;
const float SCR_HEIGHT = 600;

#define M 5

bool debugReportsActive = true;

class Page;

struct Vector2i {
    int x = 0;
    int y = 0;

    // Vector2i(float _x, float _y) { x = _x; y = _y; }
};

struct SplitData {
    bool splitted;
    int midKey;
    Page* newPage;
};

class Page {
public:
    int t = M;
    int n = 0;
    int keys[M];
    Page* links[M+1] = {};
    bool isLeaf = true;
    bool isRoot = false;

    Vector2i pos;

    void updatePos(Vector2i ref) {
        pos.x = ref.x;
        pos.y = ref.y;

        for (int i=0; i<=n; i++) {
            // printf("DRAW LINKS\n");
            if (links[i] != nullptr) {
                Vector2i newPos;
                newPos.x = pos.x;
                newPos.y = pos.y;
                newPos.y -= 100;
                newPos.x = i*t*50 + 50;
                links[i]->updatePos(newPos);
            }
        }
    }

    Vector2i getPos() {
        return pos;
    }

    void countLeaves(int& count) {
        if (isLeaf) {
            count += n+1;
            return;
        }
        else {
            for (int i=0; i<=n; i++) {
                if (links[i] != nullptr) {
                    links[i]->countLeaves(count);
                }
            }
        }
    }

    void alignLeavesPosition(int& leafIndex, int anchorPosition) {
        if (isLeaf) {
            pos.x = anchorPosition + leafIndex;
            leafIndex += (n+1)*50;
            return;
        }
        else {
            for (int i=0; i<=n; i++) {
                links[i]->alignLeavesPosition(leafIndex, anchorPosition);
            }
        }
    }

    Vector2i getLeftmostPosition() {
        if (isLeaf) {
            return pos;
        }
        else {
            return links[0]->getLeftmostPosition();
        }
    }


    Vector2i getRightmostPosition() {
        if (isLeaf) {
            Vector2i rightPos = pos;
            // rightPos.x += 50*5;
            return rightPos;
        }
        else {
            for (int i=n; i>=0; i--) {
                if (links[i] != nullptr) {
                    return links[i]->getRightmostPosition();
                }
            }
        }
        return pos;
    }

    void alignParents(int anchorPosition) {
        if (isLeaf) {
            return;
        }
        Vector2i left = getLeftmostPosition();
        Vector2i right = getRightmostPosition();
        if (isRoot) {
            // printf("left leaf pos: %d\n", left.x);
            // printf("right leaf pos: %d\n", right.x);
        }
        pos.x = left.x + ((right.x - left.x)/2);
        if (isRoot) {
            // printf("pos: %d\n", pos.x);
        }

        for (int i=0; i<=n; i++) {
            links[i]->alignParents(anchorPosition);
        }
    }

    void updateHeight(int base) {
        pos.y = base;
        for (int i=0; i<=n; i++) {
            if (links[i] != nullptr) {
                links[i]->updateHeight(base-100);
            }
        }
    }

    void updatePositions(glm::mat4& projection) {
        // printf("start update\n");
        int leavesCellCount = 0;
        countLeaves(leavesCellCount);
        // printf("leaves counted\n");
        int totalCellLength = leavesCellCount * 50;
        int leafIndex = 0;
        alignLeavesPosition(leafIndex, -totalCellLength/2);
        // printf("leaves aligned\n");
        alignParents(-totalCellLength/2);
        // printf("parents aligned\n");
        updateHeight(0);
        // printf("height updated\n");
        // std::vector<std::pair<int, int>> leafs;
    }

    void draw(unsigned int tfmLoc) {
        glm::vec2 transform(pos.x, pos.y);
        for (int i=0; i<n; i++) {
            glUniform2fv(tfmLoc, 1, glm::value_ptr(transform));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            transform.x += 50;
        }

        if (!isLeaf) {
            for (int i=0; i<=n; i++) {
                links[i]->draw(tfmLoc);
            }
        }

    }

    void writeKeyNumbers(uint transformLoc, uint cropLoc, int wid, int hei, uint* fontIndices) {
        glm::vec2 transform(pos.x+4.0f, pos.y + 20);
        for (int i=0; i<n; i++) {
            std::string key = std::to_string(keys[i]);
            glm::vec2 fontTransform = transform;
            for (int i=0; i<key.length(); i++) {
                char cpos = key.c_str()[i];
                // glm::vec4 crop( (1.0f/110.0f)*(0), (1.0f/110.0f)*0, (1.0f/110.0f), (1.0f/110.0f));
                // printf("cpos: %d\n", cpos);
                glm::vec4 crop( (1.0f/16.0f)*(cpos%16), (1.0f/8.0f)*(cpos/16), (1.0f/16.0f), (1.0f/8.0f));
                glUniform2fv(transformLoc, 1, glm::value_ptr(fontTransform));
                glUniform4fv(cropLoc, 1, glm::value_ptr(crop));
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
                fontTransform.x += 10;
            }
            // glUniform2fv(tfmLoc, 1, glm::value_ptr(transform));
            // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            transform.x += 50;
        }

        if (!isLeaf) {
            for (int i=0; i<=n; i++) {
                links[i]->writeKeyNumbers(transformLoc, cropLoc, wid, hei, fontIndices);
            }
        }
    }

    void drawLinks(uint shaderProgram) {
        if (isLeaf) {
            return;
        }
        uint pointsLoc = glGetUniformLocation(shaderProgram, "linePoints");
        for (int i=0; i<=n; i++) {
            // glm::vec3 points[] = {glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 100.0f, 0.0f), glm::vec3(100.0f, 100.0f, 0.0f)};
            float points[6] = {(float)(pos.x+(50*i)), (float)pos.y, 0, (float)links[i]->pos.x, (float)(links[i]->pos.y+50), 0.0f};
            glUniform3fv(pointsLoc, 2, points);
            glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, nullptr);

            links[i]->drawLinks(shaderProgram);
        }
    }

    Page() {

    }

    void report() {
        if (debugReportsActive) {
            printf("call from %s\n", isRoot ? "root" : "child");
            printf("call from %s leaf node\n", isLeaf ? "" : "non");
            printf("%d nodes in this page\n", n);
            printf("keys: ");
            if (n) {
                for (int i=0; i<n; i++) { printf("%d ", keys[i]);}
            }
            printf("\n");
        }
    }

    SplitData insert(int _key) {
        SplitData spd;
        report();
        if (!isLeaf) {
            printf("key passing non-leaf node\n");
            int i=0;
            while (keys[i] < _key && i < n) { i++; } // search point of intermediate value
            printf("inserting on node %d below\n", i);
            spd = links[i]->insert(_key);
            if (spd.splitted) {
                spd.splitted = false; // stop split flag backing up, this can be changed by split() call below
                // keys[n] = spd.midKey; // change to smart push

                int i=0;
                while (i < n && keys[i] < spd.midKey) { i++; }
                if (i < n)
                    links[n+1] = links[n];
                    for (int j=n; j>i; j--) {
                        keys[j] = keys[j-1];
                        links[j] = links[j-1];
                    }
                keys[i] = spd.midKey;
                links[i+1] = spd.newPage;
                n++;

                if (n == t) {
                    if (!isRoot) {
                        spd = split();
                    }
                    else {
                        rootSplit();
                    }
                }
                return spd;
            }
        }
        else {
            spd.splitted = false;
            printf("key is at leaf node\n");
            int i=0;
            while (i < n && keys[i] < _key) { i++; }
            // make room for new element
            if (i < n)
                for (int j=n; j>i; j--) {
                    printf("fowarding\n");
                    keys[j] = keys[j-1];
                }
            printf("Inserting %d at pos %d\n", _key, i);
            keys[i] = _key;
            // insert new element
            n++;
            if (n == t) {
                if (isRoot) {
                    rootSplit();
                }
                else {
                    spd = split();
                    // return spd;
                }
            }
            if (n > t) {
                std::cout << "PAGE BUFFER OVERFLOW!!!!!\n";
            }
            return spd;
        }
        return spd;
    }

    SplitData split() {
        printf("Preparing split\n");
        Page* secHalf = new Page(); // create new page
        SplitData spd; // create return structure
        spd.splitted = true; // confirm split
        spd.newPage = secHalf; // associate new page return
        int halfPos = n/2; // find mid position
        spd.midKey = keys[halfPos]; // associate return value of key
        printf("secHalf start\n");
        secHalf->report();
        // only mantains mid key if this page is leaf, or else, midkey is already at some leaf
        int copyPos = halfPos;
        if (!isLeaf) { copyPos = halfPos + 1; }
        for (int i=copyPos; i<n; i++) { // copy post-half keys to new page
            printf("copying %d\n", keys[i]);
            secHalf->insert(keys[i]);
        }
        for (int i=copyPos; i<=n; i++) {
            secHalf->links[i-copyPos] = links[i];
        }
        n = halfPos;// sets n to new value
        if (secHalf->links[0] != nullptr) {
            secHalf->isLeaf = false;
        }
        printf("Split done\n");
        return spd;
    }

    void rootSplit() {
        printf("Preparing root split\n");
        // self_draw(1);
        Page* preHalf = new Page();
        Page* postHalf = new Page();
        int halfPos = n/2;

        for (int i=0; i<halfPos; i++) {
            preHalf->keys[i] = keys[i];
            preHalf->links[i] = links[i];
        }
        preHalf->links[halfPos] = links[halfPos];
        preHalf->n = halfPos;

        // only copies mid element IF childs are leafs, else, mid element is already present at leaf level
        int copyPos = halfPos;
        if (links[0] != nullptr) { copyPos = halfPos + 1; }
        for (int i=copyPos; i<=n; i++) {
            postHalf->keys[i-(copyPos)] = keys[i];
            postHalf->links[i-(copyPos)] = links[i];
        }
        postHalf->links[n-copyPos] = links[n];
        postHalf->n = n-copyPos;

        keys[0] = keys[halfPos];
        links[0] = preHalf;
        links[1] = postHalf;
        n = 1;
        // preHalf->isRoot = false;
        // postHalf->isRoot = false
        isLeaf = false;
        if (preHalf->links[0] != nullptr) {preHalf->isLeaf = false;}
        if (postHalf->links[0] != nullptr) {postHalf->isLeaf = false;}
        printf("Root split done\n");
    }

    void self_draw(int level) {
        printf("%d%s: [", level, isLeaf ? "yL" : "nL");
        printf("%c", links[0] == nullptr ? '-' : '+');
        for (int i=0; i<n; i++) {
            printf("%d%c ", keys[i], links[i+1] == nullptr ? '-' : '+');
        }
        printf("]\n");
        for (int i=0; i<=n; i++) {
            if (links[i] != nullptr) {
                links[i]->self_draw(level+1);
            }
        }
    }
};

int main(int argc, char *argv[]) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glm::mat4 projection;
    // projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, 0.0f, 100.0f);
    // projection = glm::ortho(-800.0f/2, 800.0f/2, -600.0f/2, 600.0f/2, 0.0f, 100.0f);
    projection = glm::ortho(-SCR_WIDTH/2.0f, SCR_HEIGHT/2.0f, -SCR_HEIGHT/2.0f, SCR_HEIGHT/2.0f, 0.0f, 100.0f);
    // projection = glm::ortho(-2.0f, +2.0f, -1.5f, +1.5f, 0.1f, 100.0f);
    // projection = glm::ortho(-100.0f/2, 100.0f/2, -100.0f/2, 100.0f/2, 0.0f, 100.0f);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    uint textureVS = createShader("shaders/texture.vert", GL_VERTEX_SHADER);
    uint textureFS = createShader("shaders/texture.frag", GL_FRAGMENT_SHADER);
    unsigned int textureSP = glCreateProgram();
    glAttachShader(textureSP, textureVS);
    glAttachShader(textureSP, textureFS);
    glLinkProgram(textureSP);
    glDeleteShader(textureVS);
    glDeleteShader(textureFS);

    uint textVS = createShader("shaders/text.vert", GL_VERTEX_SHADER);
    uint textFS = createShader("shaders/text.frag", GL_FRAGMENT_SHADER);
    unsigned int textSP = glCreateProgram();
    glAttachShader(textSP, textVS);
    glAttachShader(textSP, textFS);
    glLinkProgram(textSP);
    glDeleteShader(textVS);
    glDeleteShader(textFS);

    uint lineVS = createShader("shaders/lines.vert", GL_VERTEX_SHADER);
    uint lineFS = createShader("shaders/lines.frag", GL_FRAGMENT_SHADER);
    unsigned int lineSP = glCreateProgram();
    glAttachShader(lineSP, lineVS);
    glAttachShader(lineSP, lineFS);
    glLinkProgram(lineSP);
    glDeleteShader(lineVS);
    glDeleteShader(lineFS);

    float vertices[] = {
        // positions          // colors           // texture coords
         0.0f,   0.0f, 0.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f,   // top left
         50.0f,  0.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,   // top right
         0.0f,   50.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
         50.0f,  50.0f, 0.0f,   1.0f, 1.0f, 0.0f,   1.0f, 0.0f,    // bottom right

        // positions          // colors           // texture coords
         0.0f,   0.0f, 0.0f,         1.0f, 0.0f, 0.0f,   0.0f, 1.0f,   // top left
         50.0f/4.0f,  0.0f, 0.0f,    0.0f, 1.0f, 0.0f,   1.0f, 1.0f,   // top right
         0.0f,   50.0f/4.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
         50.0f/4.0f,  50.0f/4.0f,    0.0f,   1.0f, 1.0f, 0.0f,   1.0f, 0.0f,    // bottom right

        // positions          // colors           // texture coords
         -800.0f/2.0f,   -600.0f/2.0f, 0.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f,   // top left
         600.0f/2.0f,  -600.0f/2.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,   // top right
         -800.0f/2.0f,   600.0f/2.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
         600.0f/2.0f,  600.0f/2.0f, 0.0f,   1.0f, 1.0f, 0.0f,   1.0f, 0.0f,    // bottom right
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 2,  // first Triangle
        1, 2, 3   // second Triangle
    };

    unsigned int fontIndices[] = {  // note that we start from 0!
        4, 5, 6, // text quads
        5, 6, 7
    };

    unsigned int lineIndices[] = {  // note that we start from 0!
        4, 5, 6, // text quads
        5, 6, 7
    };

    unsigned int backgroundIndices[] = {
        8, 9, 10,
        9, 10, 11
    };

    unsigned int VBO, VAO, EBO, TEBO, BGEBO;

    uint cellTexture;
    glGenTextures(1, &cellTexture);
    glBindTexture(GL_TEXTURE_2D, cellTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char *data = stbi_load("cellTexture.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else { std::cout << "Failed to load texture" << std::endl; }
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, cellTexture);

    uint fontTexture;
    glGenTextures(1, &fontTexture);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int fontWidth, fontHeight, fontnrChannels;
    unsigned char *fontData = stbi_load("bitmapfont.jpg", &fontWidth, &fontHeight, &fontnrChannels, 0);
    if (fontData) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fontWidth, fontHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, fontData);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else { std::cout << "Failed to load texture" << std::endl; }
    stbi_image_free(fontData);
    glBindTexture(GL_TEXTURE_2D, fontTexture);

    uint bgTexture;
    uint defaultBgTexture;
    int bgWidth, bgHeight, bgNrChannels;
    defaultBgTexture = loadTexture("bgImage.jpg", bgWidth, bgHeight, bgNrChannels);

    // load all background images
    std::vector<std::string> imagesPathes;
    std::string path = "backgrounds";
    if (std::filesystem::exists(path)) {
        for (const auto & entry : std::filesystem::directory_iterator(path)) {
            std::string entryPath = entry.path().string();
            if (entryPath.substr( entryPath.find_last_of(".") + 1) == "jpg") {
                imagesPathes.push_back(entryPath);
            }
        }
    }
    for (int i=0; i<imagesPathes.size(); i++) {
        std::cout << imagesPathes[i] << std::endl;
    }

    std::vector<uint> bgTextures;
    bgTextures.push_back(defaultBgTexture);
    for (int i=0; i<imagesPathes.size(); i++) {
        bgTextures.push_back( loadTexture(imagesPathes[i], bgWidth, bgHeight, bgNrChannels) );
        std::cout << "inserting texture\n";
    }
    bgTexture = defaultBgTexture;
    int currentTexture = 0;

    // scanf("%*c");
    // std::string fn = "filename.conf";
    // if (fn.substr(fn.find_last_of(".") + 1) == "conf") {
    //     std::cout << "Yes..." << std::endl;
    // } else {
    //     std::cout << "No..." << std::endl;
    // }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &TEBO);
    glGenBuffers(1, &BGEBO);
    // glGenBuffers(1, &LEBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(fontIndices), fontIndices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BGEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(backgroundIndices), backgroundIndices, GL_STATIC_DRAW);

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, LEBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(fontIndices), fontIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glm::mat4 viewMatrix = glm::mat4(1.0f);
    // viewMatrix = glm::translate(viewMatrix, glm::vec3(50.0f, 0.0f, 0.0f));
    // viewMatrix = glm::rotate(viewMatrix, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));

    Page btree;
    btree.isRoot = true;
    btree.t = M;
    Vector2i cam;
    // srand(42069);
    // for (int i=1; i<=17; i++) {
    //     // int r = rand()%1000;
    //     printf("INSERTING %d\n", i);
    //     btree.insert(i);
    //     btree.report();
    //
    //     btree.self_draw(1);
    // }

    int nextInsert = 0;
    double insertCooldown = 0.2;
    double bgChangeCooldown = 0.2;
    double lastTimeRecord = glfwGetTime();
    double actualTimeRecord = glfwGetTime();
    double frameDifference;
    while (!glfwWindowShouldClose(window)) {
        actualTimeRecord = glfwGetTime();
        frameDifference = actualTimeRecord - lastTimeRecord;
        insertCooldown -= frameDifference;
        insertCooldown = (insertCooldown < 0) ? 0 : insertCooldown;
        bgChangeCooldown -= frameDifference;
        bgChangeCooldown = (bgChangeCooldown < 0) ? 0 : bgChangeCooldown;
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            viewMatrix = glm::translate(viewMatrix, glm::vec3(-5.0f, 0.0f, 0.0f));
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            viewMatrix = glm::translate(viewMatrix, glm::vec3(5.0f, 0.0f, 0.0f));
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            viewMatrix = glm::translate(viewMatrix, glm::vec3(0.0f, -5.0f, 0.0f));
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            viewMatrix = glm::translate(viewMatrix, glm::vec3(0.0f, 5.0f, 0.0f));
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
            viewMatrix = glm::scale(viewMatrix, glm::vec3(0.95f, 0.95f, 0.0f));
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
            viewMatrix = glm::scale(viewMatrix, glm::vec3(1.05f, 1.05f, 0.0f));
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
            viewMatrix = glm::mat4(1.0f);
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && insertCooldown <= 0) {
            btree.insert(++nextInsert);
            insertCooldown = 0.2;
        }
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && insertCooldown <= 0) {
            btree.insert(--nextInsert);
            insertCooldown = 0.2;
        }
        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
            int userInsert = 0;
            printf("Digite numero: ");
            scanf("%d%*c", &userInsert);
            btree.insert(userInsert);
            nextInsert = userInsert;
        }
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && bgChangeCooldown <= 0) {
            currentTexture = (currentTexture + 1) % bgTextures.size();
            bgTexture = bgTextures[currentTexture];
            bgChangeCooldown = 0.2;
            printf("bgTexture: %d\n", currentTexture);
        }
        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && bgChangeCooldown <= 0) {
            currentTexture = (currentTexture - 1 + bgTextures.size()) % (bgTextures.size());
            bgTexture = bgTextures[currentTexture];
            bgChangeCooldown = 0.2;
            printf("bgTexture: %d\n", currentTexture);
        }
        lastTimeRecord = actualTimeRecord;
        // viewMatrix = glm::rotate(viewMatrix, 0.01f, glm::vec3(0.0f, 0.0f, 1.0f));
        // if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        //     printf("right\n");
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        btree.updatePositions(projection);

        glUseProgram(textureSP);

        glm::vec2 bgTransform = glm::vec2(0.0f, 0.0f);
        glm::vec2 transform = glm::vec2(0.0f, 0.0f);

        int transformLoc = glGetUniformLocation(textureSP, "transform");
        // std::cout << transformLoc << std::endl;
        int projectionLoc = glGetUniformLocation(textureSP, "projection");
        // std::cout << "cell projection " << projectionLoc << std::endl;
        if (projectionLoc < 0) {
            std::cout << "fail getting projection uniform location\n";
        }
        int viewLoc = glGetUniformLocation(textureSP, "view");

        // draw background
        glm::mat4 bgView(1.0f);
        glUniform2fv(transformLoc, 1, glm::value_ptr(transform));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(bgView));

        glBindVertexArray(VAO);
        glBindTexture(GL_TEXTURE_2D, bgTexture);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(backgroundIndices), backgroundIndices, GL_STATIC_DRAW);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glUniform2fv(transformLoc, 1, glm::value_ptr(transform));

        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));

        glBindVertexArray(VAO);
        glBindTexture(GL_TEXTURE_2D, cellTexture);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        btree.draw(transformLoc);

        glUseProgram(textSP);
        // glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fontTexture);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(fontIndices), fontIndices, GL_STATIC_DRAW);

        // std::cout << glGetError() << std::endl;
        transformLoc = glGetUniformLocation(textSP, "transform");
        projectionLoc = glGetUniformLocation(textSP, "projection");
        int cropLoc = glGetUniformLocation(textSP, "crop");
        viewLoc = glGetUniformLocation(textSP, "view");
        // std::cout << "transform " << transformLoc << std::endl;
        // std::cout << "projection " << projectionLoc << std::endl;
        // std::cout << "crop " << cropLoc << std::endl;
        // std::cout << "view " << viewLoc << std::endl;

        int errorIndex = 0;
        GLenum errorCode = glGetError();
        while (errorCode != GL_NO_ERROR) {
            std::cout << errorIndex << ": " << errorCode << std::endl;
            errorIndex++;
            errorCode = glGetError();
        }


        glm::mat4 fontViewMatrix = viewMatrix;
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
        btree.writeKeyNumbers(transformLoc, cropLoc, fontWidth, fontHeight, fontIndices);
        // int errorIndex = 0;
        // GLenum errorCode = glGetError();
        // while (errorCode != GL_NO_ERROR) {
        //     std::cout << errorIndex << ": " << std::hex << errorCode << std::endl;
        //     errorIndex++;
        //     errorCode = glGetError();
        // }

        glUseProgram(lineSP);
        // glBindTexture(GL_TEXTURE_2D, cellTexture);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        projectionLoc = glGetUniformLocation(lineSP, "projection");
        viewLoc = glGetUniformLocation(lineSP, "view");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
        btree.drawLinks(lineSP);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    // std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}