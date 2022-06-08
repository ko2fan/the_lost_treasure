#include "raylib.h"
#include "scenes.h"

#define BACKGROUND_LAYERS 7
#define CLICKABLE_OBJECTS 1

static Texture2D background_layers[BACKGROUND_LAYERS];

static ClickableObject clickableObjects[CLICKABLE_OBJECTS];

static int framesCounter = 0;
static int showInventory = 0;
static int showDialogue = 0;
static int selectedObject = -1;
static int highlight = -1;

void InitRuinsScene(Font font)
{
    // Init global variables
    framesCounter = 0;

    player_idle_animation = (Animation){ 0 };
    player_walk_animation = (Animation){ 0 };

    // locals
    framesCounter = 0;
    showInventory = 0;
    showDialogue = 0;
    selectedObject = -1;
    highlight = -1;

    // BACKGROUNDS ////////////////////////////////////////////////////////////
    background_layers[0] = LoadTexture("data/1.png");
    background_layers[1] = LoadTexture("data/2.png");
    background_layers[2] = LoadTexture("data/3.png");
    background_layers[3] = LoadTexture("data/4.png");
    background_layers[4] = LoadTexture("data/5.png");
    background_layers[5] = LoadTexture("data/6.png");
    background_layers[6] = LoadTexture("data/7.png");

    // PLAYER /////////////////////////////////////////////////////////////////
    player_idle_animation.sprite = LoadTexture("data/GraveRobber.png");
    player_idle_animation.total_frames = 1;

    player_walk_animation.sprite = LoadTexture("data/GraveRobber_walk2.png");
    player_walk_animation.total_frames = 6;

    player.position = (Vector2){ 0, 300 };
    player.size = (Vector2){ 48, 48 };
    player.scale = (Vector2){ 4, 4 };
    player.animation = player_idle_animation;
    player_target = (Vector2){ 0, 300 };
    
}

void UpdateRuinsScene()
{
    dir = 1;
    hover = 0;
    highlight = -1;
    mousePosition = GetMousePosition();

    showDialogue = UpdateDialogue(showDialogue);
    if (showDialogue == 1)
        return;

    showInventory = (mousePosition.y < INVENTORY_OPEN) ? 1 : 0;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        selectedObject = 0;

        player_target.x = mousePosition.x - (player.size.x * player.scale.x) / 2;
        player_target.y = mousePosition.y - player.size.y * player.scale.y;
        player_target.y = MAX(player_target.y, 300);
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

    if ((int)player.position.x == (int)player_target.x && (int)player.position.y == (int)player_target.y)
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
        if (player.position.x > 650)
        {
            ChangeScene(FOREST);
        }
    }
    else
    {
        MovePlayer();
    }
}

void DrawRuinsScene(Font font)
{
    for (int i = 0; i < BACKGROUND_LAYERS; ++i)
    {
        DrawTextureEx(background_layers[i], (Vector2) { 0, -40 }, 0, 2, WHITE);
    }
    if (++framesCounter >= (60 / player.animation.total_frames))
    {
        player.animation.frame = (player.animation.frame + 1) % player.animation.total_frames;
        framesCounter = 0;
    }

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
            clickableObjects[i].world_item.size.x* clickableObjects[i].world_item.animation.frame, 0, clickableObjects[i].world_item.size.x, clickableObjects[i].world_item.size.y
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
            Rectangle source = { 0, 0, sprite->width, sprite->height };
            Rectangle dest = { 20 * i, 20, sprite->width * scale, sprite->height * scale };
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
                (Vector2) {
                visible_dialogue->dialogue_location[i].x, visible_dialogue->dialogue_location[i].y
            },
                font.baseSize * 2,
                    4,
                    hover == i ? GREEN : BLUE);
        }

        DrawTextEx(font, "Exit", (Vector2) { 600, 400 }, font.baseSize, 4, RED);
    }
}

void UnloadRuinsScene()
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

    UnloadTexture(player_idle_animation.sprite);
    UnloadTexture(player_walk_animation.sprite);
}