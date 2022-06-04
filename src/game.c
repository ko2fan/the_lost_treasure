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

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define BACKGROUND_LAYERS 6
#define CLICKABLE_OBJECTS 2
#define MAX_INVENTORY 10
#define INVENTORY_OPEN 80

//----------------------------------------------------------------------------------
// Module Variables Definition (local)
//----------------------------------------------------------------------------------
static int framesCounter = 0;
static int finishScreen = 0;
static int dir = 1;
static int showInventory = 0;

Texture2D background_layers[BACKGROUND_LAYERS];

typedef struct Timer
{
    float lifetime;
} Timer;

void StartTimer(Timer *timer, float value)
{
    timer->lifetime = value;
}

void UpdateTimer(Timer* timer)
{
    timer->lifetime -= GetFrameTime();
}

typedef struct Animation
{
    Texture2D sprite;
    int total_frames;
    int frame;
} Animation;

typedef struct InventoryObject
{
    Texture2D object_sprite;
} InventoryObject;

typedef struct WorldObject
{
    Vector2 position;
    Vector2 size;
    Vector2 scale;
    Animation animation;
} WorldObject;
static WorldObject player = { 0 };
static WorldObject butterfly = { 0 };

typedef struct ClickableObject
{
    WorldObject world_item;
    InventoryObject inventory_item;
    bool clicked;
    bool canOpen;
    bool isOpen;
    bool canTake;
    bool isTaken;
} ClickableObject;
static ClickableObject chest = { 0 };
static ClickableObject key = { 0 };

typedef struct Inventory
{
    int items_taken;
    InventoryObject items[MAX_INVENTORY];
} Inventory;
static Inventory player_inventory = { 0, { 0 } };

ClickableObject clickableObjects[CLICKABLE_OBJECTS];

float playerSpeed = 75.0f;
Vector2 zero = {0};
Vector2 mousePosition = {0};
Vector2 playerTarget = { 0, 300 };

Animation player_idle_animation = {0};
Animation player_walk_animation = {0};

enum Scenes {
    SCENE_FOREST,
    SCENE_RUINS
};

//----------------------------------------------------------------------------------
// Gameplay Screen Functions Definition
//----------------------------------------------------------------------------------

Rectangle WorldObjectToRect(WorldObject* object)
{
     return (Rectangle){object->position.x, object->position.y, object->size.x * object->scale.x, object->size.y * object->scale.y};
}

void PickUpItem(ClickableObject *object)
{
    object->isTaken = true;
    object->canTake = false;
    player_inventory.items[player_inventory.items_taken] = object->inventory_item;
    player_inventory.items_taken += 1;
    object->clicked = false;
}

void InitGameplayScreen(void)
{
    framesCounter = 0;
    finishScreen = 0;
    background_layers[0] = LoadTexture("data/bg.png");
    background_layers[1] = LoadTexture("data/trees3.png");
    background_layers[2] = LoadTexture("data/trees2.png");
    background_layers[3] = LoadTexture("data/trees1.png");
    background_layers[4] = LoadTexture("data/bushes.png");
    background_layers[5] = LoadTexture("data/grass.png");

    player_idle_animation.sprite = LoadTexture("data/GraveRobber.png");
    player_idle_animation.total_frames = 1;

    player_walk_animation.sprite = LoadTexture("data/GraveRobber_walk2.png");
    player_walk_animation.total_frames = 6;

    player.position = (Vector2){ 0, 300};
    player.size = (Vector2){ 48, 48 };
    player.scale = (Vector2){ 4, 4 };
    player.animation = player_idle_animation;

    butterfly.position = (Vector2){ 0, 0 };
    butterfly.size = (Vector2){ 1920, 1080 };
    butterfly.scale = (Vector2){ 1, 1 };
    butterfly.animation = (Animation){ 0 };
    butterfly.animation.sprite = LoadTexture("data/butterfly1.png");

    chest.world_item = (WorldObject){ 0 };
    chest.world_item.position = (Vector2){ 400, 350 };
    chest.world_item.size = (Vector2){ 32, 32 };
    chest.world_item.scale = (Vector2){ 4, 4 };
    chest.world_item.animation = (Animation){ 0 };
    chest.world_item.animation.sprite = LoadTexture("data/Chest.png");
    chest.world_item.animation.total_frames = 4;
    chest.clicked = false;
    chest.canOpen = true;
    chest.isOpen = false;
    chest.canTake = false;
    chest.isTaken = false;

    key.world_item = (WorldObject){ 0 };
    key.world_item.position = (Vector2){ 124, 390 };
    key.world_item.size = (Vector2){ 8, 8 };
    key.world_item.scale = (Vector2){ 4, 4 };
    key.world_item.animation = (Animation){ 0 };
    key.world_item.animation.sprite = LoadTexture("data/Key.png");
    key.world_item.animation.total_frames = 4;
    key.inventory_item = (InventoryObject){0};
    key.inventory_item.object_sprite = LoadTexture("data/Key.png");
    key.clicked = false;
    key.canOpen = false;
    key.isOpen = false;
    key.canTake = true;
    key.isTaken = false;

    clickableObjects[0] = chest;
    clickableObjects[1] = key;

}

