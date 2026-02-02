//CHELES Giorgio, 312CB
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define PI 3.14159265358979323846

#define BYTE_SIZE 8

//every lsystem, image and font can be empty, just loaded, or inherited
//from previous snapshot
typedef enum {
	EMPTY_ENTRY = 0,
	NEW_ENTRY = 1,
	COPY_ENTRY = 2

} element_state;

//we use these states for every snapshot in the undo/redo system
typedef enum {
	FIRST_STATE = 0,
	LOADED_LSYSTEM = 1,
	LOADED_IMAGE = 2,
	LOADED_FONT = 3,
	TURTLE_DRAWN = 4,
	TYPED_TEXT = 5
} universal_state;

typedef enum {
	EXIT = 0,
	UNDO = 1,
	REDO = 2,
	LSYSTEM = 3,//undoable
	DERIVE = 4,//not undoable
	LOAD = 5,//undoable
	SAVE = 6,//not undoable
	TURTLE = 7,//undoable
	FONT = 8,//undoable
	TYPE = 9,//undoable
	BITCHECK = 10//not undoable
} command_keyword;

typedef struct {
	char predecessor;
	char *successor;
} rule;

typedef struct {
	element_state state;
	char *axiom;
	rule *rules;
	long rule_nmb;
	char *file_name;
} lsystem;

typedef struct __attribute__((packed)) {
	unsigned char R, G, B;
} pixel;

typedef struct {
	element_state state;
	long width, height;
	char *file_name;
	pixel **image_grid;
} ppm_image;

typedef struct {
	double x, y, angle;
} turtle;

typedef struct {
	turtle *vector;
	long size;
	long capacity;
} turtle_vector;

typedef struct {
	char *char_name;
	int code;
	long dwx, dwy;
	long bbw, bbh, bbxoff, bbyoff;
	unsigned char **glyph_map;
} glyph;

typedef struct {
	element_state state;
	char *file_name;
	char *font_name;
	long font_nmb;
	glyph *glyph_array;
} font;

//Convert degrees to radians
double to_radians(double degrees);

//Absolute value for long input
long abs_long(long x);

///Memory Allocation

//Allocate memory for an image
pixel **image_matrix_malloc(long height, long width);

//Free memory of an image
void image_matrix_free(pixel **target);

//Get a duplicate of the src image
pixel **image_matrix_copy(pixel **src, long height, long width);

///Turtle state operations

//Add a state to turtle stack
void add_state(turtle_vector *stack, turtle new_state);

//Read the last state of the turtle stack
turtle get_state(turtle_vector stack);

//Remove the last state of the turtle stack
void remove_state(turtle_vector *stack);

//Clear the turtle stack
void clear_stack(turtle_vector *stack);

///Font opearations

//Allocate memory for a glyph
unsigned char **bitmap_matrix_malloc(long height, long width);

//Free memory of a glyph
void bitmap_matrix_free(unsigned char **target);

//Read glyph matrix from FILE src
void read_glyph_map(glyph *dest, FILE *src);

//Read glyph data from FILE src
void read_glyph(glyph *dest, FILE *src);

//Get the glyph corresponding to c
glyph *get_glyph(font *f, char c);

//Type text on target using f font starting at start_x, start_y
void type_text(ppm_image *target, font *f, char *text,
			   long start_x, long start_y, pixel color);

//Load font from file file_name to target
int load_font(font *target, char *file_name);

//Unload the font
void unload_font(font *target);

//Image Processing

//Draw pixel
void draw_pixel(ppm_image *target_image, long x, long y, pixel drawn_pixel);

//Draw line from (x0, y0) to (x1, y1)
void draw_line(ppm_image *target_image, long x0, long y0,
			   long x1, long y1, pixel target_color);

//Move the turtle ahead
void move_turtle(ppm_image *target_image, turtle *target_turtle,
				 pixel target_color, double step);

//Draw on target_image using nth derivation of system starting at (x0, y0)
void draw_turtle(ppm_image *target_image, lsystem system,
				 long x0, long y0,
				 long step, double angle, double angle_step,
				 long n, pixel pixel_color);

//Load image from file file_name to image
int load_image(ppm_image *image, char *file_name);

//Unload image
void unload_image(ppm_image *image);

//Get a copy of src ppm_image
ppm_image get_ppm_image_copy(ppm_image *src);

//Save the current image
void save_image(ppm_image *image, char *file_image);

///File Processing

//Get the next quoted text from FILE src
char *get_file_quote(FILE *src);

//Get the next word from FILE src
char *get_file_word(FILE *src);

//Get the next line from FILE src
char *get_file_line(FILE *src);

//String Processing

//Get a duplicate of the src string
char *string_dup(char *src);

//Get the successor of c character from system
char *get_replacement(char c, lsystem system);

//Derive the string once using system
void derive_once(char **string, lsystem system);

//get the nth derivation of the system
char *get_derive(lsystem system, long n);

//print the nth derivation of the system
void print_derive(lsystem system, long n);

///Lsystem loading/unloading

//Load lsystem from file file_name to system
int load_lsystem(lsystem *system, char *file_name);

//Unload lsystem
void unload_lsystem(lsystem *system);

//Get enum value of string
command_keyword get_command_index(char *string);

//Print bitcheck warning
void print_bitcheck_warning(int *arr, long x, long y);

//Convert pixel to an array of 1s and 0s
void set_pixel_to_array(int *array, pixel src);

//Bitcheck the image
void bitcheck(ppm_image *image);

#endif
