//CHELES Giorgio, 312CB
#include "snapshot.h"
#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void clear_snapshot_stack(snapshot *target_stack,
						  long current_pos,
						  long last_pos)
{

	for (long i = current_pos + 1; i <= last_pos; i++) {

		unload_lsystem(&target_stack[i].snap_lsystem);

		unload_image(&target_stack[i].snap_image);

		unload_font(&target_stack[i].snap_font);
	}

	if (current_pos == last_pos) {
		return;
	}

	memset(target_stack + current_pos + 1, 0,
		   (last_pos - current_pos) * sizeof(snapshot));

}

snapshot get_snap_shallow_copy(snapshot *src)
{
	snapshot dest;
	memcpy(&dest, src, sizeof(snapshot));

	if (dest.snap_font.state == NEW_ENTRY) {
		dest.snap_font.state = COPY_ENTRY;
	}
	if (dest.snap_image.state == NEW_ENTRY) {
		dest.snap_image.state = COPY_ENTRY;
	}
	if (dest.snap_lsystem.state == NEW_ENTRY) {
		dest.snap_lsystem.state = COPY_ENTRY;
	}

	return dest;
}

void undo_command(long *current_pos)
{
	if ((*current_pos) <= 0) {
		printf("Nothing to undo\n");
		return;
	}
	(*current_pos)--;
}

void print_confirmation_message(snapshot *target, universal_state snap_state)
{
	switch (snap_state) {
	case FIRST_STATE:
		//nothing to display
		break;
	case LOADED_LSYSTEM:
		printf("Loaded %s (L-system with %ld rules)\n",
			   target->snap_lsystem.file_name,
			   target->snap_lsystem.rule_nmb);
		break;
	case LOADED_IMAGE:
		printf("Loaded %s (PPM image %ldx%ld)\n",
			   target->snap_image.file_name,
			   target->snap_image.width,
			   target->snap_image.height);
		break;
	case LOADED_FONT:
		printf("Loaded %s (bitmap font %s)\n",
			   target->snap_font.file_name,
			   target->snap_font.font_name);
		break;
	case TURTLE_DRAWN:
		printf("Drawing done\n");
		break;
	case TYPED_TEXT:
		printf("Text written\n");
		break;
	default:
		break;
	}
}

void redo_command(snapshot *target_stack, long *current_pos, long *last_pos)
{
	if ((*current_pos) >= (*last_pos)) {
		printf("Nothing to redo\n");
		return;
	}
	(*current_pos)++;

	//display the proper confirmation message
	print_confirmation_message(&target_stack[(*current_pos)],
							   target_stack[(*current_pos)].snap_state);
}

void lsystem_command(snapshot **target_stack, long *current_pos,
					 long *last_pos, long *stack_size)
{

	//create a new snapshot / make a shallow copy of the previous entry
	snapshot new_snap = get_snap_shallow_copy(&(*target_stack)[*current_pos]);

	char *file_name = get_file_word(stdin);
	if (!file_name) {
		return;
	}
	if (!load_lsystem(&new_snap.snap_lsystem, file_name)) {
		printf("Failed to load %s\n", file_name);
		free(file_name);
		return;
	}
	new_snap.snap_state = LOADED_LSYSTEM;
	free(file_name);
	new_snap.snap_lsystem.state = NEW_ENTRY;

	//clear the rest of the stack
	clear_snapshot_stack(*target_stack, *current_pos, *last_pos);

	//check if the stack is big enough for another entry
	if ((*stack_size - 1) <= (*current_pos)) {
		snapshot *temp = realloc(*target_stack,
								 *stack_size * 2 * sizeof(snapshot));
		if (!temp) {
			perror("ERROR::LSYSTEM_COMMAND: Failed to allocate memory!\n");
			return;
		}
		*target_stack = temp;
		*stack_size *= 2;
	}

	//assign the new snapshot in the stack
	(*current_pos)++;
	(*last_pos) = (*current_pos);
	(*target_stack)[(*current_pos)] = new_snap;

	print_confirmation_message(&new_snap, LOADED_LSYSTEM);
}

void derive_command(snapshot *target_stack, long pos)
{
	long n;
	scanf(" %ld", &n);

	if (target_stack[pos].snap_lsystem.state == EMPTY_ENTRY) {
		printf("No L-system loaded\n");
		return;
	}

	char *derived_string = get_derive(target_stack[pos].snap_lsystem, n);
	if (!derived_string) {
		perror("ERROR::DERIVE_COMMAND: Failed to get the derived string!\n");
		return;
	}
	printf("%s\n", derived_string);
	free(derived_string);
}

