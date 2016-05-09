/***********************************************************
             CSC418, FALL 2009
 
                 penguin.cpp
                 author: Mike Pratscher
                 based on code by: Eron Steger, J. Radulovich

		Main source file for assignment 2
		Uses OpenGL, GLUT and GLUI libraries
  
    Instructions:
        Please read the assignment page to determine 
        exactly what needs to be implemented.  Then read 
        over this file and become acquainted with its 
        design. In particular, see lines marked 'README'.
		
		Be sure to also look over keyframe.h and vector.h.
		While no changes are necessary to these files, looking
		them over will allow you to better understand their
		functionality and capabilites.

        Add source code where it appears appropriate. In
        particular, see lines marked 'TODO'.

        You should not need to change the overall structure
        of the program. However it should be clear what
        your changes do, and you should use sufficient comments
        to explain your code.  While the point of the assignment
        is to draw and animate the character, you will
        also be marked based on your design.

***********************************************************/

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glui.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "keyframe.h"
#include "timer.h"
#include "vector.h"


// *************** GLOBAL VARIABLES *************************


// --------------- USER INTERFACE VARIABLES -----------------

// Window settings
int windowID;				// Glut window ID (for display)
int Win[2];					// window (x,y) size

GLUI* glui_instruction;		// Glui window with keyframe controls

// ---------------- ANIMATION VARIABLES ---------------------

// Camera settings
bool updateCamZPos = false;
int  lastX = 0;
int  lastY = 0;

GLdouble camXPos =  0.0;
GLdouble camYPos =  0.0;
GLdouble camZPos = -25.0;

const GLdouble CAMERA_FOVY = 60.0;
const GLdouble NEAR_CLIP   = 0.1;
const GLdouble FAR_CLIP    = 1000.0;

// Animation settings
int animate_mode = 0;			// 0 = no anim, 1 = animate

// Keyframe settings
const char filenameKF[] = "keyframes.txt";	// file for loading / saving keyframes

Keyframe* keyframes;			// list of keyframes

int maxValidKeyframe   = 0;		// index of max VALID keyframe (in keyframe list)
const int KEYFRAME_MIN = 0;
const int KEYFRAME_MAX = 32;	// README: specifies the max number of keyframes

// Frame settings
char filenameF[128];			// storage for frame filename

int frameNumber = 0;			// current frame being dumped
int frameToFile = 0;			// flag for dumping frames to file

const float DUMP_FRAME_PER_SEC = 24.0;		// frame rate for dumped frames
const float DUMP_SEC_PER_FRAME = 1.0 / DUMP_FRAME_PER_SEC;

// Time settings
Timer* animationTimer;
Timer* frameRateTimer;

const float TIME_MIN = 0.0;
const float TIME_MAX = 10.0;	// README: specifies the max time of the animation
const float SEC_PER_FRAME = 1.0 / 60.0;

// Joint settings

// README: This is the key data structure for
// updating keyframes in the keyframe list and
// for driving the animation.
//   i) When updating a keyframe, use the values
//      in this data structure to update the
//      appropriate keyframe in the keyframe list.
//  ii) When calculating the interpolated pose,
//      the resulting pose vector is placed into
//      this data structure. (This code is already
//      in place - see the animate() function)
// iii) When drawing the scene, use the values in
//      this data structure (which are set in the
//      animate() function as described above) to
//      specify the appropriate transformations.
Keyframe* joint_ui_data;

//character speed
const float CHAR_SPEED = 1;

//ball settings
const int BALL_AMOUNT = 10;
const float BALL_RAD = 1;

//sizes for the ramp
const float RAMP_SCALE_X =  10.0;
const float RAMP_SCALE_Y =  10.0;
const float RAMP_SCALE_Z =  10.0;

//sizes for the character
const float ROOT_SCALE_X =  0.6;
const float ROOT_SCALE_Y =  0.8;
const float ROOT_SCALE_Z =  0.35;
const float HIP_SCALE_X =  0.6;
const float HIP_SCALE_Y =  0.2;
const float HIP_SCALE_Z =  0.35;
const float ARM_SCALE_X =  0.2;
const float ARM_SCALE_Y =  0.15;
const float ARM_SCALE_Z =  0.25;
const float FOREARM_SCALE_X =  0.2;
const float FOREARM_SCALE_Y =  0.35;
const float FOREARM_SCALE_Z =  0.25;
const float LEG_SCALE_X =  0.25;
const float LEG_SCALE_Y =  0.35;
const float LEG_SCALE_Z =  0.3;
const float SHIN_SCALE_X =  0.25;
const float SHIN_SCALE_Y =  0.25;
const float SHIN_SCALE_Z =  0.3;
const float FOOT_SCALE_X =  0.25;
const float FOOT_SCALE_Y =  0.1;
const float FOOT_SCALE_Z =  0.5;
const float HEAD_SCALE_X =  0.45;
const float HEAD_SCALE_Y =  0.45;
const float HEAD_SCALE_Z =  0.45;
const float HAIR_SCALE_X =  0.7;
const float HAIR_SCALE_Y =  0.7;
const float HAIR_SCALE_Z =  0.7;

