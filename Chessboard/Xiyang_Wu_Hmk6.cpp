/*

Author: <Xiyang Wu>
Class: ECE6122
Last Date Modified: <11/26/2019>

Description:

Homework6: OpenGL Chess Set
C++ IDE: CLion 2019.02

*/

#include <iostream>
#include <GL/glut.h> // Include the GLUT header file
#include <stdio.h>
#include <math.h>
#include <stdlib.h> // Standard definitions
#include <time.h> // Include the timer as the seed for the random number
#include <string> // The string header
#include <vector> // The vector header
#include <fstream> // The file I/O header
#include <sstream> // The file I/O header

// Global variance: the rotation angle of the chessboard
double rotate_Angle = 0.0;

// Global variance: the spatial deviation of the eye
double view_Position_Z = 0.0;

// Global variance: the current status of the chessboard
int current_Chessboard[8][8];

// The current status of all the pawns
int pawn_Status[16];

// The current status of all the knights
int knight_Status[4][2];

// The flag whether turns on the light 0
int enable_Light0 = 1;

// The flag whether turns on the light 0
int enable_Light1 = 1;

// The flag whether enables the realistic model
int enable_Realistic = 0;

// Initialize the chessboard status
void chessboard_Init()
{
    // Initialize the initial position of each piece
    // The 1st, 2nd, 7th and 8th row will be set as 1, and the rest of them will be set as 0
    for (int i = 0; i < 8; i++)
    {
        for (int j = 2; j < 6; j++)
        {
            current_Chessboard[i][j] = 0;
        }
        current_Chessboard[i][0] = 1;
        current_Chessboard[i][1] = 1;
        current_Chessboard[i][6] = 1;
        current_Chessboard[i][7] = 1;
    }

    // Store the initial position of the 1st knight
    knight_Status[0][0] = 1;
    knight_Status[0][1] = 0;

    // Store the initial position of the 2nd knight
    knight_Status[1][0] = 6;
    knight_Status[1][1] = 0;

    // Store the initial position of the 3rd knight
    knight_Status[2][0] = 1;
    knight_Status[2][1] = 7;

    // Store the initial position of the 4th knight
    knight_Status[3][0] = 6;
    knight_Status[3][1] = 7;

    // Initialize the status of each pawn
    for (int i = 0; i < 16; i++)
    {
        pawn_Status[i] = 0;
    }
}


// The class to define and store the object information loaded from the file
class objectLoader
{
public:
    // Load the model from the file
    objectLoader(std::string filename);
    // The drawing function
    void modelDrawer();

private:
    // Store the x, y,z coordinate of each point
    std::vector<std::vector<GLfloat>> pointSets;
    // Store the index of the faces
    std::vector<std::vector<GLint>> faceSets;
};

// The funciton to import the data from the .obj file
objectLoader::objectLoader(std::string filename)
{
    // The variance to store the data imported from the file line by line
    std::ifstream file(filename);
    std::string line;

    // Read the data until the EOF symbol
    while (getline(file, line))
    {
        // Read the point of each triangle face (Begin with a "v")
        if (line.substr(0, 1) == "v")
        {
            // Read and store the coordination of each point
            std::vector<GLfloat> point_Imported;
            GLfloat x, y, z;
            std::istringstream s(line.substr(2));
            s >> x;
            s >> y;
            s >> z;
            point_Imported.push_back(x);
            point_Imported.push_back(y);
            point_Imported.push_back(z);
            // Store the points' coordination to the point set
            pointSets.push_back(point_Imported);
        }
        else if (line.substr(0, 1) == "f")
        {
            // Read and store the coordination of each point
            std::vector<GLint> point_Index_Sets;
            GLint u, v, w;
            std::istringstream vtns(line.substr(2));
            vtns >> u;
            vtns >> v;
            vtns >> w;
            point_Index_Sets.push_back(u - 1);
            point_Index_Sets.push_back(v - 1);
            point_Index_Sets.push_back(w - 1);
            // Store the index of the face vertex into the corresponding vector array
            faceSets.push_back(point_Index_Sets);
        }
    }
    file.close();
}

// The function to paint the object
void objectLoader::modelDrawer()
{
    // Begin to paint the model
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < faceSets.size(); i++)
    {
        // The normal vector of the face
        GLfloat normal[3];
        // The three vertices of the triangle
        GLfloat vertex1[3];
        GLfloat vertex2[3];
        GLfloat vertex3[3];

        // If the number of the vertex points is larger than 3, output the error message
        if ((faceSets[i]).size() != 3)
        {
            std::cout << "The face set contains error points" << std::endl;
        }
        else
        {
            // Read the corresponding vertex of the current triangle face
            GLint first_Vertex = (faceSets[i])[0];
            GLint second_Vertex = (faceSets[i])[1];
            GLint third_Vertex = (faceSets[i])[2];

            // The coordination of the first vertex
            vertex1[0] = (pointSets[first_Vertex])[0];
            vertex1[1] = (pointSets[first_Vertex])[1];
            vertex1[2] = (pointSets[first_Vertex])[2];

            // The coordination of the second vertex
            vertex2[0] = (pointSets[second_Vertex])[0];
            vertex2[1] = (pointSets[second_Vertex])[1];
            vertex2[2] = (pointSets[second_Vertex])[2];

            // The coordination of the third vertex
            vertex3[0] = (pointSets[third_Vertex])[0];
            vertex3[1] = (pointSets[third_Vertex])[1];
            vertex3[2] = (pointSets[third_Vertex])[2];

            // Calculate the normal vector
            GLfloat vec1[3], vec2[3], vec3[3];
            // Calculate the normal vector 1
            vec1[0] = vertex1[0] - vertex2[0];
            vec1[1] = vertex1[1] - vertex2[1];
            vec1[2] = vertex1[2] - vertex2[2];

            // Calculate the normal vector 2
            vec2[0] = vertex1[0] - vertex3[0];
            vec2[1] = vertex1[1] - vertex3[1];
            vec2[2] = vertex1[2] - vertex3[2];

            // Calculate the normal vector 3
            vec3[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
            vec3[1] = vec1[2] * vec2[0] - vec1[0] * vec2[2];
            vec3[2] = vec1[0] * vec2[1] - vec1[1] * vec2[0];

            // Normalize the normal vector calculated
            GLfloat amplitude = sqrt(pow(vec3[0], 2) + pow(vec3[1], 2) + pow(vec3[2], 2));
            normal[0] = vec3[0] / amplitude;
            normal[1] = vec3[1] / amplitude;
            normal[2] = vec3[2] / amplitude;

            // Draw the normal vector
            glNormal3f(normal[0], normal[1], normal[2]);

            // Paint the triangle face
            glVertex3f(vertex1[0], vertex1[1], vertex1[2]);
            glVertex3f(vertex2[0], vertex2[1], vertex2[2]);
            glVertex3f(vertex3[0], vertex3[1], vertex3[2]);
        }
    }
    glEnd();
}

