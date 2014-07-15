/* -- gstdlib.c -- a stack based programming language */
#include <stdlib.h>
#include <string.h>
#include "gstack.h"

void add_prim(gstate_t* state)
{
	gobject_t* n2 = gpop_expect(state, OBJ_NUMBER);
	gobject_t* n1 = gpop_expect(state, OBJ_NUMBER);
	gpush_number(state, n1->number.value + n2->number.value);
}

void sub_prim(gstate_t* state)
{
	gobject_t* n2 = gpop_expect(state, OBJ_NUMBER);
	gobject_t* n1 = gpop_expect(state, OBJ_NUMBER);
	gpush_number(state, n1->number.value - n2->number.value);
}

void mul_prim(gstate_t* state)
{
	gobject_t* n2 = gpop_expect(state, OBJ_NUMBER);
	gobject_t* n1 = gpop_expect(state, OBJ_NUMBER);
	gpush_number(state, n1->number.value * n2->number.value);
}

void div_prim(gstate_t* state)
{
	gobject_t* n2 = gpop_expect(state, OBJ_NUMBER);
	gobject_t* n1 = gpop_expect(state, OBJ_NUMBER);
	gpush_number(state, n1->number.value / n2->number.value);
}

void cons_prim(gstate_t* state)
{
	gpush_pair(state);
}

static void print_obj(gobject_t* obj)
{
	switch(obj->type)
	{
	case OBJ_NUMBER:
		printf("%f\n", obj->number.value);
		break;
	case OBJ_BOOLEAN:
		printf("%s\n", obj->boolean.value ? "true" : "false");
		break;
	case OBJ_STRING:
		printf("%s\n", obj->string.value);
		break;
	case OBJ_PAIR:
		print_obj(obj->pair.head);
		print_obj(obj->pair.tail);
		break;
	case OBJ_NATIVE:
		printf("native pointer\n"); 
		break;
	}
}

void println_prim(gstate_t* state)
{
	gobject_t* obj = gpop_object(state);
	print_obj(obj);
}

void putchar_prim(gstate_t* state)
{
	gobject_t* obj = gpop_expect(state, OBJ_NUMBER);
	putchar((char)obj->number.value);
}

void dup_prim(gstate_t* state)
{
	gobject_t* obj = gpeek_object(state);
	gpush_object(state, obj);
}

void drop_prim(gstate_t* state)
{
	gpop_object(state);
}

static char are_objects_equal(gobject_t* o1, gobject_t* o2)
{
	if(o1->type != o2->type) gfatal_error("attempted to compare equality between 2 objects of different types (%i and %i)\n", o1->type, o2->type);
	switch(o1->type)
	{
	case OBJ_NUMBER:
		return o1->number.value == o2->number.value;
	case OBJ_BOOLEAN:
		return o1->boolean.value == o2->boolean.value;
	case OBJ_STRING:
		return (strcmp(o1->string.value, o2->string.value) == 0);
	case OBJ_PAIR:
		return (are_objects_equal(o1->pair.head, o2->pair.head) && are_objects_equal(o1->pair.tail, o2->pair.tail));
	case OBJ_NATIVE:
		return o1->native.value == o2->native.value;
	}
	return 0;
}

void equals_prim(gstate_t* state)
{
	gobject_t* o2 = gpop_object(state);
	gobject_t* o1 = gpop_object(state);
	gpush_boolean(state, are_objects_equal(o1, o2));
}

void head_prim(gstate_t* state)
{
	gobject_t* pair = gpeek_expect(state, OBJ_PAIR);
	gpush_object(state, pair->pair.head);
}

void tail_prim(gstate_t* state)
{
	gobject_t* pair = gpeek_expect(state, OBJ_PAIR);
	gpush_object(state, pair->pair.tail);
}

void gc_prim(gstate_t* state)
{
	gcollect_garbage(state);
}

void show_prim(gstate_t* state)
{
	long stack_loc = state->stack_size - 1;
	while(stack_loc >= 0)
	{	
		printf("stack location %ld\n----------------\n", stack_loc);
		print_obj(state->stack[stack_loc--]);
	}
}

void getchar_prim(gstate_t* state)
{
	gpush_number(state, getchar());
}

void gt_prim(gstate_t* state)
{
	gobject_t* o2 = gpop_expect(state, OBJ_NUMBER);
	gobject_t* o1 = gpop_expect(state, OBJ_NUMBER);
	gpush_boolean(state, o1->number.value > o2->number.value);
}

void lt_prim(gstate_t* state)
{
	gobject_t* o2 = gpop_expect(state, OBJ_NUMBER);
	gobject_t* o1 = gpop_expect(state, OBJ_NUMBER);
	gpush_boolean(state, o1->number.value < o2->number.value);
}

void gte_prim(gstate_t* state)
{
	gobject_t* o2 = gpop_expect(state, OBJ_NUMBER);
	gobject_t* o1 = gpop_expect(state, OBJ_NUMBER);
	gpush_boolean(state, o1->number.value >= o2->number.value);
}

void lte_prim(gstate_t* state)
{
	gobject_t* o2 = gpop_expect(state, OBJ_NUMBER);
	gobject_t* o1 = gpop_expect(state, OBJ_NUMBER);
	gpush_boolean(state, o1->number.value <= o2->number.value);
}

void floor_prim(gstate_t* state)
{
	gobject_t* num = gpop_expect(state, OBJ_NUMBER);
	gpush_number(state, (int)num->number.value);
}

void ceil_prim(gstate_t* state)
{
	gobject_t* num = gpop_expect(state, OBJ_NUMBER);
	gpush_number(state, (int)num->number.value + 1);
}

void mod_prim(gstate_t* state)
{
	gobject_t* modval = gpop_expect(state, OBJ_NUMBER);
	gobject_t* num = gpop_expect(state, OBJ_NUMBER);
	gpush_number(state, (int)num->number.value % (int)modval->number.value); 
}

void file_on_gc(void** value)
{
	fclose((FILE*)(*value));
	(*value) = NULL;
}

void file_open_prim(gstate_t* state)
{
	gobject_t* mode = gpop_expect(state, OBJ_STRING);
	gobject_t* path = gpop_expect(state, OBJ_STRING);
	FILE* file = fopen(path->string.value, mode->string.value);
	if(!file) gfatal_error("file open (%s with mode %s) failed\n", path->string.value, mode->string.value);
	gpush_native(state, file, NULL, NULL, file_on_gc);
}

void file_getchar_prim(gstate_t* state)
{
	gobject_t* file = gpop_expect(state, OBJ_NATIVE);
	FILE* nfile = (FILE*)file->native.value;
	gpush_number(state, fgetc(nfile));
}

static gprimitivereg_t standard_library[] =
{
	{"add", add_prim},
	{"sub", sub_prim},
	{"mul", mul_prim},
	{"div", div_prim},
	{"cons", cons_prim},
	{"debug.println", println_prim},
	{"putchar", putchar_prim},
	{"dup", dup_prim},
	{"drop", drop_prim},
	{"equals", equals_prim},
	{"head", head_prim},
	{"tail", tail_prim},
	{"gc", gc_prim},
	{"show", show_prim},
	{"getchar", getchar_prim},
	{"gt", gt_prim},
	{"lt", lt_prim},
	{"gte", gte_prim},
	{"lte", lte_prim},
	{"floor", floor_prim},
	{"ceil", ceil_prim},
	{"mod", mod_prim},
	{"file.open", file_open_prim},
	{"file.getchar", file_getchar_prim},
	{"", NULL}
};

void gload_stdlib(gstate_t* state)
{
	gadd_primitivelib(state, standard_library);
}