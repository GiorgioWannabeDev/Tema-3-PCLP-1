//CHELES Giorgio, 312CB
#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

double to_radians(double degrees)
{
	return degrees * PI / 180;
}

long abs_long(long x)
{
	return (x >= 0) ? x : ((-1) * x);
}

pixel **image_matrix_malloc(long height, long width)
{
	pixel **result = malloc(height * sizeof(pixel *));
	if (!result) {
		perror("ERROR::IMAGE_MATRIX_MALLOC:"
			   " Failed to allocate memory for image!\n");
		return NULL;
	}
	pixel *pixel_block = malloc(height * width * sizeof(pixel));
	if (!pixel_block) {
		perror("ERROR::IMAGE_MATRIX_MALLOC:"
			   " Failed to allocate memory for image!\n");
		free(result);
		return NULL;
	}

	for (long i = 0; i < height; i++) {
		result[i] = &pixel_block[i * width];
	}

	return result;
}

void image_matrix_free(pixel **target)
{
	if (!target) {
		return;
	}
	free(target[0]);
	free(target);
}

pixel **image_matrix_copy(pixel **src, long height, long width)
{
	pixel **image;

	//allocate memory for the grid
	image = image_matrix_malloc(height, width);
	if (!image) {
		perror("ERROR::IMAGE_MATRIX_COPY: Failed to allocate memory!\n");
		return NULL;
	}

	memcpy(image[0], src[0], height * width * sizeof(pixel));
	return image;
}

void add_state(turtle_vector *stack, turtle new_state)
{
	if (stack->capacity == 0) {
		stack->vector = malloc(4 * sizeof(turtle));
		if (!stack->vector) {
			stack->vector = NULL;
			return;
		}
		stack->capacity = 4;
	} else if (stack->capacity == stack->size) {
		turtle *temp = realloc(stack->vector,
						 2 * stack->capacity * sizeof(turtle));
		if (!temp) {
			perror("ERROR::ADD_STATE: Failed to reallocate memory!\n");
			return;
		}
		stack->vector = temp;

		stack->capacity *= 2;
	}
	stack->vector[stack->size++] = new_state;
}

turtle get_state(turtle_vector stack)
{
	if (stack.size == 0) {
		turtle invalid_turtle = {-1, -1, 0};
		return  invalid_turtle;
	}
	return stack.vector[stack.size - 1];
}

void remove_state(turtle_vector *stack)
{
	if (stack->size == 0) {
		return;
	}

	stack->size--;

	if (stack->size <= stack->capacity / 4 && stack->capacity >= 20) {
		turtle *temp = realloc(stack->vector,
						 (stack->capacity / 2) * sizeof(turtle));
		if (!temp) {
			perror("ERROR::REMOVE_STATE: Failed to reallocate memory!\n");
			return;
		}
		stack->vector = temp;
		stack->capacity /= 2;
	}
}

void clear_stack(turtle_vector *stack)
{
	free(stack->vector);
	stack->capacity = 0;
	stack->size = 0;
	stack->vector = NULL;
}

unsigned char **bitmap_matrix_malloc(long height, long width)
{
	if (height <= 0 || width <= 0) {
		return NULL;
	}

	unsigned char **result = malloc(height * sizeof(unsigned char *));
	if (!result) {
		perror("ERROR::BITMAP_MATRIX_MALLOC:"
			   " Failed to allocate memory for bitmap!\n");
		return NULL;
	}
	unsigned char *bitmap_block = malloc(height *
										 width *
										 sizeof(unsigned char));
	if (!bitmap_block) {
		perror("ERROR::BITMAP_MATRIX_MALLOC:"
			   " Failed to allocate memory for bitmap!\n");
		free(result);
		return NULL;
	}

	for (long i = 0; i < height; i++) {
		result[i] = &bitmap_block[i * width];
	}

	return result;
}

void bitmap_matrix_free(unsigned char **target)
{
	if (!target) {
		return;
	}
	free(target[0]);
	free(target);
}

