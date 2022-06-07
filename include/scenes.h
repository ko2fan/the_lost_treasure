#ifndef SCENES_H
#define SCENES_H

#include "raylib.h"

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
#define BACKGROUND_LAYERS 6
#define CLICKABLE_OBJECTS 3
#define MAX_INVENTORY 10
#define INVENTORY_OPEN 80
#define DIALOGUE_OPEN 120
#define MAX_DIALOGUES 1
#define MAX_OPTIONS 3
#define MAX_DESCRIPTION 128

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

typedef enum GameScene { FOREST = 0, RUINS } GameScene;

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

typedef struct Dialogue
{
	char* spoken_dialogue;
	int total_answers;
	char* answer_dialogue_options[MAX_OPTIONS];
	Rectangle dialogue_location[MAX_OPTIONS];
	bool answer_selected;
	int chosen_answer;
} Dialogue;

typedef struct NPC
{
	int current_dialogue;
	Dialogue* dialogue[MAX_DIALOGUES];
} NPC;

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

typedef struct Inventory
{
	int items_taken;
	InventoryObject items[MAX_INVENTORY];
} Inventory;

//----------------------------------------------------------------------------------
// Global Variables Declaration (shared by several modules)
//----------------------------------------------------------------------------------
extern Vector2 mousePosition;

extern WorldObject player;
extern Inventory player_inventory;
extern Animation player_idle_animation;
extern Animation player_walk_animation;
extern Dialogue* visible_dialogue;
extern float player_speed;
extern Vector2 player_target;
extern Vector2 zero;
extern Rectangle exit_location;
extern int hover;
extern int dir;


#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

	void ChangeScene(GameScene);
	Rectangle WorldObjectToRect(WorldObject*);
	void PickUpItem(ClickableObject*);
	int UpdateDialogue(int);
	void MovePlayer();

	//----------------------------------------------------------------------------------
	// Forest Scene Functions Declaration
	//----------------------------------------------------------------------------------
	void InitForestScene(Font);
	void UpdateForestScene(void);
	void DrawForestScene(Font);
	void UnloadForestScene(void);

	//----------------------------------------------------------------------------------
	// Ruins Scene Functions Declaration
	//----------------------------------------------------------------------------------
	void InitRuinsScene(Font);
	void UpdateRuinsScene(void);
	void DrawRuinsScene(Font);
	void UnloadRuinsScene(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SCENES_H