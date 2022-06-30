#include "stage.h"
#include "game.h"
#include "entity.h"

Stage* GetStage(STAGE_ID id, std::vector<Stage*> stages) {
	return stages[(int)id];
}

void SetStage(STAGE_ID* currentStage, STAGE_ID id) { // Como cambiar la stage
	*currentStage = id;
}

void IntroStage::Render(void)
{
	Game::instance->RenderAllGUIs();
	Game::instance->IntroGUI();
	drawText(100, 100, "WESTERN DETECTIVE", Vector3(0, 0, 0), 5);
}
void InfoStage::Render(void)
{
	Game::instance->RenderAllGUIs();
	Game::instance->InfoGUI();
	drawText(20, 120, "Eres un sheriff en el antiguo oeste y se ha cometido un crimen.", Vector3(0, 0, 0), 2);
	drawText(20, 140, "Tu mision es encontrar las pistas entre los diferentes niveles hasta", Vector3(0, 0, 0), 2);
	drawText(20, 160, "capturar al culpable.", Vector3(0, 0, 0), 2);
	drawText(20, 220, "BUENA SUERTE", Vector3(0, 0, 0), 3);
}
void TutorialStage::Render(void)
{
	drawText(20, 60, "COMO JUGAR:", Vector3(0, 0, 0), 3);
	drawText(20, 90, "Cambiar modo de camara con TAB", Vector3(0, 0, 0), 2);
	drawText(20, 110, "Moverse con WSAD, en caso de camara libre, mover camara con flechas", Vector3(0, 0, 0), 2);
	drawText(20, 130, "Coger pistas con F", Vector3(0, 0, 0), 2);
	drawText(20, 200, "PULSA SPACE PARA EMPEZAR", Vector3(0, 0, 0), 3);
}
void Level1Stage::Render(void)
{
	drawText(50, 100, "LEVEL1", Vector3(Game::instance->window_width / 2.0, 1.0, Game::instance->window_height / 2.0), 5);
	drawText(Game::instance->window_width - 300, 140, "F para coger pista", Vector3(0, 0, 0), 2);
	drawText(Game::instance->window_width - 300, 50, "Encuentra 3 pistas:", Vector3(1, 1, 1), 2);
	drawText(Game::instance->window_width - 300, 70, "- Pila de huesos", Vector3(1, 1, 1), 2);
	drawText(Game::instance->window_width - 300, 90, "- Cadaver", Vector3(1, 1, 1), 2);
	drawText(Game::instance->window_width - 300, 110, "- Cuchillo", Vector3(1, 1, 1), 2);

	Game::instance->level = 1;
	Game::instance->RenderAllGUIs();
	Game::instance->LevelsGUI();
}
void Level2Stage::Render(void)
{
	drawText(50, 100, "LEVEL2", Vector3(Game::instance->window_width / 2.0, 1.0, Game::instance->window_height / 2.0), 5);
	drawText(Game::instance->window_width - 300, 50, "Encuentra 2 pistas", Vector3(1, 1, 1), 2);
	Game::instance->RenderAllGUIs();
	Game::instance->LevelsGUI();
}
void Level3Stage::Render(void)
{
	drawText(50, 100, "LEVEL3", Vector3(Game::instance->window_width / 2.0, 1.0, Game::instance->window_height / 2.0), 5);
	drawText(Game::instance->window_width - 300, 50, "Encuentra 5 pistas", Vector3(1, 1, 1), 2);
	Game::instance->RenderAllGUIs();
	Game::instance->LevelsGUI();
}
void EndStage::Render(void)
{
	drawText(50, 100, "END", Vector3(Game::instance->window_width / 2.0, 1.0, Game::instance->window_height / 2.0), 5);
	drawText(20, 200, "PULSA SPACE PARA TERMINAR", Vector3(0, 0, 0), 3);
	Game::instance->RenderAllGUIs();
	Game::instance->LevelsGUI();
}