void read_glyph_map(glyph *dest, FILE *src)
{
	long bytes_per_row = (dest->bbw + 7) / 8;
	dest->glyph_map = bitmap_matrix_malloc(dest->bbh, bytes_per_row * 8);

	for (long i = 0; i < dest->bbh; i++) {

		char *hex_line = get_file_line(src);
		if (!hex_line) {
			bitmap_matrix_free(dest->glyph_map);
			dest->glyph_map = NULL;
			return;
		}
		//iterate through the hex values
		for (long b = 0; b < bytes_per_row; b++) {
			unsigned int byte;
			if (sscanf(&hex_line[2 * b], "%2x", &byte) != 1) {
				perror("ERROR::READ_GLYPH_MAP:"
					   " Failed to read from string!\n");
			}

			//Unwrap bits
			for (int bit = 0; bit < 8; bit++) {
				dest->glyph_map[i][b * 8 + 7 - bit] = byte & 1;
				byte >>= 1;
			}
		}
		free(hex_line);
	}
}

void read_glyph(glyph *dest, FILE *src)
{
	char *start_keyword = get_file_word(src);
	if (!start_keyword) {
		return;
	}

	if (strcmp(start_keyword, "STARTCHAR") != 0) {
		free(start_keyword);
		return;
	}

	dest->char_name = get_file_line(src);
	free(start_keyword);

	//from now on attributes are in no particular order
	while (1) {
		char *line = get_file_line(src);
		char *line_cpy = string_dup(line);
		char *p = strtok(line_cpy, " ");

		if (strcmp(p, "ENCODING") == 0) {

			p = strtok(NULL, " ");
			if (p) {
				dest->code = atoi(p);
			}

		} else if (strcmp(p, "DWIDTH") == 0) {

			p = strtok(NULL, " ");
			if (p) {
				dest->dwx = atol(p);
			}

			p = strtok(NULL, " ");
			if (p) {
				dest->dwy = atol(p);
			}

		} else if (strcmp(p, "BBX") == 0) {

			p = strtok(NULL, " ");
			if (p) {
				dest->bbw = atol(p);
			}

			p = strtok(NULL, " ");
			if (p) {
				dest->bbh = atol(p);
			}

			p = strtok(NULL, " ");
			if (p) {
				dest->bbxoff = atol(p);
			}

			p = strtok(NULL, " ");
			if (p) {
				dest->bbyoff = atol(p);
			}

		} else if (strcmp(p, "BITMAP") == 0) {

			read_glyph_map(dest, src);

		} else if (strcmp(p, "ENDCHAR") == 0) {

			free(line);
			free(line_cpy);
			break;

		}

		free(line);
		free(line_cpy);
	}
}

glyph *get_glyph(font *f, char c)
{
	//search the glyph in font
	for (int i = 0; i < f->font_nmb && i <= 255; i++) {
		if (f->glyph_array[i].code == (int)c) {
			return &f->glyph_array[i];
		}
	}

	return NULL;
}

void type_text(ppm_image *target, font *f, char *text, long start_x,
			   long start_y, pixel color)
{
	long cursor_x = start_x;
	long cursor_y = start_y;

	long len = strlen(text);

	for (long i = 0; i < len; i++) {
		glyph *required_glyph = get_glyph(f, text[i]);
		if (!required_glyph) {
			continue;
		}

		for (long row = 0; row < required_glyph->bbh; row++) {
			for (long column = 0; column < required_glyph->bbw; column++) {
				if (required_glyph->glyph_map[row][column] == 1) {
					//we need to draw this pixel

					//calculate x/y position on image
					long x = cursor_x + required_glyph->bbxoff + column;

					long y = cursor_y + required_glyph->bbyoff +
							 required_glyph->bbh - row - 1;

					//check if pixel is wihin bounds
					if (x >= 0 && x < target->width &&
						y >= 0 && y < target->height) {
						draw_pixel(target, x, y, color);
					}
				}
			}
		}

		cursor_x += required_glyph->dwx;
		cursor_y += required_glyph->dwy;
	}
}