//for storing position of the character
float temp_X;
float temp_Y;
float temp_Z;
float temp_Y_rot;
float temp_Z_rot;

//ball coordinates
float ball_X[BALL_AMOUNT];
float ball_Y[BALL_AMOUNT];

//initial run
int initial = 1;

// ***********  FUNCTION HEADER DECLARATIONS ****************


// Initialization functions
void initDS();
void initGlut(int argc, char** argv);
void initGlui();
void initGl();


// Callbacks for handling events in glut
void keyDirectionPressed (unsigned char key, int x, int y);
void reshape(int w, int h);
void animate();
void display(void);


// Functions to help draw the object
Vector getInterpolatedJointDOFS(float time);
void loadKeyframesFromFileButton();
void drawVictory();
void drawRamp();
void drawHair();
void drawCube();
void startButton(int);

// Image functions
void writeFrame(char* filename, bool pgm, bool frontBuffer);


// ******************** FUNCTIONS ************************



// main() function
// Initializes the user interface (and any user variables)
// then hands over control to the event handler, which calls 
// display() whenever the GL window needs to be redrawn.
int main(int argc, char** argv)
{

    // Process program arguments
    if(argc != 3) {
        printf("Usage: demo [width] [height]\n");
        printf("Using 800x600 window by default...\n");
        Win[0] = 800;
        Win[1] = 600;
    } else {
        Win[0] = atoi(argv[1]);
        Win[1] = atoi(argv[2]);
    }


    // Initialize data structs, glut, glui, and opengl
	initDS();
    initGlut(argc, argv);
    initGlui();
    initGl();

    // Invoke the standard GLUT main event loop
    glutMainLoop();

    return 0;         // never reached
}


// Create / initialize global data structures
void initDS()
{
	keyframes = new Keyframe[KEYFRAME_MAX];
	for( int i = 0; i < KEYFRAME_MAX; i++ )
		keyframes[i].setID(i);

	animationTimer = new Timer();
	frameRateTimer = new Timer();
	joint_ui_data  = new Keyframe();
}


// Initialize glut and create a window with the specified caption 
void initGlut(int argc, char** argv)
{
	// Init GLUT
	glutInit(&argc, argv);

    // Set video mode: double-buffered, color, depth-buffered
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glEnable(GL_DEPTH_TEST);

    // Create window
    glutInitWindowPosition (0, 0);
    glutInitWindowSize(Win[0],Win[1]);
    windowID = glutCreateWindow(argv[0]);

    // Setup callback functions to handle events
    glutKeyboardFunc(keyDirectionPressed);
    glutReshapeFunc(reshape);	// Call reshape whenever window resized
    glutDisplayFunc(display);	// Call display whenever new frame needed
}

//controls
void directionPressed(int direction) {
    //space
    if (direction == 32) {
        startButton(0);
    }

    //don't do anything if the game hasn't started
    if (animate_mode == 0) {
        return;
    }

    //numpad moves and changes orientation
    if (direction == 2 || direction == 50) {
        joint_ui_data->setDOF(Keyframe::ROOT_ROTATE_Y, 0);
        joint_ui_data->setDOF(Keyframe::ROOT_TRANSLATE_Y, joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Y) - CHAR_SPEED);
    } else if (direction == 3 || direction == 51) {
        joint_ui_data->setDOF(Keyframe::ROOT_ROTATE_Y, 45);
        joint_ui_data->setDOF(Keyframe::ROOT_TRANSLATE_Y, joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Y) - CHAR_SPEED);
        joint_ui_data->setDOF(Keyframe::ROOT_TRANSLATE_X, joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_X) + CHAR_SPEED);
    } else if (direction == 6 || direction == 54) {
        joint_ui_data->setDOF(Keyframe::ROOT_ROTATE_Y, 90);
        joint_ui_data->setDOF(Keyframe::ROOT_TRANSLATE_X, joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_X) + CHAR_SPEED);
    } else if (direction == 9 || direction == 57) {
        joint_ui_data->setDOF(Keyframe::ROOT_ROTATE_Y, 135);
        joint_ui_data->setDOF(Keyframe::ROOT_TRANSLATE_Y, joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Y) + CHAR_SPEED);
        joint_ui_data->setDOF(Keyframe::ROOT_TRANSLATE_X, joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_X) + CHAR_SPEED);
    } else if (direction == 8 || direction == 56) {
        joint_ui_data->setDOF(Keyframe::ROOT_ROTATE_Y, 180);
        joint_ui_data->setDOF(Keyframe::ROOT_TRANSLATE_Y, joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Y) + CHAR_SPEED);
    } else if (direction == 7 || direction == 55) {
        joint_ui_data->setDOF(Keyframe::ROOT_ROTATE_Y, 225);
        joint_ui_data->setDOF(Keyframe::ROOT_TRANSLATE_Y, joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Y) + CHAR_SPEED);
        joint_ui_data->setDOF(Keyframe::ROOT_TRANSLATE_X, joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_X) - CHAR_SPEED);
    } else if (direction == 4 || direction == 52) {
        joint_ui_data->setDOF(Keyframe::ROOT_ROTATE_Y, 270);
        joint_ui_data->setDOF(Keyframe::ROOT_TRANSLATE_X, joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_X) - CHAR_SPEED);
    } else if (direction == 1 || direction == 49) {
        joint_ui_data->setDOF(Keyframe::ROOT_ROTATE_Y, 315);
        joint_ui_data->setDOF(Keyframe::ROOT_TRANSLATE_Y, joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Y) - CHAR_SPEED);
        joint_ui_data->setDOF(Keyframe::ROOT_TRANSLATE_X, joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_X) - CHAR_SPEED);
    }

    //edge collision
    if (joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_X) < -10) {
        joint_ui_data->setDOF(Keyframe::ROOT_TRANSLATE_X, -10);
    }
    if (joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_X) > 10) {
        joint_ui_data->setDOF(Keyframe::ROOT_TRANSLATE_X, 10);
    }
    if (joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Y) < -10) {
        joint_ui_data->setDOF(Keyframe::ROOT_TRANSLATE_Y, -10);
    }
    if (joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Y) > 10) {
        joint_ui_data->setDOF(Keyframe::ROOT_TRANSLATE_Y, 10);
    }
    glutPostRedisplay();
}