// Simulate the movement of the pawn
void pawn_Movement()
{
    // Initialize the random seed
    srand((unsigned)time(NULL));

    // The amount of the pawn that could not move
    int disabled_Pawn_Count = 0;

    // Store the current status of each pawn
    int disabled_Pawn[16];

    // Initialize the initial status of each pawn
    for(int i = 0; i < 16; i++)
    {
        disabled_Pawn[i] = 0;
    }

    // The end condition of the loop
    // If all the pawns on the chessboard are disabled, the loop will break out
    while (disabled_Pawn_Count <= 16)
    {
        // Initialize the pawn selected
        int current_Pawn = rand() % 16;

        // Determine the current position of the selected pawn
        int current_Pawn_X;
        int current_Pawn_Y;

        // Determine the Y coordinate of the given pawn
        // Whether the pawn selected is a white or black one
        if (current_Pawn >= 8)
        {
            current_Pawn_X = current_Pawn - 8;
            current_Pawn_Y = 6 + pawn_Status[current_Pawn];
        }
        else
        {
            current_Pawn_X = current_Pawn;
            current_Pawn_Y = 1 + pawn_Status[current_Pawn];
        }

        // Determine whether the block on the front or back of the given pawn has been occupied or not
        if ((current_Chessboard[current_Pawn_X][current_Pawn_Y - 1] != 1)||(current_Chessboard[current_Pawn_X][current_Pawn_Y + 1] != 1))
        {
            // Initialize the random number for the movement that the pawn selected
            int move_Judge = rand() % 2;

            // If the move_Judge is 0, the default movement of the pawn is backward.
            // If the block on the back of the piece has been occupied, the piece will move forward
            if (move_Judge == 0)
            {
                // If the block on the back of the piece has not been occupied, the piece will move backward
                if (current_Chessboard[current_Pawn_X][current_Pawn_Y - 1] != 1)
                {
                    // Update the current status of the pawn's position
                    pawn_Status[current_Pawn] -= 1;

                    // Update the current status of the chessboard
                    current_Chessboard[current_Pawn_X][current_Pawn_Y - 1] = 1;
                    current_Chessboard[current_Pawn_X][current_Pawn_Y] = 0;
                }
                else
                {
                    // If the block on the back of the piece has been occupied, the piece will move forward
                    // Update the current status of the pawn's position
                    pawn_Status[current_Pawn] += 1;

                    // Update the current status of the chessboard
                    current_Chessboard[current_Pawn_X][current_Pawn_Y + 1] = 1;
                    current_Chessboard[current_Pawn_X][current_Pawn_Y] = 0;
                }
            }
            else
            {
                // If the block on the front of the piece has not been occupied, the piece will move forward
                if (current_Chessboard[current_Pawn_X][current_Pawn_Y + 1] != 1)
                {
                    // Update the current status of the pawn's position
                    pawn_Status[current_Pawn] += 1;

                    // Update the current status of the chessboard
                    current_Chessboard[current_Pawn_X][current_Pawn_Y + 1] = 1;
                    current_Chessboard[current_Pawn_X][current_Pawn_Y] = 0;
                }
                else
                {
                    // If the block on the back of the piece has been occupied, the piece will move backward
                    // Update the current status of the pawn's position
                    pawn_Status[current_Pawn] -= 1;

                    // Update the current status of the chessboard
                    current_Chessboard[current_Pawn_X][current_Pawn_Y - 1] = 1;
                    current_Chessboard[current_Pawn_X][current_Pawn_Y] = 0;
                }
            }
            // Set the current number of the disabled pawn as 0
            // In the next loop, this number will be counted from the beginning
            disabled_Pawn_Count = 0;
            break;
        }
        else
        {
            // If the selected pawn could not move, its corresponding record will be set as 1
            disabled_Pawn[current_Pawn] = 1;
        }

        // Count the current number of the disabled pawn
        for(int i = 0; i < 16; i++)
        {
            disabled_Pawn_Count++;
        }
    }
}