int load_font(font *target, char *file_name)
{
	memset(target, 0, sizeof(font));

	FILE *input = fopen(file_name, "rt");
	if (!input) {
		memset(target, 0, sizeof(font));
		return 0;
	}
	target->file_name = string_dup(file_name);

	while (1) {
		char *line = get_file_line(input);
		if (!line) {
			break;
		}
		char *line_cpy = string_dup(line);
		if (!line_cpy) {
			free(line);
			break;
		}

		char *p = strtok(line_cpy, " ");
		if (!p) {
			free(line);
			free(line_cpy);
			continue;
		}

		if (strcmp(p, "FONT") == 0) {

			//safety check for empty argument
			if (strlen(line) > 5) {
				target->font_name = string_dup(line + 5);
			}

		} else if (strcmp(p, "CHARS") == 0) {

			p = strtok(NULL, " ");
			if (p) {
				target->font_nmb = atol(p);
			}

			free(line);
			free(line_cpy);

			target->glyph_array = calloc(target->font_nmb, sizeof(glyph));
			if (!target->glyph_array) {
				perror("ERROR::LOAD_FONT: Failed to allocate memory!");
				if (target->file_name) {
					free(target->file_name);
				}
				if (target->font_name) {
					free(target->font_name);
				}

				memset(target, 0, sizeof(font));
				return 0;
			}

			for (long i = 0; i < target->font_nmb; i++) {
				read_glyph(&target->glyph_array[i], input);
			}

			break;
		}

		free(line);
		free(line_cpy);
	}

	fclose(input);

	return 1;
}

void unload_font(font *target)
{
	if (target->state == NEW_ENTRY) {
		//clear individual glyphs
		for (long i = 0 ; i < target->font_nmb; i++) {
			free(target->glyph_array[i].char_name);
			bitmap_matrix_free(target->glyph_array[i].glyph_map);
		}

		free(target->font_name);
		free(target->file_name);
		free(target->glyph_array);
	}

	memset(target, 0, sizeof(font));
}

void draw_pixel(ppm_image *target_image, long x, long y, pixel drawn_pixel)
{
	if (x < 0 ||
		y < 0 ||
		target_image->height <= y ||
		target_image->width <= x) {
		return;
	}
	target_image->image_grid[target_image->height - y - 1][x] = drawn_pixel;
}

void draw_line(ppm_image *target_image,
			   long x0, long y0,
			   long x1, long y1,
			   pixel target_color)
{
	long dx = abs_long(x1 - x0);
	long sx = (x0 < x1) ? 1 : -1;

	long dy = (-1) * abs_long(y1 - y0);
	long sy = (y0 < y1) ? 1 : -1;

	long err = dx + dy;

	while (1) {

		draw_pixel(target_image, x0, y0, target_color);

		if (x0 == x1 && y0 == y1) {
			break;
		}

		long e2 = 2 * err;

		if (e2 >= dy) {
			err += dy;
			x0 += sx;
		}

		if (e2 <= dx) {
			err += dx;
			y0 += sy;
		}
	}
}

void move_turtle(ppm_image *target_image, turtle *target_turtle,
				 pixel target_color, double step)
{
	//save the initial turtle position
	turtle initial_turtle = *target_turtle;

	//move the turtle
	target_turtle->x += step * cos(to_radians(target_turtle->angle));
	target_turtle->y += step * sin(to_radians(target_turtle->angle));

	//draw the line
	draw_line(target_image, round(initial_turtle.x), round(initial_turtle.y),
			  round(target_turtle->x), round(target_turtle->y), target_color);
}

