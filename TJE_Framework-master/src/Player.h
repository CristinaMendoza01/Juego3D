#pragma once
#include "entity.h"

enum ePlayerState {
    IDLE,
    WALK,
    RUN
};

class Player : public Entity
{
public:
    Matrix44 model;
    Mesh* mesh;
    Texture* texture;

    float yaw;
    Vector3 pos;
    float pitch;

    Camera* camera;

    ePlayerState player_state;

    Animation* anim_idle;
    Animation* anim_walk;
    Animation* anim_run;
    Animation* current_anim;

    Player();
    Player(Entity* ent);

    Camera* InitPlayerCamera();

    Vector3 PlayerCollisions(Scene* scene, Camera* camera, Vector3 playerVel, float elapsed_time);

    int CheckCollision(Camera* cam, std::vector<Entity*>& entities, Entity* sEnt);

    bool DetectHint(Camera* cam, std::vector<Entity*>& entities, Entity* sEnt);

    void RenderPlayer(Matrix44 model, Mesh* mesh, Texture* textrure, Animation* anim, Shader* shader, Camera* cam, int primitive, float yaw, float pitch, float t);

};