// Simulate the movement of the knight
void knight_Movement()
{
    // Initialize the random number seed
    srand((unsigned)time(NULL));

    // The array to store the movement status of the knight
    int disabled_Knight_Movement[8];

    // The array to store the status of the knight
    int disabled_Knight[4];

    // Count the number of the disabled knight
    int disabled_Knight_Count = 0;

    // Initialize the array to store the status of the knight
    for(int i = 0; i < 4; i++)
    {
        disabled_Knight[i] = 0;
    }

    // The end condition of the loop
    // If all of the four knight are disabled, the loop will break
    while(disabled_Knight_Count <= 4)
    {
        // The variance to count the disabled knight
        int disabled_Knight_Movement_Count = 0;

        // The random number of the selected knight
        int knight_Selected = rand() % 4;

        // The movement of the knight (Random number)
        int knight_Movement_Flag = 0;

        // The end condition of the loop
        // If all of the eight movement of the knight are disabled, the loop will break
        while (disabled_Knight_Movement_Count <= 8)
        {
            // Count the disabled movement
            disabled_Knight_Movement_Count = 0;

            // The random number to store the movement that the knight selects
            int movement_Flag = rand() % 8;

            // Determine the movement that the given knight takes
            switch (movement_Flag)
            {
                // The first kind of movement of the knight
                case 0:
                    // If the goal block is not occupied and the movement does not exceed the edge of the chessboard, update the status of the chessboard and the knight
                    if ((current_Chessboard[knight_Status[knight_Selected][0] - 2][knight_Status[knight_Selected][1] + 1] != 1)&&((knight_Status[knight_Selected][0] - 2) >= 0)&&((knight_Status[knight_Selected][1] + 1) <= 7))
                    {
                        // Update the status of the chessboard
                        current_Chessboard[knight_Status[knight_Selected][0]][knight_Status[knight_Selected][1]] = 0;
                        current_Chessboard[knight_Status[knight_Selected][0] - 2][knight_Status[knight_Selected][1] + 1] = 1;

                        // Update the stored status of the knight
                        knight_Status[knight_Selected][0] -= 2;
                        knight_Status[knight_Selected][1] += 1;
                        knight_Movement_Flag = 1;
                    }
                    // If not, set the corresponding value in the array of the disabled knight as 1
                    else
                    {
                        disabled_Knight_Movement[0] = 1;
                    }
                    break;
                // The second kind of movement of the knight
                case 1:
                    // If the goal block is not occupied and the movement does not exceed the edge of the chessboard, update the status of the chessboard and the knight
                    if ((current_Chessboard[knight_Status[knight_Selected][0] - 1][knight_Status[knight_Selected][1] + 2] != 1)&&((knight_Status[knight_Selected][0] - 1) >= 0)&&((knight_Status[knight_Selected][1] + 2) <= 7))
                    {
                        // Update the status of the chessboard
                        current_Chessboard[knight_Status[knight_Selected][0]][knight_Status[knight_Selected][1]] = 0;
                        current_Chessboard[knight_Status[knight_Selected][0] - 1][knight_Status[knight_Selected][1] + 2] = 1;

                        // Update the stored status of the knight
                        knight_Status[knight_Selected][0] -= 1;
                        knight_Status[knight_Selected][1] += 2;
                        knight_Movement_Flag = 1;
                    }
                    // If not, set the corresponding value in the array of the disabled knight as 1
                    else
                    {
                        disabled_Knight_Movement[1] = 1;
                    }
                    break;
                // The third kind of movement of the knight
                case 2:
                    // If the goal block is not occupied and the movement does not exceed the edge of the chessboard, update the status of the chessboard and the knight
                    if ((current_Chessboard[knight_Status[knight_Selected][0] + 1][knight_Status[knight_Selected][1] + 2] != 1)&&((knight_Status[knight_Selected][0] + 1) <= 7)&&((knight_Status[knight_Selected][1] + 2) <= 7))
                    {
                        // Update the status of the chessboard
                        current_Chessboard[knight_Status[knight_Selected][0]][knight_Status[knight_Selected][1]] = 0;
                        current_Chessboard[knight_Status[knight_Selected][0] + 1][knight_Status[knight_Selected][1] + 2] = 1;

                        // Update the stored status of the knight
                        knight_Status[knight_Selected][0] += 1;
                        knight_Status[knight_Selected][1] += 2;
                        knight_Movement_Flag = 1;
                    }
                    // If not, set the corresponding value in the array of the disabled knight as 1
                    else
                    {
                        disabled_Knight_Movement[2] = 1;
                    }
                    break;
                // The fourth kind of movement of the knight
                case 3:
                    // If the goal block is not occupied and the movement does not exceed the edge of the chessboard, update the status of the chessboard and the knight
                    if ((current_Chessboard[knight_Status[knight_Selected][0] + 2][knight_Status[knight_Selected][1] + 1] != 1)&&((knight_Status[knight_Selected][0] + 2) <= 7)&&((knight_Status[knight_Selected][1] + 1) <= 7))
                    {
                        // Update the status of the chessboard
                        current_Chessboard[knight_Status[knight_Selected][0]][knight_Status[knight_Selected][1]] = 0;
                        current_Chessboard[knight_Status[knight_Selected][0] + 2][knight_Status[knight_Selected][1] + 1] = 1;

                        // Update the stored status of the knight
                        knight_Status[knight_Selected][0] += 2;
                        knight_Status[knight_Selected][1] += 1;
                        knight_Movement_Flag = 1;
                    }
                    // If not, set the corresponding value in the array of the disabled knight as 1
                    else
                    {
                        disabled_Knight_Movement[3] = 1;
                    }
                    break;
                // The fifth kind of movement of the knight
                case 4:
                    // If the goal block is not occupied and the movement does not exceed the edge of the chessboard, update the status of the chessboard and the knight
                    if ((current_Chessboard[knight_Status[knight_Selected][0] + 2][knight_Status[knight_Selected][1] - 1] != 1)&&((knight_Status[knight_Selected][0] + 2) <= 7)&&((knight_Status[knight_Selected][1] - 1) >= 0))
                    {
                        // Update the status of the chessboard
                        current_Chessboard[knight_Status[knight_Selected][0]][knight_Status[knight_Selected][1]] = 0;
                        current_Chessboard[knight_Status[knight_Selected][0] + 2][knight_Status[knight_Selected][1] - 1] = 1;

                        // Update the stored status of the knight
                        knight_Status[knight_Selected][0] += 2;
                        knight_Status[knight_Selected][1] -= 1;
                        knight_Movement_Flag = 1;
                    }
                    // If not, set the corresponding value in the array of the disabled knight as 1
                    else
                    {
                        disabled_Knight_Movement[4] = 1;
                    }
                    break;
                // The sixth kind of movement of the knight
                case 5:
                    // If the goal block is not occupied and the movement does not exceed the edge of the chessboard, update the status of the chessboard and the knight
                    if ((current_Chessboard[knight_Status[knight_Selected][0] + 1][knight_Status[knight_Selected][1] - 2] != 1)&&((knight_Status[knight_Selected][0] + 1) <= 7)&&((knight_Status[knight_Selected][1] - 2) >= 0))
                    {
                        // Update the status of the chessboard
                        current_Chessboard[knight_Status[knight_Selected][0]][knight_Status[knight_Selected][1]] = 0;
                        current_Chessboard[knight_Status[knight_Selected][0] + 1][knight_Status[knight_Selected][1] - 2] = 1;

                        // Update the stored status of the knight
                        knight_Status[knight_Selected][0] += 1;
                        knight_Status[knight_Selected][1] -= 2;
                        knight_Movement_Flag = 1;
                    }
                    // If not, set the corresponding value in the array of the disabled knight as 1
                    else
                    {
                        disabled_Knight_Movement[5] = 1;
                    }
                    break;
                // The seventh kind of movement of the knight
                case 6:
                    // If the goal block is not occupied and the movement does not exceed the edge of the chessboard, update the status of the chessboard and the knight
                    if ((current_Chessboard[knight_Status[knight_Selected][0] - 1][knight_Status[knight_Selected][1] - 2] != 1)&&((knight_Status[knight_Selected][0] - 1) >= 0)&&((knight_Status[knight_Selected][1] - 2) >= 0))
                    {
                        // Update the status of the chessboard
                        current_Chessboard[knight_Status[knight_Selected][0]][knight_Status[knight_Selected][1]] = 0;
                        current_Chessboard[knight_Status[knight_Selected][0] - 1][knight_Status[knight_Selected][1] - 2] = 1;

                        // Update the stored status of the knight
                        knight_Status[knight_Selected][0] -= 1;
                        knight_Status[knight_Selected][1] -= 2;
                        knight_Movement_Flag = 1;
                    }
                    // If not, set the corresponding value in the array of the disabled knight as 1
                    else
                    {
                        disabled_Knight_Movement[6] = 1;
                    }
                    break;
                // The eighth kind of movement of the knight
                case 7:
                    // If the goal block is not occupied and the movement does not exceed the edge of the chessboard, update the status of the chessboard and the knight
                    if ((current_Chessboard[knight_Status[knight_Selected][0] - 2][knight_Status[knight_Selected][1] - 1] != 1)&&((knight_Status[knight_Selected][0] - 2) >= 0)&&((knight_Status[knight_Selected][1] - 1) >= 0))
                    {
                        // Update the status of the chessboard
                        current_Chessboard[knight_Status[knight_Selected][0]][knight_Status[knight_Selected][1]] = 0;
                        current_Chessboard[knight_Status[knight_Selected][0] - 2][knight_Status[knight_Selected][1] - 1] = 1;

                        // Update the stored status of the knight
                        knight_Status[knight_Selected][0] -= 2;
                        knight_Status[knight_Selected][1] -= 1;
                        knight_Movement_Flag = 1;
                    }
                    // If not, set the corresponding value in the array of the disabled knight as 1
                    else
                    {
                        disabled_Knight_Movement[7] = 1;
                    }
                    break;
            }

            // If the knight does not move, count the number of teh disabled knights
            if (knight_Movement_Flag == 0)
            {
                for(int i = 0; i < 8; i++)
                {
                    if (disabled_Knight_Movement[i] == 1)
                    {
                        disabled_Knight_Movement_Count++;
                    }
                }
            }
            // If not, re-initialize the array to store the disabled movement of the given knight
            else
            {
                for(int i = 0; i < 8; i++)
                {
                    disabled_Knight_Movement[i] = 0;
                }
                break;
            }

        }
        // If all the movements of the given knight are disabled, its corresponding status will be set as 1 (Disabled)
        if (knight_Movement_Flag == 0)
        {
            disabled_Knight[knight_Selected] = 1;
        }
        else
        {
            // Re-initialize the disabled knight array as 0 for the next loop
            for(int i = 0; i < 4; i++)
            {
                disabled_Knight[i] = 0;
            }
            break;
        }

        // Count the number of the disabled knights
        for(int i = 0; i < 4; i++)
        {
            if (disabled_Knight[i] == 1)
            {
                disabled_Knight_Count++;
            }
        }
    }
}

