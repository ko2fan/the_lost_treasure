#include "raylib.h"
#include "scenes.h"

static Texture2D background_layers[BACKGROUND_LAYERS];
static WorldObject butterfly = { 0 };
static Dialogue woodcutter_welcome = { 0 };
static NPC woodcutter_npc = { 0 };

static ClickableObject chest = { 0 };
static ClickableObject key = { 0 };
static ClickableObject woodcutter = { 0 };

static ClickableObject clickableObjects[CLICKABLE_OBJECTS];

static int framesCounter = 0;
static int showInventory = 0;
static int showDialogue = 0;
static int selectedObject = -1;
static int highlight = -1;

void InitForestScene(Font font)
{
    // Init global variables
    framesCounter = 0;

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

void UpdateForestScene()
{
    dir = 1;
    hover = 0;
    highlight = -1;
    mousePosition = GetMousePosition();

    showDialogue = UpdateDialogue(showDialogue);

    showInventory = (mousePosition.y < INVENTORY_OPEN) ? 1 : 0;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        selectedObject = -1;

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
    }
    else
    {
        MovePlayer();
    }
}

void DrawForestScene(Font font)
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

void UnloadForestScene()
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