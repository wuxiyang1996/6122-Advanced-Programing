/*

Author: <Xiyang Wu>
Class: ECE6122
Last Date Modified: <12/03/2019>

Description:

Final Project: GaTech Buzzy Bowl
C++ IDE: CLion 2019.02

*/

#include <iostream>
#include <math.h> // The math operation header
#include <mpi.h> // The MPI header
#include <iomanip>
#include <cstdlib> // Standard definitions
#include <chrono>  // The system clock
#include <thread> // The thread header
#include <GL/glut.h> // Include the GLUT header file
#include <stdio.h>
#include <stdlib.h> // Standard definitions
#include <time.h> // Include the timer as the seed for the random number
#include <string> // The string header
#include <vector> // The vector header
#include <fstream> // The file I/O header
#include <sstream> // The file I/O header
#include "ECE_Bitmap.h" // The bmp image loader

// Using the std namespace
using namespace std;

// Global variable: The mass of each UAV (Kg)
double uavMass = 1.0;

// Global variable: The total amplitude of the force
double uavForce = 20.0;

// Global variable: The amplitude of the gravity (Constant)
double gravity = 10.0;

// Global variable: The amplitude of the force to compensate the gravity (Constant)
double compensateGravity = 10.0;

// Global variable: Simulation time interval (0.1 Seconds / Step)
double timeInteval = 0.1;

// Global variable: Size of the bounding box
double boundingBoxSize = 1.0;

// Global variable: Maximum velocity in Step 1
double maxVelocityStep1 = 2.0;

// Global variable: Ideal velocity in Step 2 (Constant)
// To avoid the given UAV flying off the surface of the sphere
double idealVelocityStep2 = 3.0;

// Global variable: The radius of the sphere
double sphereRadius = 10.0;

// Global variable: The timer in thread 0
int timerThread0 = 0;

// Global variable: The color switching flag
int colorSwitchingFlag = -1;

// Global variable: The color value
int colorValue = 255;

// Global variable: The timer that begins when all the UAVS are flying around the sphere
int timerStatus2 = 0;

// The coordination of the center of the sphere (Target point)
// X coordination
double xTargetPoint = 0.0;
// Y coordination
double yTargetPoint = 0.0;
// Z coordination
double zTargetPoint = 50.0;

// The initial x coordination of each UAV
double xInitUav[15] = {-24.4, -24.4, -24.4, -24.4, -24.4, 0, 0, 0, 0, 0, 24.4, 24.4, 24.4, 24.4, 24.4};
// The initial y coordination of each UAV
double yInitUav[15] = {-45.72, -22.86, 0, 22.86, 45.72, -45.72, -22.86, 0, 22.86, 45.72, -45.72, -22.86, 0, 22.86, 45.72};

// The texture array
GLuint texture[1];

// The struct to store the data of the image imported
struct Image
{
    unsigned long sizeX;
    unsigned long sizeY;
    char *data;
};
typedef struct Image Image;

// Initialize the structure to store the image
BMP inBitmap;

// Display the football field
void footballFieldShow()
{
    glPushMatrix();
    // Set the color of the rectangle football field
    glColor3ub(0, 255, 0);
    
    // Enable the bitmap texture
    glBindTexture(GL_TEXTURE_2D, texture[0]);

    // Paint the rectangle (Football Field) according to the actual length
    // Bind the bitmap on the corner of the football field
    // The image will be resized slight to satisfy the requirement
    glBegin(GL_POLYGON);
    glTexCoord2f(0, 1);
    glVertex3f(1.1 * 24.4, 1.1 * 54, 0);
    glTexCoord2f(1, 1);
    glVertex3f(1.1 * 24.4, -1.1 * 54, 0);
    glTexCoord2f(1, 0);
    glVertex3f(-1.1 * 24.4, -1.1 * 54, 0);
    glTexCoord2f(0, 0);
    glVertex3f(-1.1 * 24.4, 1.1 * 54, 0);
    glEnd();

    glPopMatrix();
}

