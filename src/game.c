/**********************************************************************************************
*
*   Adventure Game Jam 2022 Entry - The Lost Treasure
*
*   Copyright (c) 2022 David Athay
*
* - Animate character
* - Draw background sprites in layers
* - Items
* - Objects to pick up, open or interact with
* - NPCs to talk to
* - Dialogue, narration
* - Multiple scenes
**********************************************************************************************/

#include "raylib.h"
#include "screens.h"
#include "scenes.h"

//----------------------------------------------------------------------------------
// Module Variables Definition (local)
//---------------------------------------------------------------------------------
static GameScene current_scene;
static int finishScreen = 0;
WorldObject player;
Inventory player_inventory;
Animation player_idle_animation;
Animation player_walk_animation;
Dialogue* visible_dialogue;
float player_speed;
Vector2 player_target;
Vector2 zero;
Vector2 mousePosition;
Rectangle exit_location;
int hover;
int dir;

//----------------------------------------------------------------------------------
// Gameplay Screen Functions Definition
//----------------------------------------------------------------------------------

Rectangle WorldObjectToRect(WorldObject* object)
{
    return (Rectangle) { object->position.x, object->position.y, object->size.x* object->scale.x, object->size.y* object->scale.y };
}

void PickUpItem(ClickableObject* object)
{
    object->isTaken = true;
    object->canTake = false;
    player_inventory.items[player_inventory.items_taken] = object->inventory_item;
    player_inventory.items_taken += 1;
}

int UpdateDialogue(int showDialogue)
{
    if (showDialogue == 1)
    {
        for (int i = 0; i < visible_dialogue->total_answers; ++i)
        {
            if (CheckCollisionPointRec(mousePosition, visible_dialogue->dialogue_location[i]))
            {
                hover = i;
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    visible_dialogue->answer_selected = true;
                    visible_dialogue->chosen_answer = i;
                    return 0;
                }
            }
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (CheckCollisionPointRec(mousePosition, exit_location))
            {
                return 0;
            }
        }
    }

    return showDialogue;
}

void MovePlayer()
{
    if (player_target.x - player.position.x > 0.35f)
    {
        player.position.x += GetFrameTime() * player_speed;
        if (player.position.x > player_target.x)
            player.position.x = player_target.x;
    }
    else if (player_target.x - player.position.x < -0.35f)
    {
        player.position.x -= GetFrameTime() * player_speed;
        dir = -1;
        if (player.position.x < player_target.x)
            player.position.x = player_target.x;
    }
    else 
    {
        player.position.x = player_target.x;
    }
    if (player_target.y - player.position.y > 0.1f)
    {
        player.position.y += GetFrameTime() * player_speed;
    }
    else if (player_target.y - player.position.y < -0.1f)
    {
        player.position.y -= GetFrameTime() * player_speed;
    }
    else
    {
        player.position.y = player_target.y;
    }
}

void ChangeScene(GameScene scene)
{
    switch (current_scene)
    {
    case FOREST:
        UnloadForestScene();
        break;
    case RUINS:
        UnloadRuinsScene();
        break;
    }
    current_scene = scene;
    switch (current_scene)
    {
    case FOREST:
        InitForestScene(font);
        break;
    case RUINS:
        InitRuinsScene(font);
        break;
    }
}

void InitGameplayScreen(void)
{
    finishScreen = 0;
    mousePosition = (Vector2){ 0 };
    current_scene = FOREST;

    zero = (Vector2){ 0 };
    player_idle_animation = (Animation){ 0 };
    player_walk_animation = (Animation){ 0 };
    player = (WorldObject){ 0 };
    player_inventory = (Inventory){0, {0}};
    player_speed = 75.0f;
    player_target = (Vector2){0, 300};
    exit_location = (Rectangle){600, 400, 25, 25};
    
    InitForestScene(font);
}

void UpdateGameplayScreen(void)
{
    switch (current_scene)
    {
    case FOREST:
        UpdateForestScene();
        break;
    case RUINS:
        UpdateRuinsScene();
        break;
    }

}

void DrawGameplayScreen(void)
{
    switch (current_scene)
    {
    case FOREST:
        DrawForestScene(font);
        break;
    case RUINS:
        DrawRuinsScene(font);
        break;
    }
    
}

void UnloadGameplayScreen(void)
{
    switch (current_scene)
    {
    case FOREST:
        UnloadForestScene();
        break;
    case RUINS:
        UnloadRuinsScene();
        break;
    }
}

int FinishGameplayScreen(void)
{
    return finishScreen;
}