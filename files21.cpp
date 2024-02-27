#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <iostream>

// Define structs for Point and Node
struct Point
{
    float x;
    float y;
};

struct Node : Point
{
    bool hasHandle1;
    bool hasHandle2;
    Point handle1;
    Point handle2;
};

// Global variables for nodes and control points
std::vector<Node> nodes;

// keep track of node index
int selectedNodeIndex = -1;
int numOfNodes = -1; // keep track of num of nodes created so far
int width = 0;
int height = 0;
// mouse position
double lastMouseX = 0.0;
double lastMouseY = 0.0;
bool mouseButtonPressed = false;
// Find the distance between two points
float distance(const Point &p1, const Point &p2)
{
    return std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
}

// Function to find the index of the node closest to the cursor position
int findClosestNodeIndex(double xpos, double ypos)
{
    int closestIndex = -1;
    float minDistance = std::numeric_limits<float>::max();
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        float dist = distance({(float)xpos, (float)ypos}, {nodes[i].x, nodes[i].y});
        if (dist < minDistance)
        {
            minDistance = dist;
            closestIndex = i;
        }
    }
    return closestIndex;
}
// function to find in cursor pressed is in 10 pixels of an point
int findOpenSpace(double xpos, double ypos)
{
    int closestIndex = -1;
    for (size_t i = 0; i < nodes.size(); ++i)
    {

        int absx = std::abs(xpos - nodes[i].x);
        int absy = std::abs(ypos - nodes[i].y);

        if (absx <= 10 && absy <= 10)
        {
            closestIndex = 0;
        }
    }
    return closestIndex;
}

// function for mouse click
void mouseClickCallback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {

            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos); // get cursor position

            float yGL = height - ypos;

            // Now the yGL represents the cursor position in OpenGL coordinates
            float xGL = static_cast<float>(xpos);

            int space = findOpenSpace(xGL, yGL);

            if (space == -1) // if theres space without node pressed go in
            {

                numOfNodes++;
                if (numOfNodes > 1)
                {
                    nodes[numOfNodes - 1].hasHandle2 = true; // add second control point
                }
                Node newNode;
                // node position
                newNode.x = xGL;
                newNode.y = yGL;
                // control point position
                newNode.handle1.x = xGL;
                newNode.handle1.y = yGL + 50;
                newNode.handle2.x = xGL;
                newNode.handle2.y = yGL - 50;
                newNode.hasHandle1 = true;
                newNode.hasHandle2 = false;
                nodes.push_back(newNode);

                // redraw
                glfwSwapBuffers(window);
            }
            else
            {
                // setup to move point
                mouseButtonPressed = true;
                selectedNodeIndex = findClosestNodeIndex(xGL, yGL);
                lastMouseX = xGL;
                lastMouseY = yGL;
            }
        }
        else if (action == GLFW_RELEASE)
        {
            mouseButtonPressed = false;
            // Reset the selected node index when mouse button is released
            selectedNodeIndex = -1;
        }
    }
}

// Callback function for mouse movement
void mouseMoveCallback(GLFWwindow *window, double xpos, double ypos)
{
    ypos = ypos + 150; // help reduce movment on first move
    if (mouseButtonPressed && selectedNodeIndex != -1)
    {
        // Calculate the offset from the last position
        double xOffset = xpos - lastMouseX;
        double yOffset = ypos - lastMouseY;

        // Update the node's position
        nodes[selectedNodeIndex].x += static_cast<float>(xOffset);
        nodes[selectedNodeIndex].y -= static_cast<float>(yOffset);
        nodes[selectedNodeIndex].handle1.x += static_cast<float>(xOffset);
        nodes[selectedNodeIndex].handle1.y -= static_cast<float>(yOffset);
        nodes[selectedNodeIndex].handle2.x += static_cast<float>(xOffset);
        nodes[selectedNodeIndex].handle2.y -= static_cast<float>(yOffset);

        // Update last mouse position
        lastMouseX = xpos;
        lastMouseY = ypos;

        glfwSwapBuffers(window);
    }
}