//passes keyboard input
void keyDirectionPressed (unsigned char key, int x, int y) {
    directionPressed(key);
}  

//load walking animation
void loadKeyframesFromFile()
{
	// Open file for reading
	FILE* file = fopen(filenameKF, "r");
	if( file == NULL )
	{
		return;
	}

	// Read in maxValidKeyframe first
	fscanf(file, "%d", &maxValidKeyframe);

	// Now read in all keyframes in the format:
	//    id
	//    time
	//    DOFs
	//
	for( int i = 0; i <= maxValidKeyframe; i++ )
	{
		fscanf(file, "%d", keyframes[i].getIDPtr());
		fscanf(file, "%f", keyframes[i].getTimePtr());

		for( int j = 0; j < Keyframe::NUM_JOINT_ENUM; j++ )
			fscanf(file, "%f", keyframes[i].getDOFPtr(j));
	}

	// Close file
	fclose(file);
}

//initiate game state
void animateBegin()
{
    frameRateTimer->reset();
    animationTimer->reset();

    initial = 0;
    animate_mode = 1;
    joint_ui_data->setDOF(Keyframe::ROOT_TRANSLATE_X, 0);
    joint_ui_data->setDOF(Keyframe::ROOT_TRANSLATE_Y, -10);

    srand(time(NULL));

    camZPos = -25.0;

    joint_ui_data->setDOF(Keyframe::ROOT_ROTATE_Z, 0);
    for(int i = 0; i < BALL_AMOUNT; i++) {
        ball_X[i] = rand() % 20 - 10;
        ball_Y[i] = rand() % 18 - 8;
    }

    GLUI_Master.set_glutIdleFunc(animate);
}

// Quits if game is in progress, otherwise, start the game
void startButton(int)
{
    if (animate_mode == 0) {
            loadKeyframesFromFile();
            animateBegin();
            animate_mode = 1;
    } else {
        exit(0);
    }
}

// Initialize GLUI
void initGlui()
{
    GLUI_Panel* glui_panel;

    GLUI_Master.set_glutIdleFunc(NULL);

    //instructions
	glui_instruction = GLUI_Master.create_glui("Instructions", 0, Win[0] + 50, 0);
    glui_panel = glui_instruction->add_panel("Instructions", GLUI_PANEL_NONE);
    glui_instruction->add_statictext_to_panel(glui_panel, "Dodge the balls and get to the top to secure a victory! Use your numpad to move!" );
    glui_instruction->add_statictext_to_panel(glui_panel, "Click the game window and press Space to begin!" );
    glui_instruction->add_statictext_to_panel(glui_panel, "Move your player with the numberpad: Left (4), Right (6), Up (8), Down(2)." );
    glui_instruction->add_statictext_to_panel(glui_panel, "Good Luck !" );

	/*
    //button controls
    glui_instruction->add_button_to_panel(glui_panel, "7", 7, directionPressed);
	glui_instruction->add_button_to_panel(glui_panel, "4", 4, directionPressed);
	glui_instruction->add_button_to_panel(glui_panel, "1", 1, directionPressed);
	glui_instruction->add_column_to_panel(glui_panel, false);
	glui_instruction->add_button_to_panel(glui_panel, "8", 8, directionPressed);
	glui_instruction->add_button_to_panel(glui_panel, "Start/Exit", 0, startButton);
	glui_instruction->add_button_to_panel(glui_panel, "2", 2, directionPressed);
	glui_instruction->add_column_to_panel(glui_panel, false);
	glui_instruction->add_button_to_panel(glui_panel, "9", 9, directionPressed);
	glui_instruction->add_button_to_panel(glui_panel, "6", 6, directionPressed);
	glui_instruction->add_button_to_panel(glui_panel, "3", 3, directionPressed);*/

    glui_instruction->set_main_gfx_window(windowID);

	GLUI_Master.set_glutKeyboardFunc(keyDirectionPressed);
}