void draw_turtle(ppm_image *target_image, lsystem system,
				 long x0, long y0, long step, double angle, double angle_step,
				 long n, pixel pixel_color)
{
	turtle current_turtle = {(double)x0, (double)y0, angle};

	char *instruction_set = get_derive(system, n);
	if (!instruction_set) {
		perror("ERROR::DRAW_TURTLE: Failed to load instruction set!\n");
		return;
	}
	long len = strlen(instruction_set);

	turtle_vector stack = {NULL, 0, 0};

	//iterate through the instruction set
	for (long i = 0; i < len; i++) {
		char instruction = instruction_set[i];
		switch (instruction) {
		case 'F':
			move_turtle(target_image, &current_turtle,
						pixel_color, (double)step);
			break;
		case '+':
			current_turtle.angle += angle_step;
			break;
		case '-':
			current_turtle.angle -= angle_step;
			break;
		case '[':
			add_state(&stack, current_turtle);
			break;
		case ']':
			current_turtle = get_state(stack);
			remove_state(&stack);
			break;
		default:
			break;
		}
	}

	clear_stack(&stack);

	free(instruction_set);
}

char *get_file_quote(FILE *src)
{
	long size = 10;
	long chpos = 0;

	int ch = fgetc(src);
	if (ch == EOF) {
		return NULL;
	}
	//we skip all space characters and "
	while (isspace(ch)) {
		ch = fgetc(src);
		if (ch == EOF) {
			return NULL;
		}
	}

	if (ch == '\"') {
		ch = fgetc(src);
	} else {
		return NULL;
	}

	char *string = malloc(size * sizeof(char));
	if (!string) {
		return NULL;
	}
	//this is where the word starts
	while (ch != EOF && ch != '\"') {
		if (chpos >= size - 1) {
			size *= 2;
			char *temp = realloc(string, size);
			if (!temp) {
				free(string);
				return NULL;
			}
			string = temp;
		}
		string[chpos++] = ch;
		ch = fgetc(src);
	}

	if (ch == EOF) {
		free(string);
		return NULL;
	}

	//add null character at the end
	string[chpos] =  '\0';
	return string;
}

char *get_file_word(FILE *src)
{
	long size = 10;
	long chpos = 0;

	int ch = fgetc(src);
	if (ch == EOF) {
		return NULL;
	}
	//we skip all space characters
	while (isspace(ch)) {
		ch = fgetc(src);
		if (ch == EOF) {
			return NULL;
		}
	}
	char *string = malloc(size * sizeof(char));
	if (!string) {
		return NULL;
	}
	//this is where the word starts
	while (!isspace(ch) && ch != EOF) {
		if (chpos >= size - 1) {
			size *= 2;
			char *temp = realloc(string, size);
			if (!temp) {
				free(string);
				return NULL;
			}
			string = temp;
		}
		string[chpos++] = ch;
		ch = fgetc(src);
	}
	//add null character at the end
	string[chpos] =  '\0';
	return string;
}

char *get_file_line(FILE *src)
{
	long size = 10;
	long chpos = 0;

	int ch = fgetc(src);
	if (ch == EOF) {
		return NULL;
	}

	//we skip all space characters
	while (isspace(ch)) {
		ch = fgetc(src);
		if (ch == EOF) {
			return NULL;
		}
	}

	char *string = malloc(size * sizeof(char));
	if (!string) {
		return NULL;
	}

	while (ch != '\n' && ch != EOF) {
		if (chpos >= size - 1) {
			size *= 2;
			char *temp = realloc(string, size);
			if (!temp) {
				free(string);
				return NULL;
			}
			string = temp;
		}
		string[chpos++] = ch;
		ch = fgetc(src);
	}

	//add null character at the end
	string[chpos] =  '\0';
	return string;
}

char *string_dup(char *src)
{
	char *new_string = malloc((strlen(src) + 1) * sizeof(char));
	if (!new_string) {
		return NULL;
	}
	strcpy(new_string, src);
	return new_string;
}

char *get_replacement(char c, lsystem system)
{
	for (long i = 0; i < system.rule_nmb; i++) {
		if (system.rules[i].predecessor == c) {
			return system.rules[i].successor;
		}
	}
	return NULL;
}