// Draw the 8*8 chessboard
void chess_Board_Display()
{
    // The loop to draw the black or white block at each location
    // The length of the edge of each block is 1
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            // Determine whether the white or black block locates at the current location
            if ((i + j) % 2 == 0)
            {
                // Draw the white block with four vertex points
                double x = 1 * i;
                double y = 1 * j;
                glBegin(GL_POLYGON);
                glColor3f(0,0,0);
                glVertex3f(x, y, 0);
                glVertex3f(x, y + 1, 0);
                glVertex3f(x + 1, y + 1, 0);
                glVertex3f(x + 1, y, 0);
                glEnd();
            }
            else
            {
                // Draw the black block with four vertex points
                double x = 1 * i;
                double y = 1 * j;
                glBegin(GL_POLYGON);
                glColor3f(1,1,1);
                glVertex3f(x, y, 0);
                glVertex3f(x, y + 1, 0);
                glVertex3f(x + 1, y + 1, 0);
                glVertex3f(x + 1, y, 0);
                glEnd();
            }

        }
    }
}

// Draw the white pawn
void pawn_White_Display(double x, double y, double z)
{
    glPushMatrix();
    // Set the color of the pawn
    glColor3ub(140,140,135);

    // Put the pawn on the intended position
    glTranslatef(x, y, z);

    // Set the dimension of the pawn
    glScalef(0.375, 0.375, 0.5);

    // Draw the solid model of the pawn (sphere)
    glutSolidSphere(1, 100, 100);
    glPopMatrix();
}

// Draw the realistic white pawn
void pawn_Realistic_White_Display(double x, double y, double z)
{
    glPushMatrix();
    // Set the color of the pawn
    glColor3ub(140,140,135);

    // Put the pawn on the intended position
    glTranslatef(x, y, z);

    // Set the dimension of the pawn
    glScalef(0.5, 0.5, 0.33);

    // Rotate the spatial pose of the pawn model
    glRotatef(90, 1, 0, 0);

    // Load the realistic model of the pawn (Using the absolute path)
    objectLoader objModel = objectLoader("./pawn.obj");

    // Draw the realistic pawn
    objModel.modelDrawer();

    glPopMatrix();
}

// Draw the white pawn
void pawn_Black_Display(double x, double y, double z)
{
    glPushMatrix();
    // Set the color of the pawn
    glColor3ub(150,75,0);

    // Put the pawn on the intended position
    glTranslatef(x, y, z);

    // Set the dimension of the pawn
    glScalef(0.375, 0.375, 0.5);

    // Draw the solid model of the pawn (sphere)
    glutSolidSphere(1, 100, 100);
    glPopMatrix();
}

// Draw the realistic black pawn
void pawn_Realistic_Black_Display(double x, double y, double z)
{
    glPushMatrix();
    // Set the color of the pawn
    glColor3ub(150,75,0);

    // Put the pawn on the intended position
    glTranslatef(x, y, z);

    // Set the dimension of the pawn
    glScalef(0.5, 0.5, 0.33);

    // Rotate the spatial pose of the pawn model
    glRotatef(90, 1, 0, 0);

    // Load the realistic model of the pawn (Using the absolute path)
    objectLoader objModel = objectLoader("./pawn.obj");

    // Draw the realistic pawn
    objModel.modelDrawer();

    glPopMatrix();
}

// Draw the white rook
void rook_White_Display(double x, double y, double z)
{
    glPushMatrix();
    // Set the color of the rook
    glColor3ub(140,140,135);

    // Put the rook on the intended position
    glTranslatef(x, y, z);

    // Set the dimension of the rook
    glScalef(0.75, 0.75, 1);

    // Draw the solid model of the rook (Cube)
    glutSolidCube(1.0);
    glPopMatrix();
}