void load_command(snapshot **target_stack, long *current_pos,
				  long *last_pos, long *stack_size)
{
	//create a new snapshot / make a shallow copy of the previous entry
	snapshot new_snap = get_snap_shallow_copy(&(*target_stack)[*current_pos]);

	char *file_name = get_file_word(stdin);
	if (!file_name) {
		return;
	}

	if (!load_image(&new_snap.snap_image, file_name)) {
		printf("Failed to load %s\n", file_name);
		free(file_name);
		return;
	}
	new_snap.snap_image.state = NEW_ENTRY;
	new_snap.snap_state = LOADED_IMAGE;
	free(file_name);

	//clear the rest of the stack
	clear_snapshot_stack(*target_stack, *current_pos, *last_pos);

	//check if the stack is big enough for another entry
	if ((*stack_size - 1) <= (*current_pos)) {
		snapshot *temp = realloc(*target_stack,
								 *stack_size * 2 * sizeof(snapshot));
		if (!temp) {
			perror("ERROR::LOAD_COMMAND: Failed to allocate memory!\n");
			return;
		}
		*target_stack = temp;
		*stack_size *= 2;
	}

	//assign the new snapshot in the stack
	(*current_pos)++;
	(*last_pos) = (*current_pos);
	(*target_stack)[(*current_pos)] = new_snap;

	print_confirmation_message(&new_snap, LOADED_IMAGE);
}

void save_command(snapshot *snap_stack, long snap_pos)
{
	char *file_name = get_file_word(stdin);
	if (!file_name) {
		return;
	}

	if (snap_stack[snap_pos].snap_image.state == EMPTY_ENTRY) {
		free(file_name);
		printf("No image loaded\n");
	}

	save_image(&snap_stack[snap_pos].snap_image, file_name);
	printf("Saved %s\n", file_name);
	free(file_name);
}

void turtle_command(snapshot **target_stack, long *current_pos,
					long *last_pos, long *stack_size)
{
	snapshot *target = (*target_stack) + *current_pos;

	int r, g, b;
	long x, y, step, orientation, angle_step, n_derivation;
	pixel color;

	scanf(" %ld %ld %ld %ld %ld %ld %d %d %d", &x, &y, &step,
		  &orientation, &angle_step, &n_derivation, &r, &g, &b);

	color.R = r;
	color.G = g;
	color.B = b;

	//verify if we can apply turtle graphics
	if (target->snap_lsystem.state == EMPTY_ENTRY) {
		printf("No L-system loaded\n");
		return;
	}
	if (target->snap_image.state == EMPTY_ENTRY) {
		printf("No image loaded\n");
		return;
	}

	//We can proceed
	snapshot new_snap = get_snap_shallow_copy(target);

	new_snap.snap_state = TURTLE_DRAWN;

	new_snap.snap_image = get_ppm_image_copy(&target->snap_image);

	draw_turtle(&new_snap.snap_image, new_snap.snap_lsystem,
				x, y, step, orientation, angle_step, n_derivation, color);

	//clear the rest of the stack
	clear_snapshot_stack(*target_stack, *current_pos, *last_pos);

	//check if the stack is big enough for another entry
	if ((*stack_size - 1) <= (*current_pos)) {
		snapshot *temp = realloc(*target_stack,
								 *stack_size * 2 * sizeof(snapshot));
		if (!temp) {
			perror("ERROR::TURTLE_COMMAND: Failed to allocate memory!\n");
			return;
		}
		*target_stack = temp;
		*stack_size *= 2;
	}

	//assign the new snapshot in the stack
	(*current_pos)++;
	(*last_pos) = (*current_pos);
	(*target_stack)[(*current_pos)] = new_snap;

	print_confirmation_message(&new_snap, TURTLE_DRAWN);
}

void font_command(snapshot **target_stack, long *current_pos,
				  long *last_pos, long *stack_size)
{
	//create a new snapshot / make a shallow copy of the previous entry
	snapshot new_snap = get_snap_shallow_copy(&(*target_stack)[*current_pos]);

	char *file_name = get_file_word(stdin);
	if (!file_name) {
		return;
	}

	if (!load_font(&new_snap.snap_font, file_name)) {
		printf("Failed to load %s\n", file_name);
		free(file_name);
		return;
	}
	new_snap.snap_font.state = NEW_ENTRY;
	new_snap.snap_state = LOADED_FONT;
	free(file_name);

	//clear the rest of the stack
	clear_snapshot_stack(*target_stack, *current_pos, *last_pos);

	//check if the stack is big enough for another entry
	if ((*stack_size - 1) <= (*current_pos)) {
		snapshot *temp = realloc(*target_stack,
								 *stack_size * 2 * sizeof(snapshot));
		if (!temp) {
			perror("ERROR::FONT_COMMAND: Failed to allocate memory!\n");
			return;
		}
		*target_stack = temp;
		*stack_size *= 2;
	}

	//assign the new snapshot in the stack
	(*current_pos)++;
	(*last_pos) = (*current_pos);
	(*target_stack)[(*current_pos)] = new_snap;

	print_confirmation_message(&new_snap, LOADED_FONT);
}