void derive_once(char **string, lsystem system)
{
	long string_len = strlen(*string);

	long new_string_size = string_len * 2;
	long new_string_pos = 0;

	char *new_string = malloc(new_string_size * sizeof(char));
	if (!new_string) {
		return;
	}

	char *replacement;

	//iterate through string
	for (long i = 0; i < string_len; i++) {
		replacement = get_replacement((*string)[i], system);

		//two cases, rule not found or rule found
		if (!replacement) {
			//we add the character unchanged
			while (new_string_pos >= new_string_size - 1) {
				//need to have space for null terminator
				char *temp = realloc(new_string, new_string_size * 2);
				if (!temp) {
					free(new_string);
					return;
				}
				new_string_size *= 2;
				new_string = temp;
			}
			new_string[new_string_pos++] = (*string)[i];

		} else {
			//we add the replacement onto the new_string
			long replacement_len = strlen(replacement);
			while (new_string_pos + replacement_len >= new_string_size) {
				//need to have space for null terminator
				char *temp = realloc(new_string, new_string_size * 2);
				if (!temp) {
					free(new_string);
					return;
				}
				new_string_size *= 2;
				new_string = temp;
			}
			memcpy(new_string + new_string_pos, replacement,
				   replacement_len * sizeof(char));
			new_string_pos += replacement_len;
		}
	}
	new_string[new_string_pos] = '\0';
	free(*string);
	*string = new_string;
}

char *get_derive(lsystem system, long n)
{
	if (!system.axiom) {
		return NULL;
	}
	//make a copy of the axiom
	char *axiom = malloc((strlen(system.axiom) + 1) * sizeof(char));
	strcpy(axiom, system.axiom);
	if (!axiom) {
		perror("ERROR::PRINT_DERIVE: Failed to allocate memory!\n");
		return NULL;
	}

	for (long i = 1; i <= n; i++) {
		derive_once(&axiom, system);
	}
	return axiom;
}

void print_derive(lsystem system, long n)
{
	if (!system.axiom) {
		printf("No L-system loaded\n");
		return;
	}
	//make a copy of the axiom
	char *axiom = malloc((strlen(system.axiom) + 1) * sizeof(char));
	strcpy(axiom, system.axiom);
	if (!axiom) {
		perror("ERROR::PRINT_DERIVE: Failed to allocate memory!\n");
		return;
	}

	for (long i = 1; i <= n; i++) {
		derive_once(&axiom, system);
	}
	printf("%s\n", axiom);
	free(axiom);
}

int load_lsystem(lsystem *system, char *file_name)
{
	FILE *input = fopen(file_name, "rt");

	if (!input) {
		system->axiom = NULL;
		system->rules = NULL;
		return 0;
	}

	//read axiom
	char *axiom = get_file_word(input);
	if (!axiom) {
		fclose(input);
		system->axiom = NULL;
		system->rules = NULL;
		return 0;
	}
	system->axiom = axiom;

	//read number of rules
	fscanf(input, " %ld", &system->rule_nmb);

	//allocate the memory
	system->rules = malloc(system->rule_nmb * sizeof(rule));
	if (!system->rules) {
		free(axiom);
		fclose(input);
		system->axiom = NULL;
		system->rules = NULL;
		return 0;
	}

	//read the rules
	for (long i = 0; i < system->rule_nmb; i++) {
		fscanf(input, " %c", &system->rules[i].predecessor);
		system->rules[i].successor = get_file_word(input);

		if (!system->rules[i].successor) {
			for (long j = 0; j < i; j++) {
				free(system->rules[j].successor);
			}
			free(axiom);
			free(system->rules);
			fclose(input);
			system->axiom = NULL;
			system->rules = NULL;
			return 0;
		}
	}

	system->file_name = malloc(strlen(file_name) + 1);
	if (system->file_name) {
		strcpy(system->file_name, file_name);
	}

	fclose(input);
	return 1;
}