// Draw the realistic white rook
void rook_Realistic_White_Display(double x, double y, double z)
{
    glPushMatrix();
    // Set the color of the rook
    glColor3ub(140,140,135);

    // Put the rook on the intended position
    glTranslatef(x, y, z);

    // Set the dimension of the rook
    glScalef(0.5, 0.5, 0.33);

    // Rotate the spatial pose of the rook model
    glRotatef(90, 1, 0, 0);

    // Load the realistic model of the rook (Using the absolute path)
    objectLoader objModel = objectLoader("./rook.obj");

    // Draw the realistic rook
    objModel.modelDrawer();

    glPopMatrix();
}

// Draw the black rook
void rook_Black_Display(double x, double y, double z)
{
    glPushMatrix();
    // Set the color of the rook
    glColor3ub(150,75,0);

    // Put the rook on the intended position
    glTranslatef(x, y, z);

    // Set the dimension of the rook
    glScalef(0.75, 0.75, 1);

    // Draw the solid model of the rook (Cube)
    glutSolidCube(1.0);
    glPopMatrix();
}

// Draw the realistic black rook
void rook_Realistic_Black_Display(double x, double y, double z)
{
    glPushMatrix();
    // Set the color of the rook
    glColor3ub(150,75,0);

    // Put the rook on the intended position
    glTranslatef(x, y, z);

    // Set the dimension of the rook
    glScalef(0.5, 0.5, 0.33);

    // Rotate the spatial pose of the rook model
    glRotatef(90, 1, 0, 0);

    // Load the realistic model of the rook (Using the absolute path)
    objectLoader objModel = objectLoader("./rook.obj");

    // Draw the realistic rook
    objModel.modelDrawer();

    glPopMatrix();
}

// Draw the white knight
void knight_White_Display(double x, double y, double z)
{
    glPushMatrix();
    // Set the color of the knight
    glColor3ub(140,140,135);

    // Put the knight on the intended position
    glTranslatef(x, y, z);

    // Set the dimension of the knight
    glScalef(0.45, 0.45, 1.25);

    // Rotate the spatial pose of the knight
    glRotatef(90, 1, 0, 0);

    // Draw the solid model of the knight (Teapot)
    glutSolidTeapot(0.5);
    glPopMatrix();
}

// Draw the realistic white knight
void knight_Realistic_White_Display(double x, double y, double z)
{
    glPushMatrix();
    // Set the color of the knight
    glColor3ub(140,140,135);

    // Put the knight on the intended position
    glTranslatef(x, y, z);

    // Set the dimension of the knight
    glScalef(0.5, 0.5, 0.33);

    // Rotate the spatial pose of the knight model
    glRotatef(90, 1, 0, 0);

    // Load the realistic model of the knight (Using the absolute path)
    objectLoader objModel = objectLoader("./knight.obj");

    // Draw the realistic knight
    objModel.modelDrawer();

    glPopMatrix();
}

// Draw the black knight
void knight_Black_Display(double x, double y, double z)
{
    glPushMatrix();
    // Set the color of the knight
    glColor3ub(150,75,0);

    // Put the knight on the intended position
    glTranslatef(x, y, z);

    // Set the dimension of the knight
    glScalef(0.45, 0.45, 1.25);

    // Rotate the spatial pose of the knight
    glRotatef(90, 1, 0, 0);

    // Draw the solid model of the knight (Teapot)
    glutSolidTeapot(0.5);
    glPopMatrix();
}

// Draw the realistic black knight
void knight_Realistic_Black_Display(double x, double y, double z)
{
    glPushMatrix();
    // Set the color of the knight
    glColor3ub(150,75,0);

    // Put the knight on the intended position
    glTranslatef(x, y, z);

    // Set the dimension of the knight
    glScalef(0.5, 0.5, 0.33);

    // Rotate the spatial pose of the knight model
    glRotatef(90, 1, 0, 0);

    // Load the realistic model of the knight (Using the absolute path)
    objectLoader objModel = objectLoader("./knight.obj");

    // Draw the realistic knight
    objModel.modelDrawer();

    glPopMatrix();
}

// Draw the realistic black bishop
void bishop_Realistic_Black_Display(double x, double y, double z)
{
    glPushMatrix();
    // Set the color of the bishop
    glColor3ub(150,75,0);

    // Put the bishop on the intended position
    glTranslatef(x, y, z);

    // Set the dimension of the bishop
    glScalef(0.5, 0.5, 0.33);

    // Rotate the spatial pose of the bishop model
    glRotatef(90, 1, 0, 0);

    // Load the realistic model of the bishop (Using the absolute path)
    objectLoader objModel = objectLoader("./bishop.obj");

    // Draw the realistic bishop
    objModel.modelDrawer();

    glPopMatrix();
}

// Draw the white bishop
void bishop_White_Display(double x, double y, double z)
{
    glPushMatrix();
    // Set the color of the bishop
    glColor3ub(140,140,135);

    // Put the bishop on the intended position
    glTranslatef(x, y, z);

    // Set the dimension of the bishop
    glScalef(0.375, 0.375, 1.0);

    // Draw the solid model of the bishop (Cone)
    glutSolidCone(1.0, 1.0, 100, 100);
    glPopMatrix();
}

// Draw the realistic white bishop
void bishop_Realistic_White_Display(double x, double y, double z)
{
    glPushMatrix();
    // Set the color of the bishop
    glColor3ub(140,140,135);

    // Put the bishop on the intended position
    glTranslatef(x, y, z);

    // Set the dimension of the bishop
    glScalef(0.5, 0.5, 0.33);

    // Rotate the spatial pose of the bishop model
    glRotatef(90, 1, 0, 0);

    // Load the realistic model of the bishop (Using the absolute path)
    objectLoader objModel = objectLoader("./bishop.obj");

    // Draw the realistic bishop
    objModel.modelDrawer();

    glPopMatrix();
}

// Draw the black bishop
void bishop_Black_Display(double x, double y, double z)
{
    glPushMatrix();
    // Set the color of the bishop
    glColor3ub(150,75,0);

    // Put the bishop on the intended position
    glTranslatef(x, y, z);

    // Set the dimension of the bishop
    glScalef(0.375, 0.375, 1.0);

    // Draw the solid model of the bishop (Cone)
    glutSolidCone(1.0, 1.0, 100, 100);
    glPopMatrix();
}