// Display the virtual sphere
void virtualSphereShow()
{
    glPushMatrix();
    // Set the color of the virtual sphere
    glColor3ub(255, 255, 255);

    // Put the virtual sphere on the intended position
    glTranslatef(0, 0, 50);

    // Paint the virtual sphere
    glutWireSphere(10.0, 30, 30);
    glPopMatrix();
}

// Display the UAV model
void uavShow(double uavX, double uavY, double uavZ, int colorValue)
{
    glPushMatrix();
    // Set the color of the UAV (Dynamic)
    glColor3ub(0, colorValue, colorValue);

    // Put the UAV on the intended position
    glTranslatef(uavX, uavY, uavZ);

    // Set the dimension of the pawn
    glScalef(0.3, 0.3, 0.3);

    // Paint the dodecahedron
    glutSolidDodecahedron();
    glPopMatrix();
}

// The function to reset the status of the window
void reshape(int windowWidth, int windowHeight)
{
    // Acquire the current size of the window
    glViewport(0, 0, (GLsizei)windowWidth, (GLsizei)windowHeight);

    // Set the projection mode
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Set the current perspective mode
    gluPerspective(60, (GLfloat)windowWidth/(GLfloat)windowHeight, 1.0, 400.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Update the window
    glFlush();
    glutSwapBuffers();
}

// This function intends to update and boardcast the current postion, velocity of each UAV, using the force acquired in the previous moment
// The input variance is the parameter of the given UAV in the previous moment, the output one is the updated parameter
double* uavPositionUpdate(double uavParameter[])
{
    // Return the updated parameter
    double* newUavParameter = new double[14];

    // The initial velocity in the x direction
    double vxInit = uavParameter[8];
    // The initial velocity in the y direction
    double vyInit = uavParameter[9];
    // The initial velocity in the z direction
    double vzInit = uavParameter[10];

    // The initial position in the x direction
    double xxInit = uavParameter[2];
    // The initial position in the y direction
    double xyInit = uavParameter[3];
    // The initial position in the z direction
    double xzInit = uavParameter[4];

    // The ideal force in the x direction at each moment
    double fxIdeal = uavParameter[5];
    // The ideal force in the y direction at each moment
    double fyIdeal = uavParameter[6];
    // The ideal force in the z direction at each moment
    double fzIdeal = uavParameter[7];

    // The acceleration value in the x direction at each moment
    double ax = fxIdeal / uavMass;
    // The acceleration value in the y direction at each moment
    double ay = fyIdeal / uavMass;
    // The acceleration value in the z direction at each moment
    double az = fzIdeal / uavMass;

    // Velocity update
    // X direction
    double vx = vxInit + ax * timeInteval;
    // Y direction
    double vy = vyInit + ay * timeInteval;
    // Z direction
    double vz = vzInit + az * timeInteval;

    // Position update
    // X direction
    double xx = xxInit + vxInit * timeInteval + 0.5 * ax * timeInteval * timeInteval;
    // Y direction
    double xy = xyInit + vyInit * timeInteval + 0.5 * ay * timeInteval * timeInteval;
    // Z direction
    double xz = xzInit + vzInit * timeInteval + 0.5 * az * timeInteval * timeInteval;

    // Update distance between the given UAV and the center of the sphere (Target point)
    double currentDistance = sqrt(pow((xTargetPoint - xx), 2) + pow((yTargetPoint - xy), 2) + pow((zTargetPoint - xz), 2));

    // The rank of UAV
    newUavParameter[0] = uavParameter[0];
    // The status of UAV
    newUavParameter[1] = uavParameter[1];
    // The updated position of UAV
    newUavParameter[2] = xx;
    newUavParameter[3] = xy;
    newUavParameter[4] = xz;
    // The force of UAV
    newUavParameter[5] = uavParameter[5];
    newUavParameter[6] = uavParameter[6];
    newUavParameter[7] = uavParameter[7];
    // The updated velocity of UAV
    newUavParameter[8] = vx;
    newUavParameter[9] = vy;
    newUavParameter[10] = vz;
    // The derivative and integral error
    newUavParameter[11] = uavParameter[11];
    newUavParameter[12] = uavParameter[12];
    // The updated distance between the given UAV and the center of the sphere (Target point)
    newUavParameter[13] = currentDistance;

    return newUavParameter;
};

// The collision function: determine the velocity after the collision between two UAVs
// The collision condition corresponds to the size of the UAV's bounding box
// When the collision happens between two UAVs, the velocity of the corresponding UAVs will be switched
double* uavCollisionJudgement(double uavParameter[])
{
    // Initialize the array for the updated parameter
    static double newUavParameter[240];

    // Transfer the value in the UAV_Para to the new_UAV_Parameter
    for (int i = 0; i < 240; i++)
    {
        newUavParameter[i] = uavParameter[i];
    }

    // Search all the UAVs available
    for (int i = 0; i < 15; i++)
    {
        // Within a certain moment, the collision condition between any two UAVs will only be judged once
        for (int j = i; i < 15; i++)
        {
            // If the distances between the given two UAVs in all the three direction are smaller than the 2.01m (The size of two bounding box and the extra 1cm), the collision will happen
            if ((fabs(uavParameter[14 * i + 2] - uavParameter[14 * j + 2]) <= 1.01)&&(fabs(uavParameter[14 * i + 3] - uavParameter[14 * j + 3]) <= 1.01)&&(fabs(uavParameter[14 * i + 4] - uavParameter[14 * j + 4]) <= 1.01))
            {
                // Initialize the data transferring variable
                double temp1, temp2, temp3;
                // Switch the velocity component of x
                temp1 = newUavParameter[14 * i + 8];
                newUavParameter[14 * i + 8] = newUavParameter[14 * j + 8];
                newUavParameter[14 * j + 8] = temp1;

                // Switch the velocity component of y
                temp2 = newUavParameter[14 * i + 9];
                newUavParameter[14 * i + 9] = newUavParameter[14 * j + 9];
                newUavParameter[14 * j + 9] = temp2;

                // Switch the velocity component of z
                temp3 = newUavParameter[14 * i + 10];
                newUavParameter[14 * i + 10] = newUavParameter[14 * j + 10];
                newUavParameter[14 * j + 10] = temp3;
            }
        }
    }

    // Return the update UAV parameters
    return newUavParameter;
}

// This function intends to simulate the UAV's transferring flight process after 5s (Step 1)
// The output of this function will update the force of the UAV
// The thrust of the given UAV will generate 10N to compensate the gravity and use 5N as the propulsion force
// When the velocity of the UAV is close to 2m/s, the propulsion force will be 0N
double* uavTransferringForceGenerator(double uavParameter[])
{
    // Initialize the array for the updated parameter for the given UAV
    static double newUavParameter[14];

    // The propulsion force (Fixed)
    double propulsionForce = 5.0;

    // Initialize the updating array for the given UAV
    for (int i = 0; i < 14; i++)
    {
        newUavParameter[i] = uavParameter[i];
    }

    // Calculate the velocity vector
    double vectorX = xTargetPoint - uavParameter[2];
    double vectorY = yTargetPoint - uavParameter[3];
    double vectorZ = zTargetPoint - uavParameter[4];

    // Normalize the velocity vector
    double vectorAmplitude = sqrt(pow(vectorX, 2) + pow(vectorY, 2) + pow(vectorZ, 2));

    // When the status of the given UAV is 0, use this function to simulate its transferring flight
    if (uavParameter[1] == 0)
    {
        // Calculate the acceleration vector
        double acceVectorX = vectorX / vectorAmplitude * propulsionForce / uavMass;
        double acceVectorY = vectorY / vectorAmplitude * propulsionForce / uavMass;
        double acceVectorZ = vectorZ / vectorAmplitude * propulsionForce / uavMass;

        // Calculate the intended velocity if using the current acceleration
        double vxUpdated = uavParameter[8] + acceVectorX * timeInteval;
        double vyUpdated = uavParameter[9] + acceVectorY * timeInteval;
        double vzUpdated = uavParameter[10] + acceVectorZ * timeInteval;

        // Calculate the amplitude of the current velocity
        double velocityAmplitude = sqrt(pow(vxUpdated, 2) + pow(vyUpdated, 2) + pow(vzUpdated, 2));

        // If the amplitude of the intended velocity does not exceed the limitation, keep the acceleration acquired
        if (velocityAmplitude <= maxVelocityStep1)
        {
            newUavParameter[5] = vectorX / vectorAmplitude * propulsionForce / uavMass;
            newUavParameter[6] = vectorY / vectorAmplitude * propulsionForce / uavMass;
            newUavParameter[7] = vectorZ / vectorAmplitude * propulsionForce / uavMass;
        }
        else
        {
            // If not, fill the gap between the current velocity and the limitation in that direction, set the status as 1 (Drifting)
            newUavParameter[5] = (maxVelocityStep1 * vectorX / vectorAmplitude - newUavParameter[5]) * uavMass;
            newUavParameter[6] = (maxVelocityStep1 * vectorY / vectorAmplitude - newUavParameter[6]) * uavMass;
            newUavParameter[7] = (maxVelocityStep1 * vectorZ / vectorAmplitude - newUavParameter[7]) * uavMass;
            newUavParameter[1] = 1;
        }

    }
    else
    {
        // If the status of the given UAV is 1, set the propulsion force as 0
        newUavParameter[5] = 0;
        newUavParameter[6] = 0;
        newUavParameter[7] = 0;
        // If the UAV enters the surface of the virtual sphere, set its status as 2
        if ((vectorAmplitude <= 10.0)&&(newUavParameter[1] != 2))
        {
            newUavParameter[1] = 2;
        }
    }

    // Compensate the effect of the gravity
    newUavParameter[7] = newUavParameter[7] - gravity + compensateGravity;

    // Return the update UAV parameters
    return newUavParameter;
}

// This function intends to simulate the UAV's transferring flight process after 5s (Step 1)
// The output of this function will update the force of the UAV
// The variable of UAV_Para is the extracted parameter for the given UAV, the rank is the seed for random number
double* uavSurroundingForceGenerator(double uavParameter[])
{
    // Initialize the array for the updated parameter for the given UAV
    static double newUavParameter[14];

    // Initialize the updating array for the given UAV
    for (int i = 0; i < 14; i++)
    {
        newUavParameter[i] = uavParameter[i];
    }

    // The triangular velocity changing rate
    double triangularChangingRate = 1.0;

    // Calculate the normal vector
    double normalX = xTargetPoint - uavParameter[2];
    double normalY = yTargetPoint - uavParameter[3];
    double normalZ = zTargetPoint - uavParameter[4];

    // Calculate the amplitude of the normal vector for normalization
    double normalAmplitude = sqrt(pow(normalX, 2) + pow(normalY, 2) + pow(normalZ, 2));

    // Determine the direction and amplitude of the triangular force
    // Calculate the initial vector
    double triangularRawX = rand() / double(RAND_MAX);
    double triangularRawY = rand() / double(RAND_MAX);
    double triangularRawZ = rand() / double(RAND_MAX);

    // Calculate the amplitude of the initial vector
    double triangularRawAmplitude = sqrt(pow(triangularRawX, 2) + pow(triangularRawY, 2) + pow(triangularRawZ, 2));

    // Avoid the circumstance that generated vector is parallel with the the normal vector
    // If so, re-calculate the triangular raw vector
    if ((triangularRawX * normalX + triangularRawY * normalY + triangularRawZ * normalZ) == (normalAmplitude * triangularRawAmplitude))
    {
        triangularRawX = rand() / double(RAND_MAX);
        triangularRawY = rand() / double(RAND_MAX);
        triangularRawZ = rand() / double(RAND_MAX);

        // Calculate the amplitude of the initial vector
        triangularRawAmplitude = sqrt(pow(triangularRawX, 2) + pow(triangularRawY, 2) + pow(triangularRawZ, 2));
    }

    // Calculate the dot product of the previous two vectors
    double vectorDotProduct = triangularRawX * normalX + triangularRawY * normalY + triangularRawZ * normalZ;

    // Calculate the factor for normalization
    double factorNorm = vectorDotProduct / (triangularRawAmplitude * normalAmplitude);

    // Calculate the raw triangular vector
    double triangularX = triangularRawX - factorNorm * normalX / normalAmplitude * triangularRawAmplitude;
    double triangularY = triangularRawY - factorNorm * normalY / normalAmplitude * triangularRawAmplitude;
    double triangularZ = triangularRawZ - factorNorm * normalZ / normalAmplitude * triangularRawAmplitude;

    // Calculate the amplitude of the triangular vector for normalization
    double triangularAmplitude = sqrt(pow(triangularX, 2) + pow(triangularY, 2) + pow(triangularZ, 2));

    // Normalize the triangular vector
    double normTriangularX = triangularX / triangularAmplitude;
    double normTriangularY = triangularY / triangularAmplitude;
    double normTriangularZ = triangularZ / triangularAmplitude;

    // Calculate the amplitude of the updated speed
    double updatedTriangularX = uavParameter[8] + normTriangularX * triangularChangingRate * timeInteval;
    double updatedTriangularY = uavParameter[9] + normTriangularY * triangularChangingRate * timeInteval;
    double updatedTriangularZ = uavParameter[10] + normTriangularZ * triangularChangingRate * timeInteval;

    // Calculate the amplitude of the updated velocity
    double updatedTriangularAmplitude = sqrt(pow(updatedTriangularX, 2) + pow(updatedTriangularY, 2) + pow(updatedTriangularZ, 2));

    double triangularForceX = (updatedTriangularX / updatedTriangularAmplitude * idealVelocityStep2 - uavParameter[8]) / timeInteval * uavMass;
    double triangularForceY = (updatedTriangularY / updatedTriangularAmplitude * idealVelocityStep2 - uavParameter[9]) / timeInteval * uavMass;
    double triangularForceZ = (updatedTriangularZ / updatedTriangularAmplitude * idealVelocityStep2 - uavParameter[10]) / timeInteval * uavMass;

    // Determine the normal force (Centripetal Force)
    // Using the current status
    // Calculate the dot product between the current velocity vector and the normal vector
    double velocityNormalDotProduct = uavParameter[8] * normalX + uavParameter[9] * normalY + uavParameter[10] * normalZ;

    // Calculate the amplitude of the current velocity vector
    double currentVelocityAmplitude = sqrt(pow(uavParameter[8], 2) + pow(uavParameter[9], 2) + pow(uavParameter[10], 2));

    // Calculate the angle between the current velocity vector and the normal vector
    double angleVelocityNormal = velocityNormalDotProduct / (currentVelocityAmplitude * normalAmplitude);

    // Calculate the current triangular velocity in each direction
    double currentTriangularVelocityX = uavParameter[8] - angleVelocityNormal * normalX / normalAmplitude * currentVelocityAmplitude;
    double currentTriangularVelocityY = uavParameter[9] - angleVelocityNormal * normalY / normalAmplitude * currentVelocityAmplitude;
    double currentTriangularVelocityZ = uavParameter[10] - angleVelocityNormal * normalZ / normalAmplitude * currentVelocityAmplitude;

    // Calculate the amplitude of current triangular velocity
    double currentTriangularVelocity = sqrt(pow(currentTriangularVelocityX, 2) + pow(currentTriangularVelocityY, 2) + pow(currentTriangularVelocityZ, 2));

    // Calculate the current Centripetal Force
    double currentCentripetalForce = uavMass * currentTriangularVelocity * currentTriangularVelocity / sphereRadius;

    //double current_Centripetal_Force = 1;

    // Calculate the current error between the UAV's actual and ideal position
    double disError = (normalAmplitude - sphereRadius) / sphereRadius;
    // Calculate the error derivative between the UAV's actual and ideal position
    double disErrorDiff = (disError - uavParameter[11]) / timeInteval;
    // Calculate the error integral between the UAV's actual and ideal position
    double disErrorIntegral = uavParameter[12] + disError * timeInteval;

    // Define the factor for the PID controller
    double kp = 500.0;
    double kd = 100.0;
    double ki = 50;

    // Calculate the compensation force on the normal direction of the sphere's surface
    double compensateForceX = currentCentripetalForce * (disErrorIntegral * ki + disError * kp + disErrorDiff * kd) * normalX / normalAmplitude;
    double compensateForceY = currentCentripetalForce * (disErrorIntegral * ki + disError * kp + disErrorDiff * kd) * normalY / normalAmplitude;
    double compensateForceZ = currentCentripetalForce * (disErrorIntegral * ki + disError * kp + disErrorDiff * kd) * normalZ / normalAmplitude;

    // Update the force for the UAVs that are flying around the sphere
    newUavParameter[5] = compensateForceX + triangularForceX;
    newUavParameter[6] = compensateForceY + triangularForceY;
    newUavParameter[7] = compensateForceZ + triangularForceZ;

    // Store the current error between the UAV's actual and ideal position
    newUavParameter[11] = disError;
    // Store the current error sum between the UAV's actual and ideal position
    newUavParameter[12] = disErrorIntegral;

    // Compensate the effect of the gravity
    newUavParameter[7] = newUavParameter[7] - gravity + compensateGravity;

    // Return the update UAV parameters
    return newUavParameter;
}

// Display the football field
void footballFieldDisplay()
{
    // Initialzie the broadcast message
    // The size of the message array is determined by the number of UAV
    double msgBcast[240];

    // Initialize the send buffer (The kinematic parameter of the background)
    double backgroundUpdate[14];

    // Search all the UAVs, receive the status update message from the UAVs
    for (int i = 1; i < 16; i++)
    {
        // Receive the status update message from the given UAV
        MPI_Recv(backgroundUpdate,         // The receive buffer
                 14,                  // The size of the receive  buffer
                 MPI_DOUBLE,                // The type of the receive  message
                 i,                         // The source thread
                 99,                   // The message index
                 MPI_COMM_WORLD,            // MPI_Comm
                 MPI_STATUS_IGNORE   // MPI_Status
        );

        // Compose the message into the global message buffer
        for (int j = 0; j < 14; j++)
        {
            msgBcast[14 * i + j] = backgroundUpdate[j];
        }
    }

    // Change the color of UAV every 20 time steps (2.0s)
    if ((timerThread0 > 0) && (timerThread0 % 20 == 0))
    {
        // If the color value reaches 255, set the sign of color oscillation as -1.0
        if (colorValue == 255)
        {
            colorSwitchingFlag = -1.0;
        }
        else
        {
            // If the color value reaches 128, set the sign of color oscillation as 1.0
            if (colorValue == 128)
            {
                colorSwitchingFlag = 1.0;
            }
        }

        // Update the value of the UAV's color
        colorValue = colorValue + colorSwitchingFlag;
    }

    // Determine whether the collision between different UAVs in the given group will happen
    double* collisionPtr = uavCollisionJudgement(msgBcast);

    // Update the message array
    for (int i = 0; i < 240; i++)
    {
        msgBcast[i] = *(collisionPtr + i);
    }

    // Clear the display buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set the normal direction of the world
    glNormal3f(0.0, 0.0, 1.0);

    // Set the smooth model option
    glShadeModel(GL_SMOOTH);

    // Enable the depth testing
    glEnable(GL_DEPTH_TEST);

    // Set the property of the view point
    glLoadIdentity();
    gluLookAt(100, 0, 100, 0, 0, 25, -1, 0, 1);

    // The parameters for Light 0
    // The position of Light 0
    GLfloat light0Position[] = {100.0, 0.0, 100.0, 1.0};
    // Diffuse lighting
    GLfloat light0Diffuse[] = {0.5, 0.5, 0.5, 1.0};
    // Specular Lighting
    GLfloat light0Specular[] = {0.3, 0.3, 0.3, 1.0};

    // Enable each kind of lighting for Light 0
    glLightfv(GL_LIGHT0, GL_POSITION, light0Position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0Diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0Specular);

    // Enable the lighting
    glEnable(GL_LIGHTING);

    // Enable the Light0
    glEnable(GL_LIGHT0);

    // Enable the color material of the UAV
    glEnable(GL_COLOR_MATERIAL);

    // Import the image from the file
    inBitmap.read("./AmFBfield.bmp");

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Create Textures
    glGenTextures(1, texture);

    // Setup the football field texture
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    //scale linearly when image bigger than texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //scale linearly when image smalled than texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Import the image to the texture
    glTexImage2D(GL_TEXTURE_2D, 0, 3, inBitmap.bmp_info_header.width, inBitmap.bmp_info_header.height, 0,
                 GL_BGR_EXT, GL_UNSIGNED_BYTE, &inBitmap.data[0]);

    // Enable the texture
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glEnable(GL_TEXTURE_2D);

    glPushMatrix();

    // Display the football field
    footballFieldShow();

    // Clear the bitmap
    glBindTexture(GL_TEXTURE_2D, 0);

    // Display the virtual sphere
    virtualSphereShow();

    // Display all 15 UAVs
    for (int i = 1; i < 16; i++)
    {
        uavShow(msgBcast[14 * i + 2], msgBcast[14 * i + 3], msgBcast[14 * i + 4], colorValue);
    }

    glPopMatrix();

    // Update the window
    glFlush();
    glutSwapBuffers();

    // Send the given message to all the threads
    for (int i = 1; i < 16; i++)
    {
        MPI_Send(msgBcast,      // The send buffer
                 240,      // The size of the send message
                 MPI_DOUBLE,     // The type of the send message
                 i,              // The target thread
                 98,        // The index of the message
                 MPI_COMM_WORLD  // MPI_Comm
        );
    }

    // Update the time in thread 0
    timerThread0++;
}

// Timer function, called whenever timer fires
void timerFunction(int id)
{
    glutPostRedisplay();
    glutTimerFunc(100, timerFunction, 0);
}

// The funciton of the OpenGL part of the project
int mainOpenGL(int argc, char **argv)
{
    // General initializations
    glutInit(&argc, argv);

    // Initialize the display mode of the window
    glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);

    // Set the window size
    glutInitWindowSize(400, 400);

    // Name the window
    glutCreateWindow("Football Field");

    // Enable the reshape recalling function
    glutReshapeFunc(reshape);

    // Enable the display recalling function
    glutDisplayFunc(footballFieldDisplay);

    glutTimerFunc(100, timerFunction, 0);;

    // Enter GLUT event processing cycle
    glutMainLoop();

    return 0;
}


