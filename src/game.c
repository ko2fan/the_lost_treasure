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
#define CLICKABLE_OBJECTS 3
#define MAX_INVENTORY 10
#define INVENTORY_OPEN 80
#define DIALOGUE_OPEN 120
#define MAX_DIALOGUES 1
#define MAX_OPTIONS 3
#define MAX_DESCRIPTION 128

//----------------------------------------------------------------------------------
// Module Variables Definition (local)
//----------------------------------------------------------------------------------
static int framesCounter = 0;
static int finishScreen = 0;
static int dir = 1;
static int showInventory = 0;
static int showDialogue = 0;
static int hover = 0;
static int selectedObject = -1;
static int highlight = -1;

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

typedef struct Dialogue
{
    char* spoken_dialogue;
    int total_answers;
    char* answer_dialogue_options[MAX_OPTIONS];
    Rectangle dialogue_location[MAX_OPTIONS];
    bool answer_selected;
    int chosen_answer;
} Dialogue;
static Dialogue woodcutter_welcome = { 0 };

typedef struct NPC
{
    int current_dialogue;
    Dialogue* dialogue[MAX_DIALOGUES];
} NPC;
static NPC woodcutter_npc = { 0 };

typedef struct ClickableObject
{
    WorldObject world_item;
    InventoryObject inventory_item;
    char description[MAX_DESCRIPTION];
    bool canOpen;
    bool isOpen;
    bool canTake;
    bool isTaken;
    bool canTalk;
    NPC* npc;
} ClickableObject;
static ClickableObject chest = { 0 };
static ClickableObject key = { 0 };
static ClickableObject woodcutter = { 0 };

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
Rectangle exitLocation = { 600, 400, 25, 25 };

Animation player_idle_animation = {0};
Animation player_walk_animation = {0};
Dialogue* visible_dialogue;

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
}

void InitGameplayScreen(void)
{
    framesCounter = 0;
    finishScreen = 0;

    // BACKGROUNDS ////////////////////////////////////////////////////////////
    background_layers[0] = LoadTexture("data/bg.png");
    background_layers[1] = LoadTexture("data/trees3.png");
    background_layers[2] = LoadTexture("data/trees2.png");
    background_layers[3] = LoadTexture("data/trees1.png");
    background_layers[4] = LoadTexture("data/bushes.png");
    background_layers[5] = LoadTexture("data/grass.png");

    // PLAYER /////////////////////////////////////////////////////////////////
    player_idle_animation.sprite = LoadTexture("data/GraveRobber.png");
    player_idle_animation.total_frames = 1;

    player_walk_animation.sprite = LoadTexture("data/GraveRobber_walk2.png");
    player_walk_animation.total_frames = 6;

    player.position = (Vector2){ 0, 300};
    player.size = (Vector2){ 48, 48 };
    player.scale = (Vector2){ 4, 4 };
    player.animation = player_idle_animation;

    // BUTTERFLY //////////////////////////////////////////////////////////////
    butterfly.position = (Vector2){ 0, 0 };
    butterfly.size = (Vector2){ 1920, 1080 };
    butterfly.scale = (Vector2){ 1, 1 };
    butterfly.animation = (Animation){ 0 };
    butterfly.animation.sprite = LoadTexture("data/butterfly1.png");

    // DIALOGUE ///////////////////////////////////////////////////////////////
    woodcutter_welcome.spoken_dialogue = "Hello, World!";
    woodcutter_welcome.total_answers = 3;
    woodcutter_welcome.answer_dialogue_options[0] = "Hello Mr.";
    woodcutter_welcome.answer_dialogue_options[1] = "You say hello, I say goodbye";
    woodcutter_welcome.answer_dialogue_options[2] = "Goodbye";
    Vector2 textSize = MeasureTextEx(font, woodcutter_welcome.answer_dialogue_options[0], font.baseSize * 2, 4);
    woodcutter_welcome.dialogue_location[0] = (Rectangle){ 200, 320, textSize.x, textSize.y };
    textSize = MeasureTextEx(font, woodcutter_welcome.answer_dialogue_options[1], font.baseSize * 2, 4);
    woodcutter_welcome.dialogue_location[1] = (Rectangle){ 200, 320 + textSize.y + 5, textSize.x, textSize.y };
    textSize = MeasureTextEx(font, woodcutter_welcome.answer_dialogue_options[2], font.baseSize * 2, 4);
    woodcutter_welcome.dialogue_location[2] = (Rectangle){ 200, 320 + ((textSize.y + 5) * 2), textSize.x, textSize.y };

    // WOODCUTTER NPC /////////////////////////////////////////////////////////
    woodcutter_npc.current_dialogue = 0;
    woodcutter_npc.dialogue[0] = &woodcutter_welcome;

    // CHEST //////////////////////////////////////////////////////////////////
    chest.world_item = (WorldObject){ 0 };
    chest.world_item.position = (Vector2){ 400, 350 };
    chest.world_item.size = (Vector2){ 32, 32 };
    chest.world_item.scale = (Vector2){ 4, 4 };
    chest.world_item.animation = (Animation){ 0 };
    chest.world_item.animation.sprite = LoadTexture("data/Chest.png");
    chest.world_item.animation.total_frames = 4;
    TextCopy(chest.description, "Treasure Chest");
    chest.canOpen = true;
    chest.isOpen = false;
    chest.canTake = false;
    chest.isTaken = false;
    chest.canTalk = false;
    chest.npc = 0;

    // KEY ////////////////////////////////////////////////////////////////////
    key.world_item = (WorldObject){ 0 };
    key.world_item.position = (Vector2){ 124, 390 };
    key.world_item.size = (Vector2){ 8, 8 };
    key.world_item.scale = (Vector2){ 4, 4 };
    key.world_item.animation = (Animation){ 0 };
    key.world_item.animation.sprite = LoadTexture("data/Key.png");
    key.world_item.animation.total_frames = 4;
    key.inventory_item = (InventoryObject){0};
    key.inventory_item.object_sprite = LoadTexture("data/Key.png");
    TextCopy(key.description, "A silver key");
    key.canOpen = false;
    key.isOpen = false;
    key.canTake = true;
    key.isTaken = false;
    key.canTalk = false;
    key.npc = 0;

    // WOODCUTTER /////////////////////////////////////////////////////////////
    woodcutter.world_item = (WorldObject){ 0 };
    woodcutter.world_item.position = (Vector2){ 600, 320 };
    woodcutter.world_item.size = (Vector2){ 48, 48 };
    woodcutter.world_item.scale = (Vector2){ 4, 4 };
    woodcutter.world_item.animation = (Animation){ 0 };
    woodcutter.world_item.animation.sprite = LoadTexture("data/Woodcutter.png");
    woodcutter.world_item.animation.total_frames = 4;
    TextCopy(woodcutter.description, "A man holding an axe");
    woodcutter.canOpen = false;
    woodcutter.isOpen = false;
    woodcutter.canTake = false;
    woodcutter.isTaken = false;
    woodcutter.canTalk = true;
    woodcutter.npc = &woodcutter_npc;

    // CLICKABLE OBJECTS //////////////////////////////////////////////////////
    clickableObjects[0] = chest;
    clickableObjects[1] = key;
    clickableObjects[2] = woodcutter;


}