void type_command(snapshot **target_stack, long *current_pos,
				  long *last_pos, long *stack_size)
{
	snapshot *target = (*target_stack) + *current_pos;

	char *text;

	text = get_file_quote(stdin);

	long start_x, start_y;
	int r, g, b;
	pixel color;

	scanf(" %ld %ld %d %d %d", &start_x, &start_y, &r, &g, &b);

	color.R = r;
	color.G = g;
	color.B = b;

	if (!text) {
		return;
	}

	//verify if we can type text
	if (target->snap_image.state == EMPTY_ENTRY) {
		printf("No image loaded\n");
		free(text);
		return;
	}
	if (target->snap_font.state == EMPTY_ENTRY) {
		printf("No font loaded\n");
		free(text);
		return;
	}

	//We can proceed
	snapshot new_snap = get_snap_shallow_copy(target);

	new_snap.snap_state = TYPED_TEXT;

	new_snap.snap_image = get_ppm_image_copy(&target->snap_image);

	type_text(&new_snap.snap_image, &new_snap.snap_font,
			  text, start_x, start_y, color);

	free(text);

	//clear the rest of the stack
	clear_snapshot_stack(*target_stack, *current_pos, *last_pos);

	//check if the stack is big enough for another entry
	if ((*stack_size - 1) <= (*current_pos)) {
		snapshot *temp = realloc(*target_stack,
								 *stack_size * 2 * sizeof(snapshot));
		if (!temp) {
			perror("ERROR::TYPE_COMMAND: Failed to allocate memory!\n");
			return;
		}
		*target_stack = temp;
		*stack_size *= 2;
	}

	new_snap.snap_image.state = NEW_ENTRY;

	//assign the new snapshot in the stack
	(*current_pos)++;
	(*last_pos) = (*current_pos);
	(*target_stack)[(*current_pos)] = new_snap;

	print_confirmation_message(&new_snap, TYPED_TEXT);
}

void bitcheck_command(snapshot *snap_stack, long snap_pos)
{
	if (snap_stack[snap_pos].snap_image.state == EMPTY_ENTRY) {
		printf("No image loaded\n");
		return;
	}

	bitcheck(&snap_stack[snap_pos].snap_image);
}

void command_line(void)
{
	//init the initial snapshot
	snapshot *snap_stack;
	long snap_pos = 0;
	long snap_stack_size = 1;
	long last_pos = 0;
	snap_stack = calloc(1, sizeof(snapshot));

	//get into reading commands
	int ok = 1;
	while (ok) {
		char *command_string = get_file_word(stdin);
		command_keyword command_index = get_command_index(command_string);
		free(command_string);

		switch (command_index) {
		case EXIT:
			ok = 0;
			break;
		case UNDO:
			undo_command(&snap_pos);
		break;
		case REDO:
			redo_command(snap_stack, &snap_pos, &last_pos);
		break;
		case LSYSTEM:
			lsystem_command(&snap_stack, &snap_pos,
							&last_pos, &snap_stack_size);
		break;
		case DERIVE:
			derive_command(snap_stack, snap_pos);
		break;
		case LOAD:
			load_command(&snap_stack, &snap_pos,
						 &last_pos, &snap_stack_size);
		break;
		case SAVE:
			save_command(snap_stack, snap_pos);
		break;
		case TURTLE:
			turtle_command(&snap_stack, &snap_pos,
						   &last_pos, &snap_stack_size);
		break;
		case FONT:
			font_command(&snap_stack, &snap_pos,
						 &last_pos, &snap_stack_size);
		break;
		case TYPE:
			type_command(&snap_stack, &snap_pos,
						 &last_pos, &snap_stack_size);
		break;
		case BITCHECK:
			bitcheck_command(snap_stack, snap_pos);
		break;
		default:

		break;
		}
	}

	clear_snapshot_stack(snap_stack, 0, last_pos);
	free(snap_stack);
}