void initGl(void)
{
    //Set background
    glClearColor(0.5f,0.5f,0.5f,1.0f);
}


// Calculates the interpolated joint DOF vector
// using Catmull-Rom interpolation of the keyframes
Vector getInterpolatedJointDOFS(float time)
{
	// Need to find the keyframes between which
	// the supplied time lies.
	// At the end of the loop we have:
	//    keyframes[i-1].getTime() < time <= keyframes[i].getTime()
	//
	int i = 0;
	while( i <= maxValidKeyframe && keyframes[i].getTime() < time )
		i++;

	// If time is before or at first defined keyframe, then
	// just use first keyframe pose
	if( i == 0 )
		return keyframes[0].getDOFVector();

	// If time is beyond last defined keyframe, then just
	// use last keyframe pose
	if( i > maxValidKeyframe )
		return keyframes[maxValidKeyframe].getDOFVector();

	// Need to normalize time to (0, 1]
	float alpha = (time - keyframes[i-1].getTime()) / (keyframes[i].getTime() - keyframes[i-1].getTime());

	// Get appropriate data points
	// for computing the interpolation
	Vector p1 = keyframes[i-1].getDOFVector();
	Vector p2 = keyframes[i].getDOFVector();

    //use Catmull-Rom if there are more than three points
    if ( i - 2 >= 0 && i+1 <=maxValidKeyframe) {
        Vector p0 = keyframes[i-2].getDOFVector();
    	Vector p3 = keyframes[i+1].getDOFVector();

        return ((p1 * 2.0) + (((p0 * -1.0) + p2) * alpha) + (((p0 * 2.0) - (p1 * 5.0) + (p2 * 4.0) - p3) * alpha * alpha) + (((p0 * -1.0) + (p1 * 3.0) - (p2 * 3.0) + p3) * alpha * alpha * alpha)) * 0.5;
    } else {
	    // Return the linearly interpolated Vector
    	return p1 * (1-alpha) + p2 * alpha;
    }
}


// Callback idle function for animating the scene
void animate()
{
	// Only update if enough time has passed
	// (This locks the display to a certain frame rate rather
	//  than updating as fast as possible. The effect is that
	//  the animation should run at about the same rate
	//  whether being run on a fast machine or slow machine)
	if( frameRateTimer->elapsed() > SEC_PER_FRAME )
	{

        //check for collision between balls and player
        for(int i = 0; i < BALL_AMOUNT; i++) {
            if (ball_X[i] + BALL_RAD  + HIP_SCALE_X > joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_X) && 
                ball_X[i] - BALL_RAD - HIP_SCALE_X < joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_X) &&
                ball_Y[i] + BALL_RAD + HIP_SCALE_X > joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Y) &&
                ball_Y[i] - BALL_RAD - HIP_SCALE_X < joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Y)) {
                joint_ui_data->setDOF(Keyframe::ROOT_ROTATE_Z, 90);
                animate_mode = 0;
                GLUI_Master.set_glutIdleFunc(NULL);
            }
            
            //advance balls every frame
            ball_Y[i] = ball_Y[i] - 0.1;
            if (ball_Y[i] < -10) {
                ball_X[i] = rand() % 20 - 10;
                ball_Y[i] = 10;
            }
        }

        //move camera to the offscreen win screen
        if (joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Y) >= 10) {
            camZPos = -900.0;
            animate_mode = 0;
        }
		// Tell glut window to update itself. This will cause the display()
		// callback to be called, which renders the object 
		glutSetWindow(windowID);
		glutPostRedisplay();

		// Restart the frame rate timer
		// for the next frame
		frameRateTimer->reset();
	}
}


// Handles the window being resized by updating the viewport
// and projection matrices
void reshape(int w, int h)
{
	// Update internal variables and OpenGL viewport
	Win[0] = w;
	Win[1] = h;
	glViewport(0, 0, (GLsizei)Win[0], (GLsizei)Win[1]);

    // Setup projection matrix for new window
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(CAMERA_FOVY, (GLdouble)Win[0]/(GLdouble)Win[1], NEAR_CLIP, FAR_CLIP);
}