void unload_lsystem(lsystem *system)
{
	if (!system) {
		return;
	}
	if (system->state == NEW_ENTRY) {
		if (system->rules) {
			for (long i = 0; i < system->rule_nmb; i++) {
				free(system->rules[i].successor);
			}
			free(system->rules);
			system->rules = NULL;
		}
	free(system->axiom);
	free(system->file_name);
	}

	memset(system, 0, sizeof(lsystem));
}

command_keyword get_command_index(char *string)
{
	if (strcmp(string, "EXIT") == 0) {
		return EXIT;
	}
	if (strcmp(string, "UNDO") == 0) {
		return UNDO;
	}
	if (strcmp(string, "REDO") == 0) {
		return REDO;
	}
	if (strcmp(string, "LSYSTEM") == 0) {
		return LSYSTEM;
	}
	if (strcmp(string, "DERIVE") == 0) {
		return DERIVE;
	}
	if (strcmp(string, "LOAD") == 0) {
		return LOAD;
	}
	if (strcmp(string, "SAVE") == 0) {
		return SAVE;
	}
	if (strcmp(string, "TURTLE") == 0) {
		return TURTLE;
	}
	if (strcmp(string, "FONT") == 0) {
		return FONT;
	}
	if (strcmp(string, "TYPE") == 0) {
		return TYPE;
	}
	if (strcmp(string, "BITCHECK") == 0) {
		return BITCHECK;
	}
	return -1;
}

int load_image(ppm_image *image, char *file_name)
{
	FILE *input = fopen(file_name, "rb");
	if (!input) {
		memset(image, 0, sizeof(ppm_image));
		return 0;
	}

	image->file_name = string_dup(file_name);
	if (!image->file_name) {
		perror("ERROR::LOAD_IMAGE: Failed to allocate memory for duplicate!\n");
		memset(image, 0, sizeof(ppm_image));
		fclose(input);
		return 0;
	}

	char *header = get_file_word(input);
	if (strcmp(header, "P6") != 0) {
		free(image->file_name);
		free(header);
		memset(image, 0, sizeof(ppm_image));
		fclose(input);
		return 0;
	}
	free(header);
	fscanf(input, " %ld", &image->width);
	fscanf(input, " %ld", &image->height);

	int maxval;
	fscanf(input, " %d", &maxval);
	fgetc(input); //consume the newline character

	//allocate the memory
	image->image_grid = image_matrix_malloc(image->height, image->width);
	if (!image->image_grid) {
		free(image->file_name);
		memset(image, 0, sizeof(ppm_image));
		fclose(input);
		return 0;
	}

	fread(image->image_grid[0], sizeof(pixel),
		  image->height * image->width, input);

	image->state = NEW_ENTRY;

	fclose(input);

	return 1;
}

ppm_image get_ppm_image_copy(ppm_image *src)
{
	ppm_image copy_image;
	memcpy(&copy_image, src, sizeof(ppm_image));

	copy_image.file_name = malloc((strlen(src->file_name) + 1) * sizeof(char));
	if (!copy_image.file_name) {
		perror("ERROR::GET_PPM_IMAGE_COPY: Failed to allocate memory!\n");
		memset(&copy_image, 0, sizeof(ppm_image));
		return copy_image;
	}
	strcpy(copy_image.file_name, src->file_name);

	copy_image.image_grid = image_matrix_copy(src->image_grid,
											  copy_image.height,
											  copy_image.width);
	copy_image.state = NEW_ENTRY;

	return copy_image;
}

void unload_image(ppm_image *image)
{
	if (image->state == NEW_ENTRY) {
		image_matrix_free(image->image_grid);
		free(image->file_name);
	}

	memset(image, 0, sizeof(ppm_image));
}

