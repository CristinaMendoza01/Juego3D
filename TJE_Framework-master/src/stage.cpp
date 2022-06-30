#include "stage.h"
#include "game.h"

Stage* GetStage(STAGE_ID id, std::vector<Stage*> stages) {
	return stages[(int)id];
}

void SetStage(STAGE_ID* currentStage, STAGE_ID id) { // Como cambiar la stage
	*currentStage = id;
}

void IntroStage::Render(void)
{
	Game::instance->RenderAllGUIs();
	drawText(50, 100, "PRESS SPACE TO START", Vector3(Game::instance->window_width / 2.0, 1.0, Game::instance->window_height / 2.0), 2);
}