// display callback
void display(void)
{
    //test for depth
    glEnable(GL_DEPTH_TEST);

    // Clear the screen with the background colour
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  

    // Setup the model-view transformation matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	// Specify camera transformation
	glTranslatef(camXPos, camYPos, camZPos);

	// Get the time for the current animation step, if necessary
	if( animate_mode )
	{
		float curTime = animationTimer->elapsed();

		if( curTime >= keyframes[maxValidKeyframe].getTime() )
		{
			// Restart the animation
			animationTimer->reset();
			curTime = animationTimer->elapsed();
		}

        //maintains character position/rotation while animating limbs rotations
        temp_X = joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_X);
        temp_Y = joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Y);
        temp_Z = joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Z);
        temp_Y_rot = joint_ui_data->getDOF(Keyframe::ROOT_ROTATE_Y);
        temp_Z_rot = joint_ui_data->getDOF(Keyframe::ROOT_ROTATE_Z);

		joint_ui_data->setDOFVector( getInterpolatedJointDOFS(curTime) );

        joint_ui_data->setDOF(Keyframe::ROOT_TRANSLATE_X, temp_X);
        joint_ui_data->setDOF(Keyframe::ROOT_TRANSLATE_Y, temp_Y);
        joint_ui_data->setDOF(Keyframe::ROOT_TRANSLATE_Z, temp_Z);
        joint_ui_data->setDOF(Keyframe::ROOT_ROTATE_Y, temp_Y_rot);
        joint_ui_data->setDOF(Keyframe::ROOT_ROTATE_Z, temp_Z_rot);
	}

    //Begin drawing the graphical objects 
	glPushMatrix();

        //draws the ramp
        glScalef(RAMP_SCALE_X, RAMP_SCALE_Y, RAMP_SCALE_Z);
        drawRamp();
        glScalef(1/RAMP_SCALE_X, 1/RAMP_SCALE_Y, 1/RAMP_SCALE_Z);

	glPopMatrix();

    //don't draw anything else if this is the first run
    if(initial == 1) {
        glFlush();
        glutSwapBuffers();
        return;
    }
    glPushMatrix();
        //draws victory screen
        drawVictory();
    glPopMatrix();

    //draws each ball
    for (int i = 0; i < BALL_AMOUNT; i++) {
	    glPushMatrix();
		    glTranslatef(ball_X[i], ball_Y[i] + BALL_RAD, -ball_Y[i]);
            glColor3f(1,0,0);
            glutSolidSphere(BALL_RAD,25,25);
	    glPopMatrix();
    }
    glPushMatrix();

		// setup transformation for character
		glTranslatef(joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_X),
                     joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Y),
                    -joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Y));
        glRotatef(joint_ui_data->getDOF(Keyframe::ROOT_ROTATE_Y), 0.0, 1.0, 0.0);
        glRotatef(joint_ui_data->getDOF(Keyframe::ROOT_ROTATE_Z), 0.0, 0.0, 1.0);


        glTranslatef(0, ROOT_SCALE_Y *2+ HIP_SCALE_Y*2 + SHIN_SCALE_Y*2 + FOOT_SCALE_Y*2,0);
		//upper body
		glColor3f(1.0, 1.0, 1.0);
        glScalef(ROOT_SCALE_X, ROOT_SCALE_Y, ROOT_SCALE_Z);
		drawCube();
        glScalef(1/ROOT_SCALE_X, 1/ROOT_SCALE_Y, 1/ROOT_SCALE_Z);

        glPushMatrix();
            //Head
            glTranslatef(0.0, ROOT_SCALE_Y + HEAD_SCALE_Y, 0.0);
            glRotatef(joint_ui_data->getDOF(Keyframe::HEAD), 0.0, 1.0, 0.0);
            glColor3f(1.0, 0.85, 0.8);
            glScalef(HEAD_SCALE_X, HEAD_SCALE_Y, HEAD_SCALE_Z);
		    drawCube();
            glColor3f(0.3, 0.3, 0.0);
            drawHair();
            glScalef(1/HEAD_SCALE_X, 1/HEAD_SCALE_Y, 1/HEAD_SCALE_Z);

        glPopMatrix();

        glPushMatrix();
            //Left Arm
            glTranslatef(ROOT_SCALE_X + ARM_SCALE_X, 3*(ROOT_SCALE_Y + HIP_SCALE_Y)/4, 0.0);
            glRotatef(joint_ui_data->getDOF(Keyframe::L_SHOULDER_ROLL), 1.0, 0.0, 0.0);
            glRotatef(joint_ui_data->getDOF(Keyframe::L_SHOULDER_PITCH), 0.0, 1.0, 0.0);
            glRotatef(joint_ui_data->getDOF(Keyframe::L_SHOULDER_YAW), 0.0, 0.0, 1.0);
            glTranslatef(0.0, -ARM_SCALE_Y, 0.0);
            glColor3f(1.0, 1.0, 1.0);
            glScalef(ARM_SCALE_X, ARM_SCALE_Y, ARM_SCALE_Z);
		    drawCube();
            glScalef(1/ARM_SCALE_X, 1/ARM_SCALE_Y, 1/ARM_SCALE_Z);

            glTranslatef(0.0, -ARM_SCALE_Y * 2, 0.0);
            glColor3f(1.0, 0.85, 0.8);
            glScalef(ARM_SCALE_X, ARM_SCALE_Y, ARM_SCALE_Z);
		    drawCube();
            glScalef(1/ARM_SCALE_X, 1/ARM_SCALE_Y, 1/ARM_SCALE_Z);            

            //Left Forearm
            glTranslatef(0.0, -ARM_SCALE_Y, 0.0);
            glRotatef(joint_ui_data->getDOF(Keyframe::L_ELBOW), -1.0, 0.0, 0.0);
            glTranslatef(0.0, -FOREARM_SCALE_Y, 0.0);
            glColor3f(1.0, 0.85, 0.8);
            glScalef(FOREARM_SCALE_X, FOREARM_SCALE_Y, FOREARM_SCALE_Z);
		    drawCube();
            glScalef(1/FOREARM_SCALE_X, 1/FOREARM_SCALE_Y, 1/FOREARM_SCALE_Z);            
        glPopMatrix();

        glPushMatrix();
            //Right Arm
            glTranslatef(-ROOT_SCALE_X - ARM_SCALE_X, 3*(ROOT_SCALE_Y + HIP_SCALE_Y)/4, 0.0);
            glRotatef(joint_ui_data->getDOF(Keyframe::R_SHOULDER_ROLL), 1.0, 0.0, 0.0);
            glRotatef(joint_ui_data->getDOF(Keyframe::R_SHOULDER_PITCH), 0.0, 1.0, 0.0);
            glRotatef(-joint_ui_data->getDOF(Keyframe::R_SHOULDER_YAW), 0.0, 0.0, 1.0);
            glTranslatef(0.0, -ARM_SCALE_Y, 0.0);
            glColor3f(1.0, 1.0, 1.0);
            glScalef(ARM_SCALE_X, ARM_SCALE_Y, ARM_SCALE_Z);
		    drawCube();
            glScalef(1/ARM_SCALE_X, 1/ARM_SCALE_Y, 1/ARM_SCALE_Z);

            glTranslatef(0.0, -ARM_SCALE_Y * 2, 0.0);
            glColor3f(1.0, 0.85, 0.8);
            glScalef(ARM_SCALE_X, ARM_SCALE_Y, ARM_SCALE_Z);
		    drawCube();
            glScalef(1/ARM_SCALE_X, 1/ARM_SCALE_Y, 1/ARM_SCALE_Z);    

            //Right Forearm
            glTranslatef(0.0, -ARM_SCALE_Y, 0.0);
            glRotatef(joint_ui_data->getDOF(Keyframe::R_ELBOW), -1.0, 0.0, 0.0);
            glTranslatef(0.0, -FOREARM_SCALE_Y, 0.0);
            glColor3f(1.0, 0.85, 0.8);
            glScalef(FOREARM_SCALE_X, FOREARM_SCALE_Y, FOREARM_SCALE_Z);
		    drawCube();
            glScalef(1/FOREARM_SCALE_X, 1/FOREARM_SCALE_Y, 1/FOREARM_SCALE_Z);
        glPopMatrix();

        //lower body
        glTranslatef(0.0, -ROOT_SCALE_Y - HIP_SCALE_Y, 0.0);
        glColor3f(0.4, 0.4, 1.0);
        glScalef(HIP_SCALE_X, HIP_SCALE_Y, HIP_SCALE_Z);
	    drawCube();
        glScalef(1/HIP_SCALE_X, 1/HIP_SCALE_Y, 1/HIP_SCALE_Z); 

        glPushMatrix();
            //Left Leg
            glTranslatef(HIP_SCALE_X/2, - HIP_SCALE_Y, 0.0);
            glRotatef(joint_ui_data->getDOF(Keyframe::L_HIP_ROLL), 1.0, 0.0, 0.0);
            glRotatef(joint_ui_data->getDOF(Keyframe::L_HIP_PITCH), 0.0, 1.0, 0.0);
            glRotatef(joint_ui_data->getDOF(Keyframe::L_HIP_YAW), 0.0, 0.0, 1.0);
            glTranslatef(0.0, -LEG_SCALE_Y, 0.0);
            glColor3f(0.4, 0.4, 1.0);
            glScalef(LEG_SCALE_X, LEG_SCALE_Y, LEG_SCALE_Z);
		    drawCube();
            glScalef(1/LEG_SCALE_X, 1/LEG_SCALE_Y, 1/LEG_SCALE_Z);

            //Left Shin
            glTranslatef(0.0, -LEG_SCALE_Y, 0.0);
            glRotatef(joint_ui_data->getDOF(Keyframe::L_KNEE), 1.0, 0.0, 0.0);
            glTranslatef(0.0, -SHIN_SCALE_Y, 0.0);
            glColor3f(0.4, 0.4, 1.0);
            glScalef(SHIN_SCALE_X, SHIN_SCALE_Y, SHIN_SCALE_Z);
		    drawCube();
            glScalef(1/SHIN_SCALE_X, 1/SHIN_SCALE_Y, 1/SHIN_SCALE_Z);

            //Left Foot
            glTranslatef(0.0, -SHIN_SCALE_Y - FOOT_SCALE_Y, (FOOT_SCALE_Z - SHIN_SCALE_Z));
            glColor3f(0.0, 0.0, 0.0);
            glScalef(FOOT_SCALE_X, FOOT_SCALE_Y, FOOT_SCALE_Z);
		    drawCube();
            glScalef(1/FOOT_SCALE_X, 1/FOOT_SCALE_Y, 1/FOOT_SCALE_Z);
        glPopMatrix();

        glPushMatrix();
            //Right Leg
            glTranslatef(-HIP_SCALE_X/2, - HIP_SCALE_Y, 0.0);
            glRotatef(joint_ui_data->getDOF(Keyframe::R_HIP_ROLL), 1.0, 0.0, 0.0);
            glRotatef(joint_ui_data->getDOF(Keyframe::R_HIP_PITCH), 0.0, 1.0, 0.0);
            glRotatef(-joint_ui_data->getDOF(Keyframe::R_HIP_YAW), 0.0, 0.0, 1.0);
            glTranslatef(0.0, -LEG_SCALE_Y, 0.0);
            glColor3f(0.4, 0.4, 1.0);
            glScalef(LEG_SCALE_X, LEG_SCALE_Y, LEG_SCALE_Z);
		    drawCube();
            glScalef(1/LEG_SCALE_X, 1/LEG_SCALE_Y, 1/LEG_SCALE_Z);

            //Right Shin
            glTranslatef(0.0, -LEG_SCALE_Y, 0.0);
            glRotatef(joint_ui_data->getDOF(Keyframe::R_KNEE), 1.0, 0.0, 0.0);
            glTranslatef(0.0, -SHIN_SCALE_Y, 0.0);
            glColor3f(0.4, 0.4, 1.0);
            glScalef(SHIN_SCALE_X, SHIN_SCALE_Y, SHIN_SCALE_Z);
		    drawCube();
            glScalef(1/SHIN_SCALE_X, 1/SHIN_SCALE_Y, 1/SHIN_SCALE_Z);   

            //Right Foot
            glTranslatef(0.0, -SHIN_SCALE_Y - FOOT_SCALE_Y, (FOOT_SCALE_Z - SHIN_SCALE_Z));
            glColor3f(0.0, 0.0, 0.0);
            glScalef(FOOT_SCALE_X, FOOT_SCALE_Y, FOOT_SCALE_Z);
		    drawCube();
            glScalef(1/FOOT_SCALE_X, 1/FOOT_SCALE_Y, 1/FOOT_SCALE_Z);         
        glPopMatrix();

	glPopMatrix();
	//End graphical object creation

    // Execute any GL functions that are in the queue just to be safe
    glFlush();
                        
    // Now, show the frame buffer that we just drew into.
    // (this prevents flickering).
    glutSwapBuffers();
}