// clear nodes if e is pressed
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
    {

        nodes.clear();
    }
}
// Function to calculate a point on a cubic Bezier curve
Point calculateBezierPoint(const Point &p0, const Point &p1, const Point &p2, const Point &p3, float t)
{
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    // Bezier curve equation
    Point point;
    point.x = uuu * p0.x + 3 * uu * t * p1.x + 3 * u * tt * p2.x + ttt * p3.x;
    point.y = uuu * p0.y + 3 * uu * t * p1.y + 3 * u * tt * p2.y + ttt * p3.y;

    return point;
}

// Function to render the spline, nodes, and control points
void render()
{
    // Render each piece of the spline as an independent cubic Bezier curve
    glLineWidth(2.0f);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor3f(0.0f, 0.0f, 1.0f); // Example color (blue)

    if (nodes.size() != 0)
    {

        for (size_t i = 0; i < nodes.size() - 1; ++i)
        {

            glBegin(GL_LINE_STRIP);
            for (int j = 0; j <= 200; ++j)
            {
                float t = static_cast<float>(j) / 200.0f;

                // Calculate intermediate control points for the Bezier curve
                Point p0 = nodes[i];             // Start point of the curve segment
                Point p3 = nodes[i + 1];         // End point of the curve segment
                Point p1 = nodes[i].handle2;     // First control point (handle2 of the start node)
                Point p2 = nodes[i + 1].handle1; // Second control point (handle1 of the end node)

                // Calculate the point on the Bezier curve
                Point curvePoint = calculateBezierPoint(p0, p1, p2, p3, t);
                glVertex2f(curvePoint.x, curvePoint.y);
            }
            glEnd();
        }
    }

    // Render nodes as squares
    glPointSize(10.0f);
    glColor3f(1.0f, 0.0f, 0.0f); // make lines red
    glBegin(GL_QUADS);
    for (const auto &node : nodes)
    {
        float halfSize = 5.0f;

        // Render a square centered at the node position
        glVertex2f(node.x - halfSize, node.y - halfSize); // Bottom-left corner
        glVertex2f(node.x + halfSize, node.y - halfSize); // Bottom-right corner
        glVertex2f(node.x + halfSize, node.y + halfSize); // Top-right corner
        glVertex2f(node.x - halfSize, node.y + halfSize); //
    }
    glEnd();

    // Render control points as points

    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_POINTS);
    for (const auto &node : nodes)
    {
        if (node.hasHandle1)
        {
            // Render handle1
            glVertex2f(node.handle1.x, node.handle1.y);
        }
        if (node.hasHandle2)
        {
            // Render handle2
            glVertex2f(node.handle2.x, node.handle2.y);
        }
    }
    glEnd();

    // Render dotted lines connecting control points to nodes
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(4, 0xAAAA);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINES);
    for (const auto &node : nodes)
    {
        if (node.hasHandle1)
        {
            glVertex2f(node.x, node.y);
            glVertex2f(node.handle1.x, node.handle1.y);
        }
        if (node.hasHandle2)
        {
            glVertex2f(node.x, node.y);
            glVertex2f(node.handle2.x, node.handle2.y);
        }
    }
    glEnd();
    glDisable(GL_LINE_STIPPLE);
}

int main(int argc, char *argv[])
{
    // Check command-line arguments
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <screen_width> <screen_height>" << std::endl;
        return -1;
    }

    height = std::atoi(argv[1]);
    width = std::atoi(argv[2]);

    // Initialize GLFW
    if (!glfwInit())
    {
        return -1;
    }

    // Create a windowed mode window and its OpenGL context
    GLFWwindow *window = glfwCreateWindow(width, height, "Bezier Spline Tool", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Set up projection matrix and viewport to match window coordinates
    glOrtho(0, width, 0, height, -1, 1);
    glViewport(0, 0, width, height);

    // Enable 4 times multisampling for anti-aliasing
    glEnable(GL_MULTISAMPLE);

    // Set up callbacks for mouse and keyboard input
    glfwSetMouseButtonCallback(window, mouseClickCallback);
    glfwSetCursorPosCallback(window, mouseMoveCallback);
    glfwSetKeyCallback(window, keyCallback);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Clear the buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // Render the spline, nodes, and control points
        render();

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Terminate GLFW
    glfwTerminate();

    return 0;
}
