/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	This class encapsulates the game, is in charge of creating the game, getting the user input, process the update and render.
*/

#ifndef GAME_H
#define GAME_H

#include "includes.h"
#include "camera.h"
#include "utils.h"
#include "shader.h"
#include "texture.h"
#include "audio.h"

class Game
{
public:
	static Game* instance;

	//window
	SDL_Window* window;
	int window_width;
	int window_height;

	//some globals
	long frame;
    float time;
	float elapsed_time;
	int fps;
	bool must_exit;
	
	//shaders
	Shader* shader;
	Shader* gui_shader;
	Shader* a_shader;

	//GUIs
	Matrix44 quadModel;
	Texture* menu_inicial;
	Texture* menu;
	Texture* menu1;

	int level;
	int numPistas;

	// Audio
	HSAMPLE hSample1;
	HSAMPLE hSample2;
	Audio* audio;

	//some vars
	Camera* camera; //our global camera
	bool mouse_locked; //tells if the mouse is locked (not seen)

	Game( int window_width, int window_height, SDL_Window* window );

	//main functions
	void render( void );
	void update( double dt );

	//events
	void onKeyDown( SDL_KeyboardEvent event );
	void onKeyUp(SDL_KeyboardEvent event);
	void onMouseButtonDown( SDL_MouseButtonEvent event );
	void onMouseButtonUp(SDL_MouseButtonEvent event);
	void onMouseWheel(SDL_MouseWheelEvent event);
	void onGamepadButtonDown(SDL_JoyButtonEvent event);
	void onGamepadButtonUp(SDL_JoyButtonEvent event);
	void onResize(int width, int height);
	//void MultiPassRender(const Matrix44 model, Mesh* mesh, std::vector<LightEntity*> l, Camera* camera);
	void RenderGUI(Texture* tex, Shader* a_shader, float centerx, float centery, float w, float h, Vector4 tex_range, Vector4 color, bool flipYV);

	bool RenderButton(Texture* tex, Shader* a_shader, float centerx, float centery, float w, float h, Vector4 tex_range, Vector4 color, bool flipYV);
	void RenderAllGUIs();
	void IntroGUI();
	void InfoGUI();
	void LevelsGUI();
};


#endif 