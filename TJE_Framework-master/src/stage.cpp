#include "stage.h"
#include "game.h"

Stage* GetStage(STAGE_ID id, std::vector<Stage*> stages) {
	return stages[(int)id];
}

void SetStage(STAGE_ID* currentStage, STAGE_ID id) { // Como cambiar la stage
	*currentStage = id;
}