void UpdateGameplayScreen(void)
{
    dir = 1;
    mousePosition = GetMousePosition();
    if (mousePosition.y < INVENTORY_OPEN)
    {
        showInventory = 1;
        return; // Early out, so that player doesnt move when clicking in inventory
    }
    else
    {
        showInventory = 0;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        for (int i = 0; i < CLICKABLE_OBJECTS; ++i)
        {
            clickableObjects[i].clicked = false;
            
            if (CheckCollisionPointRec(mousePosition, WorldObjectToRect(&clickableObjects[i].world_item)))
            {
                clickableObjects[i].clicked = true;
            }
        }
        
        playerTarget.x = mousePosition.x - (player.size.x * player.scale.x) / 2;
        playerTarget.y = mousePosition.y - player.size.y * player.scale.y;
        playerTarget.y = MAX(playerTarget.y, 300);
        player.animation = player_walk_animation;
    }

    if ((int)player.position.x == (int)playerTarget.x && (int)player.position.y == (int)playerTarget.y)
    {
        player.animation = player_idle_animation;
        for (int i = 0; i < CLICKABLE_OBJECTS; ++i)
        {
            if (clickableObjects[i].clicked)
            {
                if (clickableObjects[i].canOpen)
                {
                    clickableObjects[i].isOpen = true;
                }
                else if (clickableObjects[i].canTake)
                {
                    PickUpItem(&clickableObjects[i]);
                }
            }
        }
    }
    else
    {
        if (playerTarget.x - player.position.x > 0.1f)
        {
            player.position.x += GetFrameTime() * playerSpeed;
        }
        else if (playerTarget.x - player.position.x < -0.1f)
        {
            player.position.x -= GetFrameTime() * playerSpeed;
            dir = -1;
        }
        else {
            player.position.x = playerTarget.x;
        }
        if (playerTarget.y - player.position.y > 0.1f)
        {
            player.position.y += GetFrameTime() * playerSpeed;
        }
        else if (playerTarget.y - player.position.y < -0.1f)
        {
            player.position.y -= GetFrameTime() * playerSpeed;
        }
        else
        {
            player.position.y = playerTarget.y;
        }
    }

}

void DrawGameplayScreen(void)
{
    for (int i = 0; i < BACKGROUND_LAYERS; ++i)
    {
        DrawTextureEx(background_layers[i], zero, 0, 0.5f, WHITE);
    }
    if (++framesCounter >= (60 / player.animation.total_frames))
    {
        player.animation.frame = (player.animation.frame + 1) % player.animation.total_frames;
        framesCounter = 0;
    }

    DrawTextureEx(butterfly.animation.sprite, butterfly.position, 0, 0.5f, WHITE);

    for (int i = 0; i < CLICKABLE_OBJECTS; ++i)
    {
        if (clickableObjects[i].isTaken)
            continue;
        if (clickableObjects[i].isOpen)
        {
            clickableObjects[i].world_item.animation.frame = MIN(clickableObjects[i].world_item.animation.frame + 1, clickableObjects[i].world_item.animation.total_frames - 1);
        }
        DrawTexturePro(
            clickableObjects[i].world_item.animation.sprite,
            (Rectangle) {
                clickableObjects[i].world_item.size.x * clickableObjects[i].world_item.animation.frame, 0, clickableObjects[i].world_item.size.x, clickableObjects[i].world_item.size.y
            },
            WorldObjectToRect(&clickableObjects[i].world_item),
            zero,
            0.0f,
            WHITE
        );

    }  
    
    Rectangle source = { player.size.x * player.animation.frame, 0, dir * player.size.x, player.size.y };
    DrawTexturePro(player.animation.sprite, source, WorldObjectToRect(&player), zero, 0.0f, WHITE);

    if (showInventory != 0)
    {
        DrawRectangle(0, 0, GetScreenWidth(), INVENTORY_OPEN, DARKGRAY);
        for (int i = 0; i < player_inventory.items_taken; ++i)
        {
            float scale = 4.0f;
            Texture2D* sprite = &player_inventory.items[i].object_sprite;
            Rectangle source = { 0, 0, sprite->width, sprite->height};
            Rectangle dest = { 20 * i, 20, sprite->width * scale, sprite->height * scale};
            DrawTexturePro(*sprite, source, dest, zero, 0.0f, WHITE);
        }
    }
}

void UnloadGameplayScreen(void)
{
    for (int i = 0; i < BACKGROUND_LAYERS; ++i)
    {
        UnloadTexture(background_layers[i]);
    }
    for (int i = 0; i < CLICKABLE_OBJECTS; ++i)
    {
        UnloadTexture(clickableObjects[i].world_item.animation.sprite);
    }
    for (int i = 0; i < player_inventory.items_taken; ++i)
    {
        UnloadTexture(player_inventory.items[i].object_sprite);
    }

    UnloadTexture(butterfly.animation.sprite);

    UnloadTexture(player_idle_animation.sprite);
    UnloadTexture(player_walk_animation.sprite);
}

int FinishGameplayScreen(void)
{
    return finishScreen;
}