// Draw the white queen
void queen_White_Display(double x, double y, double z)
{
    glPushMatrix();
    // Set the color of the queen
    glColor3ub(140,140,135);

    // Put the queen on the intended position
    glTranslatef(x, y, z);

    // Set the dimension of the queen
    glScalef(0.375, 0.375, 0.75);

    // Rotate the spatial pose of the queen
    glRotatef(270, 0, 1, 0);

    // Draw the solid model of the queen (Tetrahedron)
    glutSolidTetrahedron();
    glPopMatrix();
}

// Draw the realistic white queen
void queen_Realistic_White_Display(double x, double y, double z)
{
    glPushMatrix();
    // Set the color of the queen
    glColor3ub(140,140,135);

    // Put the queen on the intended position
    glTranslatef(x, y, z);

    // Set the dimension of the queen
    glScalef(0.5, 0.5, 0.33);

    // Rotate the spatial pose of the queen model
    glRotatef(90, 1, 0, 0);

    // Load the realistic model of the queen (Using the absolute path)
    objectLoader objModel = objectLoader("./queen.obj");

    // Draw the realistic queen
    objModel.modelDrawer();

    glPopMatrix();
}

// Draw the black queen
void queen_Black_Display(double x, double y, double z)
{
    glPushMatrix();
    // Set the color of the queen
    glColor3ub(150,75,0);

    // Put the queen on the intended position
    glTranslatef(x, y, z);

    // Set the dimension of the queen
    glScalef(0.375, 0.375, 0.75);

    // Rotate the spatial pose of the queen
    glRotatef(270, 0, 1, 0);

    // Draw the solid model of the queen (Tetrahedron)
    glutSolidTetrahedron();
    glPopMatrix();
}

// Draw the realistic black queen
void queen_Realistic_Black_Display(double x, double y, double z)
{
    glPushMatrix();
    // Set the color of the queen
    glColor3ub(150,75,0);

    // Put the queen on the intended position
    glTranslatef(x, y, z);

    // Set the dimension of the queen
    glScalef(0.5, 0.5, 0.33);

    // Rotate the spatial pose of the queen model
    glRotatef(90, 1, 0, 0);

    // Load the realistic model of the queen (Using the absolute path)
    objectLoader objModel = objectLoader("./queen.obj");

    // Draw the realistic queen
    objModel.modelDrawer();

    glPopMatrix();
}

// Draw the white king
void king_White_Display(double x, double y, double z)
{
    glPushMatrix();
    // Set the color of the king
    glColor3ub(140,140,135);

    // Put the king on the intended position
    glTranslatef(x, y, z);

    // Set the dimension of the king
    glScalef(0.375, 0.375, 0.5);

    // Draw the solid model of the king (Octahedron)
    glutSolidOctahedron();
    glPopMatrix();
}

// Draw the realistic white king
void king_Realistic_White_Display(double x, double y, double z)
{
    glPushMatrix();
    // Set the color of the king
    glColor3ub(140,140,135);

    // Put the queen on the intended position
    glTranslatef(x, y, z);

    // Set the dimension of the king
    glScalef(0.5, 0.5, 0.33);

    // Rotate the spatial pose of the king
    glRotatef(90, 1, 0, 0);

    // Load the realistic model of the king (Using the absolute path)
    objectLoader objModel = objectLoader("./king.obj");

    // Draw the realistic king
    objModel.modelDrawer();

    glPopMatrix();
}

// Draw the black king
void king_Black_Display(double x, double y, double z)
{
    glPushMatrix();
    // Set the color of the king
    glColor3ub(150,75,0);

    // Put the king on the intended position
    glTranslatef(x, y, z);

    // Set the dimension of the king
    glScalef(0.375, 0.375, 0.5);

    // Draw the solid model of the king (Octahedron)
    glutSolidOctahedron();
    glPopMatrix();
}

// Draw the realistic black king
void king_Realistic_Black_Display(double x, double y, double z)
{
    glPushMatrix();
    // Set the color of the king
    glColor3ub(150,75,0);

    // Put the king on the intended position
    glTranslatef(x, y, z);

    // Set the dimension of the king
    glScalef(0.5, 0.5, 0.33);

    // Rotate the spatial pose of the king model
    glRotatef(90, 1, 0, 0);

    // Load the realistic model of the king (Using the absolute path)
    objectLoader objModel = objectLoader("./king.obj");

    // Draw the realistic king
    objModel.modelDrawer();

    glPopMatrix();
}