void save_image(ppm_image *image, char *file_image)
{
	FILE *output = fopen(file_image, "wb");
	if (!output) {
		perror("ERROR::SAVE_IMAGE: Failed to open file for writing!\n");
		return;
	}

	//write the ascii data
	fprintf(output, "P6\n%ld %ld\n255\n", image->width, image->height);
	size_t raster_size = image->width * image->height;
	fwrite(image->image_grid[0], sizeof(pixel), raster_size, output);

	fclose(output);
}

void print_bitcheck_warning(int *arr, long x, long y)
{
	unsigned char *rgb = calloc(3, sizeof(char));
	if (!rgb) {
		perror("ERROR::PRINT_BITCHECK_WARNING: Failed to allocate memory!\n");
		return;
	}

	for (int current_char = 0; current_char < 3; current_char++) {
		int z = 1;

		for (int i = 0; i < 8; i++) {
			rgb[current_char] += arr[(current_char + 1) * 8 - 1 - i] * z;
			z <<= 1;
		}
	}

	printf("Warning: pixel at (%ld, %ld) may be read as (%d, %d, %d)\n",
		   x, y, (int)rgb[0], (int)rgb[1], (int)rgb[2]);

	free(rgb);
}

void set_pixel_to_array(int *array, pixel src)
{
	for (int i = 0; i < 8; i++) {
		array[10 - i] = src.R & 1;
		array[18 - i] = src.G & 1;
		array[26 - i] = src.B & 1;

		src.R >>= 1;
		src.G >>= 1;
		src.B >>= 1;
	}
}

void check_bitcheck_sequence(ppm_image *image, int *array,
							 int x, int y, int z, int t,
							 int i, int j)
{
	if (array[x] == 0 &&
		array[y] == 0 &&
		array[z] == 1 &&
		array[t] == 0) {
		array[z] = 0;
		print_bitcheck_warning(array + 3, j, image->height - 1 - i);
		array[z] = 1;
	}
	if (array[x] == 1 &&
		array[y] == 1 &&
		array[z] == 0 &&
		array[t] == 1) {
		array[z] = 1;
		print_bitcheck_warning(array + 3, j, image->height - 1 - i);
		array[z] = 0;
	}
}

void bitcheck(ppm_image *image)
{
	int *array = malloc(30 * sizeof(int));
	if (!array) {
		perror("ERROR::BITCHECK: Failed to allocate memory!\n");
		return;
	}

	pixel current_pixel;
	unsigned char previous_B;
	unsigned char next_R;

	for (long i = 0; i < image->height; i++) {
		for (long j = 0; j < image->width; j++) {
			current_pixel = image->image_grid[i][j];

			set_pixel_to_array(array, current_pixel);

			//get a peek at the previous and next byte
			if (i != 0 || j != 0) {
				if (j == 0) {
					previous_B = image->image_grid[i - 1][image->width - 1].B;
				} else {
					previous_B = image->image_grid[i][j - 1].B;
				}
			}
			if (i != image->height - 1 || j != image->width - 1) {
				if (j == image->width - 1) {
					next_R = image->image_grid[i + 1][0].R;
				} else {
					next_R = image->image_grid[i][j + 1].R;
				}
			}

			next_R >>= 5;
			for (int t = 0; t < 3; t++) {
				if (i != 0 || j != 0) {
					array[2 - t] = previous_B & 1;
				}
				if (i != image->height - 1 || j != image->width - 1) {
					array[29 - t] = next_R & 1;
				}
				previous_B >>= 1;
				next_R >>= 1;
			}

			int x = 0, y = 1, z = 2, t = 3;
			if (i == 0 && j == 0) {
				x = 3, y = 4, z = 5, t = 6;
			}

			int last_t = 29;
			if (i == (image->height - 1) && j == (image->width - 1)) {
				last_t -= 2;
			}

			while (t < last_t) {
				//wont matter if z is outside the current pixel
				if (z < 3 || z > 26) {
					x++; y++; z++; t++;
					continue;
				}

				check_bitcheck_sequence(image, array, x, y, z, t, i, j);
				x++; y++; z++; t++;
			}
		}
	}

	free(array);
}