//Handles end game screen in the event of a win (victory)
void drawVictory()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glColor3f(1,1,1);

    //draws white background
    glTranslatef(0,0, 100);
	glBegin(GL_QUADS);
        glNormal3d(0,0,1);
		glVertex3f(-1000.0, -1000.0, 0.0);
		glVertex3f( 1000.0, -1000.0, 0.0);
		glVertex3f( 1000.0,  1000.0, 0.0);
		glVertex3f(-1000.0,  1000.0, 0.0);
	glEnd();

    glColor3f(0,0,0);
    glTranslatef(-400,0, 100);

    //Send out game winning message 
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, 'V');
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, 'I');
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, 'C');
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, 'T');
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, 'O');
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, 'R');
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, 'Y');
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, '!');
}


//Handles the plane sketching of ramp 
void drawRamp() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_QUADS);

	    // draw front and the top of the face
        glNormal3d(0,1,1);
	    glVertex3f(-1.0, -1.0, 1.0);
	    glVertex3f( 1.0, -1.0, 1.0);
	    glVertex3f( 1.0,  1.0, -1.0);
	    glVertex3f(-1.0,  1.0, -1.0);

	    // draw back face
        glNormal3d(0,0,-1);
	    glVertex3f( 1.0, -1.0, -1.0);
	    glVertex3f(-1.0, -1.0, -1.0);
	    glVertex3f(-1.0,  1.0, -1.0);
	    glVertex3f( 1.0,  1.0, -1.0);

	    // draw bottom
        glNormal3d(0,-1,0);
	    glVertex3f(-1.0, -1.0, -1.0);
	    glVertex3f( 1.0, -1.0, -1.0);
	    glVertex3f( 1.0, -1.0,  1.0);
	    glVertex3f(-1.0, -1.0,  1.0);
    glEnd();

    glBegin(GL_POLYGON);
        // draw left face
        glNormal3d(-1,0,0);
	    glVertex3f(-1.0, -1.0, -1.0);
	    glVertex3f(-1.0, -1.0,  1.0);
	    glVertex3f(-1.0,  1.0, -1.0);
    glEnd();

    glBegin(GL_POLYGON);
	    // draw right face
        glNormal3d(1,0,0);
	    glVertex3f( 1.0, -1.0,  1.0);
	    glVertex3f( 1.0, -1.0, -1.0);
	    glVertex3f( 1.0,  1.0,  -1.0);
    glEnd();
}

