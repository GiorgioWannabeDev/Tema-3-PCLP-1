//CHELES Giorgio, 312CB
#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "functions.h"

typedef struct {
	universal_state snap_state;
	ppm_image snap_image;
	lsystem snap_lsystem;
	font snap_font;
} snapshot;

//Clear the stack from current_pos + 1 to last_pos
void clear_snapshot_stack(snapshot *target_stack, long current_pos,
						  long last_pos);

//Shallow copy a snapshot
snapshot get_snap_shallow_copy(snapshot *src);

//Undo
void undo_command(long *current_pos);

//Print command confirmation message based on snap_state
void print_confirmation_message(snapshot *target, universal_state snap_state);

//Redo
void redo_command(snapshot *target_stack, long *current_pos, long *last_pos);

//Load lsystem command
void lsystem_command(snapshot **target_stack, long *current_pos,
					 long *last_pos, long *stack_size);

//Derive command
void derive_command(snapshot *target_stack, long pos);

//Load image command
void load_command(snapshot **target_stack, long *current_pos,
				  long *last_pos, long *stack_size);

//Save image command
void save_command(snapshot *snap_stack, long snap_pos);

//Turtle graphics command
void turtle_command(snapshot **target_stack, long *current_pos,
					long *last_pos, long *stack_size);

//Load font command
void font_command(snapshot **target_stack, long *current_pos,
				  long *last_pos, long *stack_size);

//Type text command
void type_command(snapshot **target_stack, long *current_pos,
				  long *last_pos, long *stack_size);

//Bitcheck command
void bitcheck_command(snapshot *snap_stack, long snap_pos);

void command_line(void);

#endif