// The function to reset the status of the window
void reshape(int w, int h)
{
    // Acquire the current size of the window
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    // Set the projection mode
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Set the current perspective mode
    gluPerspective(60, (GLfloat)w/(GLfloat)h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Update the window
    glFlush();
    glutSwapBuffers();
}

// Display the chessboard
void chessboard_Display()
{
    // Clear the display buffer
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    // Set the normal direction of the world
    glNormal3f(0.0, 0.0, 1.0);

    // Enable the normal vector
    glEnable(GL_NORMALIZE);

    // Set the smooth model option
    glShadeModel(GL_SMOOTH);

    // Enable the depth testing
    glEnable(GL_DEPTH_TEST);

    // Set the property of the view point
    glLoadIdentity();
    gluLookAt(4, -5, 10 + view_Position_Z, 4, 4, 0, 0, 1, 0);

    // The parameters for Light 0
    // Ambient lighting
    GLfloat light0_Ambient[] = {0.2, 0.2, 0.2, 1.0};
    // Diffuse Lighting
    GLfloat light0_Diffuse[] = {0.0, 0.0, 0.0, 0.0};
    // Specular Lighting
    GLfloat light0_Specular[] = {0.0, 0.0, 0.0, 0.0};

    // Enable each kind of lighting for Light 0
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_Ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_Diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_Specular);

    // The parameters for Light 1
    // The position of Light 1
    GLfloat light1_Position[] = {5.0, 5.0, 8.0, 1.0};
    // Diffuse lighting
    GLfloat light1_Diffuse[] = {0.5, 0.5, 0.5, 1.0};
    // Specular Lighting
    GLfloat light1_Specular[] = {0.3, 0.3, 0.3, 1.0};

    // Enable each kind of lighting for Light 1
    glLightfv(GL_LIGHT1, GL_POSITION, light1_Position);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_Diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_Specular);

    // Enable the lighting
    glEnable(GL_LIGHTING);

    // If the symbol of turning on the Light0 is 1, turn on Light0
    if (enable_Light0 == 1)
    {
        glEnable(GL_LIGHT0);
    }
    // If not, turn off Light0
    else
    {
        glDisable(GL_LIGHT0);
    }

    // If the symbol of turning on the Light1 is 1, turn on Light1
    if (enable_Light1 == 1)
    {
        glEnable(GL_LIGHT1);
    }
    // If not, turn off Light0
    else
    {
        glDisable(GL_LIGHT1);
    }

    // Determine the material of the object
    // The spectular property of the material
    GLfloat mat_Spectular[] = {0.5, 0.5, 0.5, 1.0};
    // The shininess of the material
    GLfloat mat_Shininess[] = {50.0};

    // Enable the material property
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_Spectular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_Shininess);
    glEnable(GL_COLOR_MATERIAL);

    // Open the transform matrix
    glPushMatrix();
    // Set the rotation preperty
    // When pressing "r" or "R", the entire chessboard will rotate around the axis com across its center vertically
    glTranslatef(4, 4, 0);
    glRotatef(rotate_Angle, 0, 0, 1);
    glTranslatef(-4, -4, 0);

    // Display the chessboard
    chess_Board_Display();

    // Display the white king
    // If the enable_Realistic is 0, use the geometry model
    // If not, use the realistic model
    if (enable_Realistic == 0)
    {
        king_White_Display(4.5, 0.5, 0.51);
    }
    else
    {
        king_Realistic_White_Display(4.5, 0.5, 0.01);
    }

    // Display the black king
    // If the enable_Realistic is 0, use the geometry model
    // If not, use the realistic model
    if (enable_Realistic == 0)
    {
        king_Black_Display(4.5, 7.5, 0.51);
    }
    else
    {
        king_Realistic_Black_Display(4.5, 7.5, 0.01);
    }

    // Display the white queen
    // If the enable_Realistic is 0, use the geometry model
    // If not, use the realistic model
    if (enable_Realistic == 0)
    {
        queen_White_Display(3.5, 0.5, 0.5);
    }
    else
    {
        queen_Realistic_White_Display(3.5, 0.5, 0.01);
    }

    // Display the black queen
    // If the enable_Realistic is 0, use the geometry model
    // If not, use the realistic model
    if (enable_Realistic == 0)
    {
        queen_Black_Display(3.5, 7.5, 0.5);
    }
    else
    {
        queen_Realistic_Black_Display(3.5, 7.5, 0.01);
    }

    // Display the black bishop
    // If the enable_Realistic is 0, use the geometry model
    // If not, use the realistic model
    if (enable_Realistic == 0)
    {
        bishop_Black_Display(2.5, 7.5, 0.01);
        bishop_Black_Display(5.5, 7.5, 0.01);
    }
    else
    {
        bishop_Realistic_Black_Display(2.5, 7.5, 0.01);
        bishop_Realistic_Black_Display(5.5, 7.5, 0.01);
    }

    // Display the white bishop
    // If the enable_Realistic is 0, use the geometry model
    // If not, use the realistic model
    if (enable_Realistic == 0)
    {
        bishop_White_Display(2.5, 0.5, 0.01);
        bishop_White_Display(5.5, 0.5, 0.01);
    }
    else
    {
        bishop_Realistic_White_Display(2.5, 0.5, 0.01);
        bishop_Realistic_White_Display(5.5, 0.5, 0.01);
    }

    // Display the white knight
    // The location of the knight will be determined by the knight selection and movement function
    // If the enable_Realistic is 0, use the geometry model
    // If not, use the realistic model
    if (enable_Realistic == 0)
    {
        knight_White_Display(knight_Status[0][0] + 0.5, knight_Status[0][1] + 0.5, 0.51);
        knight_White_Display(knight_Status[1][0] + 0.5, knight_Status[1][1] + 0.5, 0.51);
    }
    else
    {
        knight_Realistic_White_Display(knight_Status[0][0] + 0.5, knight_Status[0][1] + 0.5, 0.01);
        knight_Realistic_White_Display(knight_Status[1][0] + 0.5, knight_Status[1][1] + 0.5, 0.01);
    }

    // Display the black knight
    // The location of the knight will be determined by the knight selection and movement function
    // If the enable_Realistic is 0, use the geometry model
    // If not, use the realistic model
    if (enable_Realistic == 0)
    {
        knight_Black_Display(knight_Status[2][0] + 0.5, knight_Status[2][1] + 0.5, 0.51);
        knight_Black_Display(knight_Status[3][0] + 0.5, knight_Status[3][1] + 0.5, 0.51);
    }
    else
    {
        knight_Realistic_Black_Display(knight_Status[2][0] + 0.5, knight_Status[2][1] + 0.5, 0.01);
        knight_Realistic_Black_Display(knight_Status[3][0] + 0.5, knight_Status[3][1] + 0.5, 0.01);
    }

    // Display the black rook
    // If the enable_Realistic is 0, use the geometry model
    // If not, use the realistic model
    if (enable_Realistic == 0)
    {
        rook_Black_Display(0.5, 7.5, 0.51);
        rook_Black_Display(7.5, 7.5, 0.51);
    }
    else
    {
        rook_Realistic_Black_Display(0.5, 7.5, 0.01);
        rook_Realistic_Black_Display(7.5, 7.5, 0.01);
    }

    // Display the white rook
    // If the enable_Realistic is 0, use the geometry model
    // If not, use the realistic model
    if (enable_Realistic == 0)
    {
        rook_White_Display(0.5, 0.5, 0.51);
        rook_White_Display(7.5, 0.5, 0.51);
    }
    else
    {
        rook_Realistic_White_Display(0.5, 0.5, 0.01);
        rook_Realistic_White_Display(7.5, 0.5, 0.01);
    }

    // Display the white pawn
    // The location of the pawn will be determined by the knight selection and movement function
    // If the enable_Realistic is 0, use the geometry model
    // If not, use the realistic model
    if (enable_Realistic == 0)
    {
        pawn_White_Display(0.5, 1.5 + pawn_Status[0], 0.51);
        pawn_White_Display(1.5, 1.5 + pawn_Status[1], 0.51);
        pawn_White_Display(2.5, 1.5 + pawn_Status[2], 0.51);
        pawn_White_Display(3.5, 1.5 + pawn_Status[3], 0.51);
        pawn_White_Display(4.5, 1.5 + pawn_Status[4], 0.51);
        pawn_White_Display(5.5, 1.5 + pawn_Status[5], 0.51);
        pawn_White_Display(6.5, 1.5 + pawn_Status[6], 0.51);
        pawn_White_Display(7.5, 1.5 + pawn_Status[7], 0.51);
    }
    else
    {
        pawn_Realistic_White_Display(0.5, 1.5 + pawn_Status[0], 0.01);
        pawn_Realistic_White_Display(1.5, 1.5 + pawn_Status[1], 0.01);
        pawn_Realistic_White_Display(2.5, 1.5 + pawn_Status[2], 0.01);
        pawn_Realistic_White_Display(3.5, 1.5 + pawn_Status[3], 0.01);
        pawn_Realistic_White_Display(4.5, 1.5 + pawn_Status[4], 0.01);
        pawn_Realistic_White_Display(5.5, 1.5 + pawn_Status[5], 0.01);
        pawn_Realistic_White_Display(6.5, 1.5 + pawn_Status[6], 0.01);
        pawn_Realistic_White_Display(7.5, 1.5 + pawn_Status[7], 0.01);
    }


    // Display the black pawn
    // The location of the pawn will be determined by the knight selection and movement function
    // If the enable_Realistic is 0, use the geometry model
    // If not, use the realistic model
    if (enable_Realistic == 0)
    {
        pawn_Black_Display(0.5, 6.5 + pawn_Status[8], 0.51);
        pawn_Black_Display(1.5, 6.5 + pawn_Status[9], 0.51);
        pawn_Black_Display(2.5, 6.5 + pawn_Status[10], 0.51);
        pawn_Black_Display(3.5, 6.5 + pawn_Status[11], 0.51);
        pawn_Black_Display(4.5, 6.5 + pawn_Status[12], 0.51);
        pawn_Black_Display(5.5, 6.5 + pawn_Status[13], 0.51);
        pawn_Black_Display(6.5, 6.5 + pawn_Status[14], 0.51);
        pawn_Black_Display(7.5, 6.5 + pawn_Status[15], 0.51);
    }
    else
    {
        pawn_Realistic_Black_Display(0.5, 6.5 + pawn_Status[8], 0.01);
        pawn_Realistic_Black_Display(1.5, 6.5 + pawn_Status[9], 0.01);
        pawn_Realistic_Black_Display(2.5, 6.5 + pawn_Status[10], 0.01);
        pawn_Realistic_Black_Display(3.5, 6.5 + pawn_Status[11], 0.01);
        pawn_Realistic_Black_Display(4.5, 6.5 + pawn_Status[12], 0.01);
        pawn_Realistic_Black_Display(5.5, 6.5 + pawn_Status[13], 0.01);
        pawn_Realistic_Black_Display(6.5, 6.5 + pawn_Status[14], 0.01);
        pawn_Realistic_Black_Display(7.5, 6.5 + pawn_Status[15], 0.01);
    }

    // Close the transform matrix
    glPopMatrix();
    glFlush();
    glutSwapBuffers();
}