//Handles hair sketch (of player)
void drawHair() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_QUADS);
        // draw front face
        glNormal3d(0,0,1);
		glVertex3f(-1.3,  0.8, 1.3);
		glVertex3f( 1.3,  0.8, 1.3);
		glVertex3f( 1.3,  1.3, 1.3);
		glVertex3f(-1.3,  1.3, 1.3);

		// draw back face
        glNormal3d(0,0,-1);
		glVertex3f( 1.3,  0.8, -1.3);
		glVertex3f(-1.3,  0.8, -1.3);
		glVertex3f(-1.3,  1.3, -1.3);
		glVertex3f( 1.3,  1.3, -1.3);

		// draw left face
        glNormal3d(-1,0,0);
		glVertex3f(-1.3,  0.8, -1.3);
		glVertex3f(-1.3,  0.8,  1.3);
		glVertex3f(-1.3,  1.3,  1.3);
		glVertex3f(-1.3,  1.3, -1.3);

		// draw right face
        glNormal3d(1,0,0);
		glVertex3f( 1.3,  0.8,  1.3);
		glVertex3f( 1.3,  0.8, -1.3);
		glVertex3f( 1.3,  1.3, -1.3);
		glVertex3f( 1.3,  1.3,  1.3);

		// draw top
        glNormal3d(0,1,0);
		glVertex3f(-1.3,  1.3,  1.3);
		glVertex3f( 1.3,  1.3,  1.3);
		glVertex3f( 1.3,  1.3, -1.3);
		glVertex3f(-1.3,  1.3, -1.3);


        // draw front/bottom face
        glNormal3d(0,-1,0.8);
		glVertex3f(-1.3, -1.3, -1.3);
		glVertex3f( 1.3, -1.3, -1.3);
		glVertex3f( 1.3,  0.8,  1.3);
		glVertex3f(-1.3,  0.8,  1.3);

		// draw back face
        glNormal3d(0,0,-1);
		glVertex3f( 1.3, -1.3, -1.3);
		glVertex3f(-1.3, -1.3, -1.3);
		glVertex3f(-1.3,  0.8, -1.3);
		glVertex3f( 1.3,  0.8, -1.3);
    glEnd();

    glBegin(GL_POLYGON);
		// draw left face
        glNormal3d(-1,0,0);
		glVertex3f(-1.3, -1.3, -1.3);
		glVertex3f(-1.3,  0.8,  1.3);
		glVertex3f(-1.3,  0.8, -1.3);
    glEnd();

    glBegin(GL_POLYGON);
		// draw right face
        glNormal3d(1,0,0);
		glVertex3f( 1.3, -1.3, -1.3);
		glVertex3f( 1.3,  0.8, -1.3);
		glVertex3f( 1.3,  0.8,  1.3);
    glEnd();
}

