#include <stdio.h>
#include <GLUT/glut.h>
#include <iostream>
#include <chrono>
#include <thread>

#include "include/cpu.hpp"
#include "include/Interrupt/InterruptController.hpp"
#include "include/Peripheral/ppu.hpp"
#include "include/Peripheral/lcdcStatus.hpp"
#include "include/Peripheral/bootRom.hpp"
#include "include/Peripheral/soundController.hpp"
#include "include/memoryBus.hpp"
#include "include/Peripheral/socRAM.hpp"
#include "include/Peripheral/timer.hpp"
#include "include/Peripheral/serial.hpp"
#include "include/Cartridge/cartridgeBuilder.hpp"
#include "include/Peripheral/controller.hpp"

// Display size
#define SCREEN_HEIGHT 144
#define SCREEN_WIDTH 160

int modifier = 2;

uint8_t frameBuffer[V_RES][H_RES];

// Window size
int display_width = SCREEN_WIDTH * modifier;
int display_height = SCREEN_HEIGHT * modifier;

void display();
void reshape_window(GLsizei w, GLsizei h);
void keyboardUp(unsigned char key, int x, int y);
void keyboardDown(unsigned char key, int x, int y);
void specialUp(int key, int x, int y);
void specialDown(int key, int x, int y);
void setupTexture();

InterruptController interruptController;
SocRam socRam;
LcdcStatus lcdStatus(interruptController);
MemoryBus memoryBus;
PictureProcessingUnit ppu(interruptController, lcdStatus);
Cpu cpu(interruptController, memoryBus);
SoundController apu;
Timer timer(interruptController);
Serial serial;
Controller controller(interruptController);


int main(int argc, char **argv) 
{		
	if(argc < 2)
	{
		printf("Usage: myChip8.exe chip8application\n\n");
		return 1;
	}
	
	auto cartridge = CartridgeBuilder::openROM(argv[1]);
	
	memoryBus.registerPeripheral(&socRam);
    memoryBus.registerPeripheral(&ppu);
    memoryBus.registerPeripheral(&apu);
    memoryBus.registerPeripheral(&lcdStatus);
	memoryBus.registerPeripheral(cartridge.get());
	memoryBus.registerPeripheral(&timer);
	memoryBus.registerPeripheral(&interruptController);
	memoryBus.registerPeripheral(&serial);
	memoryBus.registerPeripheral(&controller);

	ppu.registerDmaHandler([](const uint16_t _address, uint8_t& value)
	{
		uint16_t address = value * 0x100;
		for (int i = 0; i < OAM_SIZE; i++)
		{
			uint8_t ramValue = memoryBus.readMemoryBus(address + i);
			ppu.objectAttributeMemory()[i] = ramValue;
		}
	});
    
	// Setup OpenGL
	glutInit(&argc, argv);          
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(display_width, display_height);
    glutInitWindowPosition(320, 320);
	glutCreateWindow("myChip8 by Laurence Muller");
	
	glutDisplayFunc(display);
	glutIdleFunc(display);
    glutReshapeFunc(reshape_window);

	glutKeyboardFunc(keyboardDown);
	glutSpecialFunc(specialDown);

	glutSpecialUpFunc(specialUp);
	glutKeyboardUpFunc(keyboardUp); 


	setupTexture();			


	glutMainLoop(); 

	return 0;
}

// Setup Texture
void setupTexture()
{
	// Create a texture 
	glTexImage2D(GL_TEXTURE_2D, 0, 3, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)ppu.screenData);

	// Set up the texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); 

	// Enable textures
	glEnable(GL_TEXTURE_2D);
}

void updateTexture(PictureProcessingUnit& ppu)
{	
	// Update Texture
	glTexSubImage2D(GL_TEXTURE_2D, 0 ,0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)ppu.screenData);

	glBegin( GL_QUADS );
		glTexCoord2d(0.0, 0.0);		glVertex2d(0.0,			  0.0);
		glTexCoord2d(1.0, 0.0); 	glVertex2d(display_width, 0.0);
		glTexCoord2d(1.0, 1.0); 	glVertex2d(display_width, display_height);
		glTexCoord2d(0.0, 1.0); 	glVertex2d(0.0,			  display_height);
	glEnd();
}

// Old gfx code
void drawPixel(int x, int y)
{
	glBegin(GL_QUADS);
		glVertex3f((x * modifier) + 0.0f,     (y * modifier) + 0.0f,	 0.0f);
		glVertex3f((x * modifier) + 0.0f,     (y * modifier) + modifier, 0.0f);
		glVertex3f((x * modifier) + modifier, (y * modifier) + modifier, 0.0f);
		glVertex3f((x * modifier) + modifier, (y * modifier) + 0.0f,	 0.0f);
	glEnd();
}

void display()
{
    const int MAXCYCLES = 69905 ;
    int cyclesThisUpdate = 0 ;

    while (cyclesThisUpdate < MAXCYCLES)
    {
        uint8_t ticks = cpu.step();
        ppu.tick(ticks);
		timer.step(ticks);
        cyclesThisUpdate += ticks;
    }
        
    glClear(GL_COLOR_BUFFER_BIT);
    
    updateTexture(ppu);

	glutSwapBuffers();    
}

void reshape_window(GLsizei w, GLsizei h)
{
	glClearColor(0.0f, 0.0f, 0.5f, 0.0f);
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, h, 0);        
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, w, h);

	// Resize quad
	display_width = w;
	display_height = h;
}

void keyboardDown(unsigned char key, int x, int y)
{
	if(key == 27)    // esc
		exit(0);

	switch(key)
	{
		case 'a' : controller.buttonPressed(Button::BUTTON_A); break;
		case 's' : controller.buttonPressed(Button::BUTTON_B); break;
		case 'z' : controller.buttonPressed(Button::BUTTON_SELECT); break;
		case 'x' : controller.buttonPressed(Button::BUTTON_START); break;
	}
}

void specialDown(int key, int x, int y)
{
	switch(key)
	{
		case GLUT_KEY_UP: controller.dpadPressed(Dpad::DPAD_UP); break;
		case GLUT_KEY_DOWN: controller.dpadPressed(Dpad::DPAD_DOWN); break;
		case GLUT_KEY_LEFT: controller.dpadPressed(Dpad::DPAD_LEFT); break;
		case GLUT_KEY_RIGHT: controller.dpadPressed(Dpad::DPAD_RIGHT); break;
	}
}

void specialUp(int key, int x, int y)
{
	switch(key)
	{
		case GLUT_KEY_UP: controller.dpadReleased(Dpad::DPAD_UP); break;
		case GLUT_KEY_DOWN: controller.dpadReleased(Dpad::DPAD_DOWN); break;
		case GLUT_KEY_LEFT: controller.dpadReleased(Dpad::DPAD_LEFT); break;
		case GLUT_KEY_RIGHT: controller.dpadReleased(Dpad::DPAD_RIGHT); break;
	}
}

void keyboardUp(unsigned char key, int x, int y)
{
	switch(key)
	{
		case 'a' : controller.buttonReleased(Button::BUTTON_A); break;
		case 's' : controller.buttonReleased(Button::BUTTON_B); break;
		case 'z' : controller.buttonReleased(Button::BUTTON_SELECT); break;
		case 'x' : controller.buttonReleased(Button::BUTTON_START); break;
	}
}