// The keyboard recall function
void keyboard(unsigned char key, int x, int y)
{
    // Determine the action according to the input value
    switch(key)
    {
        case 'r':
            // Enter "r": rotate the chessboard for 10 degrees
            rotate_Angle += 10.0;
            glutPostRedisplay();
            break;
        case 'R':
            // Enter "R": rotate the chessboard for 10 degrees
            rotate_Angle += 10.0;
            glutPostRedisplay();
            break;
        case 'd':
            // Enter "d": decrease the height of the view point for 0.25m
            view_Position_Z -= 0.25;
            glutPostRedisplay();
            break;
        case 'D':
            // Enter "D": decrease the height of the view point for 0.25m
            view_Position_Z -= 0.25;
            glutPostRedisplay();
            break;
        case 'u':
            // Enter "u": increase the height of the view point for 0.25m
            view_Position_Z += 0.25;
            glutPostRedisplay();
            break;
        case 'U':
            // Enter "U": increase the height of the view point for 0.25m
            view_Position_Z += 0.25;
            glutPostRedisplay();
            break;
        case '0':
            // Enter "0": change the current status of Light0
            if (enable_Light0 == 0)
            {
                enable_Light0 = 1;
            }
            else
            {
                enable_Light0 = 0;
            }
            glutPostRedisplay();
            break;
        case '1':
            // Enter "1": change the current status of Light1
            if (enable_Light1 == 0)
            {
                enable_Light1 = 1;
            }
            else
            {
                enable_Light1 = 0;
            }
            glutPostRedisplay();
            break;
        case 'p':
            // Enter "p": move the random pawn with random movement
            pawn_Movement();
            glutPostRedisplay();
            break;
        case 'P':
            // Enter "P": move the random pawn with random movement
            pawn_Movement();
            glutPostRedisplay();
            break;
        case 'k':
            // Enter "k": move the random knight with random movement
            knight_Movement();
            glutPostRedisplay();
            break;
        case 'K':
            // Enter "K": move the random knight with random movement
            knight_Movement();
            glutPostRedisplay();
            break;
        case 'e':
            // Enter "e": change the current status whether enables the realistic model
            if (enable_Realistic == 0)
            {
                enable_Realistic = 1;
            }
            else
            {
                enable_Realistic = 0;
            }
            glutPostRedisplay();
            break;
        case 'E':
            // Enter "E": change the current status whether enables the realistic model
            if (enable_Realistic == 0)
            {
                enable_Realistic = 1;
            }
            else
            {
                enable_Realistic = 0;
            }
            glutPostRedisplay();
            break;
        default:
            // The default option: just leave the loop
            break;
    }

}

// The main funciton of the project
int main(int argc, char **argv)
{
    // General initializations
    glutInit(&argc, argv);

    // Initialize the display mode of the window
    glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);

    // Set the window size
    glutInitWindowSize(600, 600);

    // Initialize the paramenter of the chessboard
    chessboard_Init();

    // Name the window
    glutCreateWindow("Chessboard");

    // Enable the keyboard recalling function
    glutKeyboardFunc(keyboard);

    // Enable the display recalling function
    glutDisplayFunc(chessboard_Display);

    // Enable the reshape recalling function
    glutReshapeFunc(reshape);

    // Enter GLUT event processing cycle
    glutMainLoop();

    return 0;
}
