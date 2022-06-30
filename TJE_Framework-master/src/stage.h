#ifndef STAGE_H
#define STAGE_H

#include "game.h"

enum STAGE_ID {
	INTRO = 0,
	INFO = 1,
	TUTORIAL = 2,
	LEVEL1 = 3,
	LEVEL2 = 4,
	LEVEL3 = 5,
	END = 6,
	EXIT = 7
}; //Para indexar las diferentes stages

class Stage {
public:
	virtual STAGE_ID GetID() = 0;
	virtual void Render(void) = 0;
	virtual void Update(float seconds_elapsed) = 0;
};

Stage* GetStage(STAGE_ID id, std::vector<Stage*> stages);
void SetStage(STAGE_ID* currentStage, STAGE_ID id);

class IntroStage : public Stage {
public:
	STAGE_ID GetID() {
		return STAGE_ID::INTRO;
	};
	void Render(void);
	void Update(float seconds_elapsed) {}
};

class InfoStage : public Stage {
public:
	STAGE_ID GetID() {
		return STAGE_ID::INFO;
	};
	void Render(void);
	void Update(float seconds_elapsed) {}
};

class TutorialStage : public Stage {
public:
	STAGE_ID TutorialStage::GetID() {
		return STAGE_ID::TUTORIAL;
	};
	void Render(void);
	void Update(float seconds_elapsed) {}
};

class Level1Stage : public Stage {
public:
	STAGE_ID Level1Stage::GetID() {
		return STAGE_ID::LEVEL1;
	};
	void Render(void);
	void Update(float seconds_elapsed) {}
};

class Level2Stage : public Stage {
public:
	STAGE_ID Level2Stage::GetID() {
		return STAGE_ID::LEVEL2;
	};
	void Render(void);
	void Update(float seconds_elapsed) {}
};

class Level3Stage : public Stage {
public:
	STAGE_ID Level3Stage::GetID() {
		return STAGE_ID::LEVEL3;
	};
	void Render(void);
	void Update(float seconds_elapsed) {}
};

class EndStage : public Stage {
public:
	STAGE_ID EndStage::GetID() {
		return STAGE_ID::END;
	};
	void Render(void);
	void Update(float seconds_elapsed) {}
};

class ExitStage : public Stage {
public:
	STAGE_ID ExitStage::GetID() {
		return STAGE_ID::EXIT;
	};
	void Render(void) {};
	void Update(float seconds_elapsed) {}
};

#endif