void UpdateGameplayScreen(void)
{
    dir = 1;
    hover = 0;
    highlight = -1;
    mousePosition = GetMousePosition();

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
                    showDialogue = 0;
                }
            }
        }
        
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (CheckCollisionPointRec(mousePosition, exitLocation))
            {
                showDialogue = 0;
            }
        }
        
        return; // early out so you cant walk around whilst in dialogue
    }
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
        selectedObject = 0;

        playerTarget.x = mousePosition.x - (player.size.x * player.scale.x) / 2;
        playerTarget.y = mousePosition.y - player.size.y * player.scale.y;
        playerTarget.y = MAX(playerTarget.y, 300);
        player.animation = player_walk_animation;
    }
    
    for (int i = 0; i < CLICKABLE_OBJECTS; ++i)
    {
        if (CheckCollisionPointRec(mousePosition, WorldObjectToRect(&clickableObjects[i].world_item)))
        {
            if (!clickableObjects[i].isTaken)
                highlight = i;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                selectedObject = i;
            }
        }
    }

    if ((int)player.position.x == (int)playerTarget.x && (int)player.position.y == (int)playerTarget.y)
    {
        player.animation = player_idle_animation;
        if (selectedObject != -1)
        {
            if (clickableObjects[selectedObject].canOpen)
            {
                clickableObjects[selectedObject].isOpen = true;
            }
            else if (clickableObjects[selectedObject].canTake)
            {
                PickUpItem(&clickableObjects[selectedObject]);
                selectedObject = -1;
            }
            else if (clickableObjects[selectedObject].canTalk)
            {
                showDialogue = 1;
                visible_dialogue = clickableObjects[selectedObject].npc->dialogue[clickableObjects[selectedObject].npc->current_dialogue];
                selectedObject = -1;
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
    
    if (highlight != -1)
    {
        Vector2 location = { clickableObjects[highlight].world_item.position.x - 40, clickableObjects[highlight].world_item.position.y };
        DrawTextEx(font, clickableObjects[highlight].description, location, font.baseSize * 2, 4, WHITE);
    }

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

    if (showDialogue != 0)
    {
        DrawRectangle(0, 300, GetScreenWidth(), DIALOGUE_OPEN, DARKGRAY);
        DrawTextEx(font, visible_dialogue->spoken_dialogue, (Vector2) { 20, 300 }, font.baseSize * 2, 4, YELLOW);
        for (int i = 0; i < visible_dialogue->total_answers; ++i)
        {
            DrawTextEx(
                font,
                visible_dialogue->answer_dialogue_options[i],
                (Vector2) { visible_dialogue->dialogue_location[i].x, visible_dialogue->dialogue_location[i].y },
                font.baseSize * 2,
                4,
                hover == i ? GREEN : BLUE);
        }
        
        DrawTextEx(font, "Exit", (Vector2) { 600, 400 }, font.baseSize, 4, RED);
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