// Draw a unit cube, centered at the current location
void drawCube()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   
	glBegin(GL_QUADS);
		// draw front face
        glNormal3d(0,0,1);
		glVertex3f(-1.0, -1.0, 1.0);
		glVertex3f( 1.0, -1.0, 1.0);
		glVertex3f( 1.0,  1.0, 1.0);
		glVertex3f(-1.0,  1.0, 1.0);

		// draw back face
        glNormal3d(0,0,-1);
		glVertex3f( 1.0, -1.0, -1.0);
		glVertex3f(-1.0, -1.0, -1.0);
		glVertex3f(-1.0,  1.0, -1.0);
		glVertex3f( 1.0,  1.0, -1.0);

		// draw left face
        glNormal3d(-1,0,0);
		glVertex3f(-1.0, -1.0, -1.0);
		glVertex3f(-1.0, -1.0,  1.0);
		glVertex3f(-1.0,  1.0,  1.0);
		glVertex3f(-1.0,  1.0, -1.0);

		// draw right face
        glNormal3d(1,0,0);
		glVertex3f( 1.0, -1.0,  1.0);
		glVertex3f( 1.0, -1.0, -1.0);
		glVertex3f( 1.0,  1.0, -1.0);
		glVertex3f( 1.0,  1.0,  1.0);

		// draw top
        glNormal3d(0,1,0);
		glVertex3f(-1.0,  1.0,  1.0);
		glVertex3f( 1.0,  1.0,  1.0);
		glVertex3f( 1.0,  1.0, -1.0);
		glVertex3f(-1.0,  1.0, -1.0);

		// draw bottom
        glNormal3d(0,-1,0);
		glVertex3f(-1.0, -1.0, -1.0);
		glVertex3f( 1.0, -1.0, -1.0);
		glVertex3f( 1.0, -1.0,  1.0);
		glVertex3f(-1.0, -1.0,  1.0);
	glEnd();
}