// The main function
int main(int argc, char** argv)
{
    // Initialize the broadcast message
    // The size of the message array is determined by the number of UAV
    double msgBcast[240];

    // Initialize the random number seed
    srand((unsigned)time(0));

    // Initialize the MPI environment
    int rc = MPI_Init(&argc, &argv);

    // If the program fails to open the MPI environment, send the error message and exit the program
    if (rc != MPI_SUCCESS)
    {
        printf("Error starting MPI program. Terminating.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    // Get the number of processes
    int world_Size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_Size);

    // Get the rank of teh processor
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Set the initial status of each UAV in Process 0
    if (rank == 0)
    {
        // Read the initial position and velocity parameters from the file
        for (int i = 1; i < 16; i++)
        {
            // Initialize the RankID
            msgBcast[14 * i + 0] = i;
            // The initial status of each UAV, which is Active (0)
            // Define: Transferring accelerating: 0, Transferring drifting: 1, Surrounding the surface 2
            msgBcast[14 * i + 1] = 0;
            // The initial position of each UAV
            msgBcast[14 * i + 2] = xInitUav[i - 1];
            msgBcast[14 * i + 3] = yInitUav[i - 1];
            msgBcast[14 * i + 4] = 0.5;
            // The initial force of each UAV, which equals to zero
            msgBcast[14 * i + 5] = 0;
            msgBcast[14 * i + 6] = 0;
            msgBcast[14 * i + 7] = 0;
            // The initial speed of each UAV in each direction
            msgBcast[14 * i + 8] = 0;
            msgBcast[14 * i + 9] = 0;
            msgBcast[14 * i + 10] = 0;

            // The initial distance error between the actual and ideal distance
            msgBcast[14 * i + 11] = 0;
            // The initial sum of the distance error between the actual and ideal distance
            msgBcast[14 * i + 12] = 0;
            // The initial distance between the UAV and the target point
            msgBcast[14 * i + 13] = sqrt(pow(xTargetPoint - msgBcast[14 * i + 2], 2) + pow(yTargetPoint - msgBcast[14 * i + 3], 2) + pow(zTargetPoint - msgBcast[14 * i + 4], 2));
        }
    }

    // Broadcast the data imported from the file
    MPI_Bcast(msgBcast, // The broadcast buffer
              240, // The number of the broadcast message
              MPI_DOUBLE, // Broadcast Type
              0, // Root node
              MPI_COMM_WORLD // MPI_Comm
    );

    //Wait until other thread has been finished
    MPI_Barrier(MPI_COMM_WORLD);

    // The root thread, which simulates the environment
    if (rank == 0)
    {
        mainOpenGL(argc, argv);
    }
    // The rest of the threads will simulate the movement of each UAV
    else
    {
        // Set the entire simulation time is 120s, while the UAVs will stop for 5s, fly towards the center of the sphere for about 40s, and fly around the sphere for 60s
        // The main loop to simulate the UAV and the environment
        // time is the period of simulation
        for (int time = 0; time <= 1200; time++)
        {
            // Initialize the send buffer (The kinematic parameter of the UAV)
            double uavCurrentParameter[14];

            // If current time is larger than 0, begin to receive the global status message
            if (time > 0)
            {
                // Receive the message sent by the main thread
                MPI_Recv(msgBcast,               // The receive buffer
                         240,               // The size of the receive message
                         MPI_DOUBLE,              // The type of the receive message
                         0,                // The source thread
                         98,                 // The index of the message
                         MPI_COMM_WORLD,          // MPI Comm
                         MPI_STATUS_IGNORE // MPI Status
                );
            }

            // Extract the parameter for the given UAV
            for (int i = 0; i < 14; i++)
            {
                uavCurrentParameter[i] = msgBcast[14 * rank + i];
            }

            // If current time is less than 5s, the UAVs will be silent.
            // If not, the UAVs will directly fly towards the target point
            if (time >= 50)
            {
                // Update the current status of the given UAV (Position, Velocity)
                double *uavPtr = uavPositionUpdate(uavCurrentParameter);

                // Update the send buffer
                for (int i = 0; i < 14; i++)
                {
                    uavCurrentParameter[i] = *(uavPtr + i);
                }

                // Update the current status of the given UAV (Thrust for the next moment)
                // Check the current status of the given UAV
                double *uavPtr1;
                if (uavCurrentParameter[1] == 2)
                {
                    // If the given UAV is in status 2 (Flying around the sphere), use the UAV_Surrounding_Force_Generator
                    uavPtr1 = uavSurroundingForceGenerator(uavCurrentParameter);
                }
                else
                {
                    // If not, use the UAV_Position_Update
                    uavPtr1 = uavTransferringForceGenerator(uavCurrentParameter);
                }

                // Update the send buffer
                for (int i = 0; i < 14; i++)
                {
                    uavCurrentParameter[i] = *(uavPtr1 + i);
                }

                // For thread 1, search the status for all the UAVs, if all the status are 2, enable the time_Status_2
                if (rank == 1)
                {
                    // Count the number of the UAVs that reaches the surface of the sphere
                    int countStatus2 = 0;
                    for (int i = 1; i < 16; i++)
                    {
                        if (msgBcast[14 * i + 1] == 2)
                        {
                            countStatus2++;
                        }
                    }

                    // If the result of the counter equals to the actual number of the UAVs, enable the timer
                    if (countStatus2 == 15)
                    {
                        timerStatus2++;
                    }
                }
            }

            // Send the given message to the main thread
            MPI_Send(uavCurrentParameter,    // The send message buffer
                     14,             // The size of the send message
                     MPI_DOUBLE,           // The type of the send message
                     0,               // The target thread
                     99,              // The message index
                     MPI_COMM_WORLD        // The MPI Comm
            );
            
             // For thread 1, if all the UAVs have flied around the sphere for 60s, exit the loop
            if (rank == 1)
            {
                if (timerStatus2 == 600)
                {
                    break;
                }
            }
        }

        // Exit the program peacefully
        exit(0);
    }

    // If the simulation time exceeds the given time length, exit the loop and close the mpi environment
    // Close the MPI environment
    MPI_Finalize();
}