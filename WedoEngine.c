#include "WedoEngine.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

#include "ferite.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "ini.h"

#ifndef FALSE
	#define FALSE 0
	#define TRUE !FALSE
#endif

#define NOT !
#define AND &&
#define OR ||

#define OBJECT_SET_NUMBER_LONG_VAR( SCRIPT, VARIABLE, NAME, VALUE ) \
	ferite_object_set_var(SCRIPT, VAO(VARIABLE), NAME, ferite_create_number_long_variable(SCRIPT, NAME, VALUE, FE_STATIC));

#define OBJECT_SET_STRING_VAR( SCRIPT, VARIABLE, NAME, VALUE ) \
	ferite_object_set_var(SCRIPT, VAO(VARIABLE), NAME, ferite_create_string_variable_from_ptr(SCRIPT, NAME, VALUE, 0, FE_CHARSET_DEFAULT, FE_STATIC))

static SDL_Window   *_WedoEngine_Window = NULL;
static SDL_Renderer *_WedoEngine_Renderer = NULL;

static FeriteVariable *_WedoEngine_CreateFeriteKeyboardEvent( FeriteScript *script, FeriteNamespace *namespace, SDL_Event *event ) {
	FeriteNamespaceBucket *nsb = ferite_find_namespace(script, namespace, "KeyboardEvent", FENS_CLS);
	FeriteVariable *event_variable = ferite_build_object(script, nsb->data);

	OBJECT_SET_NUMBER_LONG_VAR(script, event_variable, "type", event->key.type);
	OBJECT_SET_NUMBER_LONG_VAR(script, event_variable, "key", event->key.keysym.sym);

	return event_variable;
}

static FeriteVariable *_WedoEngine_CreateFeriteMouseButtonEvent( FeriteScript *script, FeriteNamespace *namespace, SDL_Event *event ) {
	FeriteNamespaceBucket *nsb = ferite_find_namespace(script, namespace, "MouseButtonEvent", FENS_CLS);
	FeriteVariable *event_variable = ferite_build_object(script, nsb->data);

	OBJECT_SET_NUMBER_LONG_VAR(script, event_variable, "type", event->button.type);
	OBJECT_SET_NUMBER_LONG_VAR(script, event_variable, "device", event->button.which);
	OBJECT_SET_NUMBER_LONG_VAR(script, event_variable, "button", event->button.button);
	OBJECT_SET_NUMBER_LONG_VAR(script, event_variable, "x", event->button.x);
	OBJECT_SET_NUMBER_LONG_VAR(script, event_variable, "y", event->button.y);

	return event_variable;
}

static FeriteVariable *_WedoEngine_CreateFeriteMouseMotionEvent( FeriteScript *script, FeriteNamespace *namespace, SDL_Event *event )  {
	FeriteNamespaceBucket *nsb = ferite_find_namespace(script, namespace, "MouseMotionEvent", FENS_CLS);
	FeriteVariable *event_variable = ferite_build_object(script, nsb->data);

	OBJECT_SET_NUMBER_LONG_VAR(script, event_variable, "type", event->motion.type);
	OBJECT_SET_NUMBER_LONG_VAR(script, event_variable, "device", event->motion.which);
	OBJECT_SET_NUMBER_LONG_VAR(script, event_variable, "x", event->motion.x);
	OBJECT_SET_NUMBER_LONG_VAR(script, event_variable, "y", event->motion.y);
	OBJECT_SET_NUMBER_LONG_VAR(script, event_variable, "xRelative", event->motion.xrel);
	OBJECT_SET_NUMBER_LONG_VAR(script, event_variable, "yRelative", event->motion.yrel);

	return event_variable;
}

static FeriteVariable *_WedoEngine_CreateFeriteQuitEvent( FeriteScript *script, FeriteNamespace *namespace, SDL_Event *event ) {
	FeriteNamespaceBucket *nsb = ferite_find_namespace(script, namespace, "QuitEvent", FENS_CLS);
	FeriteVariable *event_variable = ferite_build_object(script, nsb->data);
	OBJECT_SET_NUMBER_LONG_VAR(script, event_variable, "type", event->quit.type);
	return event_variable;
}

static FeriteVariable *_WedoEngine_CreateFeriteUnhandledEvent( FeriteScript *script, FeriteNamespace *namespace, SDL_Event *event ) {
	FeriteNamespaceBucket *nsb = ferite_find_namespace(script, namespace, "UnhandledEvent", FENS_CLS);
	FeriteVariable *event_variable = ferite_build_object(script, nsb->data);
	OBJECT_SET_NUMBER_LONG_VAR(script, event_variable, "type", event->type);
	return event_variable;
}

typedef struct __WedoEngine_ParseINI {
	FeriteScript *script;
	FeriteUnifiedArray *array;
} WedoEngine_ParseINI;

static int _WedoEngine_ParseINIHandler( void *user, const char *section, const char *name, const char *value ) {
	WedoEngine_ParseINI *data = user;
	FeriteNamespaceBucket *nsb = NULL;
	FeriteNamespace *engine_namespace = NULL;
	FeriteVariable *ini_variable = NULL;

	nsb = ferite_find_namespace(data->script, data->script->mainns, "Engine", FENS_NS);
	engine_namespace = nsb->data;

	nsb = ferite_find_namespace(data->script, engine_namespace, "INI", FENS_CLS);
	ini_variable = ferite_build_object(data->script, nsb->data);

	OBJECT_SET_STRING_VAR(data->script, ini_variable, "section", (char *)section);
	OBJECT_SET_STRING_VAR(data->script, ini_variable, "name", (char *)name);
	OBJECT_SET_STRING_VAR(data->script, ini_variable, "value", (char *)value);

	ferite_uarray_add(data->script, data->array, ini_variable, NULL, FE_ARRAY_ADD_AT_END);

	return 1;
}

static int _WedoEngine_InitializeSDL() {
	Uint32 flags = SDL_INIT_VIDEO | SDL_INIT_EVENTS;
	if ( SDL_Init(flags) < 0 )
		return FALSE;
	if( TTF_Init() < 0 ) {
		SDL_Quit();
		return FALSE;
	}
	return TRUE;
}

static int _WedoEngine_InitializeENet() {
	return TRUE;
}

static int _WedoEngine_InitializeFerite() {
	if( ferite_init(0, NULL) )
		return TRUE;
	return FALSE;
}

static void _WedoEngine_TerminateSDL() {
	if( _WedoEngine_Renderer ) {
		SDL_DestroyRenderer(_WedoEngine_Renderer);
		_WedoEngine_Renderer = NULL;
	}
	if( _WedoEngine_Window ) {
		SDL_DestroyWindow(_WedoEngine_Window);
		_WedoEngine_Window = NULL;
	}

	TTF_Quit();
	SDL_Quit();
}

static void _WedoEngine_TerminateENet() {

}

static void _WedoEngine_TerminateFerite() {
	ferite_deinit();
}

static void _WedoEngine_SetWindow( SDL_Window *window ) {
	_WedoEngine_Window = window;
}

static void _WedoEngine_SetRenderer( SDL_Renderer *renderer ) {
	_WedoEngine_Renderer = renderer;
}

static SDL_Window *_WedoEngine_GetWindow() {
	return _WedoEngine_Window;
}

static SDL_Renderer *_WedoEngine_GetRenderer() {
	return _WedoEngine_Renderer;
}

#include "WedoEngineFerite.c.in"
#include "WedoEngineFeriteArray.c.in"
#include "WedoEngineFeriteNumber.c.in"
#include "WedoEngineFeriteString.c.in"
#include "WedoEngineFeriteImage.c.in"
#include "WedoEngineFeriteTexture.c.in"
#include "WedoEngineFeriteFont.c.in"

static void _WedoEngine_RegisterFeriteFunctions( FeriteScript *script) {
	FeriteNamespaceBucket *nsb = ferite_find_namespace(script, script->mainns, "Engine", FENS_NS);
	FeriteNamespace *engine_namespace = (nsb AND nsb->data ? nsb->data : ferite_register_namespace(script, "Engine", script->mainns));
	FeriteNamespace *array_namespace = NULL;
	FeriteNamespace *number_namespace = NULL;
	FeriteNamespace *string_namespace = NULL;
	FeriteClass *image_class = ferite_register_inherited_class(script, engine_namespace, "Image", NULL);
	FeriteClass *texture_class = ferite_register_inherited_class(script, engine_namespace, "Texture", NULL);
	FeriteClass *font_class = ferite_register_inherited_class(script, engine_namespace, "Font", NULL);

	nsb = ferite_find_namespace(script, script->mainns, "Array", FENS_NS);
	array_namespace = (nsb && nsb->data ? nsb->data : ferite_register_namespace(script, "Array", script->mainns));

	nsb = ferite_find_namespace(script, script->mainns, "Number", FENS_NS);
	number_namespace = (nsb && nsb->data ? nsb->data : ferite_register_namespace(script, "Number", script->mainns));

	nsb = ferite_find_namespace(script, script->mainns, "String", FENS_NS);
	string_namespace = (nsb && nsb->data ? nsb->data : ferite_register_namespace(script, "String", script->mainns));

	#define REGISTER_NAMESPACE_NUMBER_LONG_VAR( SCRIPT, NAMESPACE, NAME, VALUE ) \
		ferite_register_ns_variable(SCRIPT, NAMESPACE, NAME, ferite_create_number_long_variable(SCRIPT, NAME, VALUE, FE_STATIC));	
	#define REGISTER_NAMESPACE_FUNCTION( SCRIPT, NAMESPACE, NAME, PARAMETERS, FUNCTION ) \
		ferite_register_ns_function(SCRIPT, NAMESPACE, ferite_create_external_function(SCRIPT, NAME, FUNCTION, PARAMETERS));
	#define REGISTER_CLASS_FUNCTION( SCRIPT, CLASS, NAME, PARAMETERS, FUNCTION ) \
		ferite_register_class_function(SCRIPT, CLASS, ferite_create_external_function(SCRIPT, NAME, FUNCTION, PARAMETERS), FE_FALSE);

	REGISTER_NAMESPACE_FUNCTION(script, array_namespace, "size", "a", _WedoEngine_FeriteArraySize);
	REGISTER_NAMESPACE_FUNCTION(script, array_namespace, "keyExists", "as", _WedoEngine_FeriteArrayKeyExists);
	REGISTER_NAMESPACE_FUNCTION(script, array_namespace, "deleteAt", "an", _WedoEngine_FeriteArrayDeleteAt);

	REGISTER_NAMESPACE_FUNCTION(script, number_namespace, "round", "n", _WedoEngine_FeriteNumberRound);
	REGISTER_NAMESPACE_FUNCTION(script, number_namespace, "floor", "n", _WedoEngine_FeriteNumberFloor);
	REGISTER_NAMESPACE_FUNCTION(script, number_namespace, "numberToByte", "n", _WedoEngine_FeriteNumberToByte);

	REGISTER_NAMESPACE_FUNCTION(script, string_namespace, "length", "s", _WedoEngine_FeriteStringLength);
	REGISTER_NAMESPACE_FUNCTION(script, string_namespace, "byteToNumber", "s", _WedoEngine_FeriteStringByteToNumber);
	REGISTER_NAMESPACE_FUNCTION(script, string_namespace, "toNumber", "s", _WedoEngine_FeriteStringToNumber);
	REGISTER_NAMESPACE_FUNCTION(script, string_namespace, "toLower", "s", _WedoEngine_FeriteStringToLower);

	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "MESSAGEBOX_ERROR", SDL_MESSAGEBOX_ERROR);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "MESSAGEBOX_WARNING", SDL_MESSAGEBOX_WARNING);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "MESSAGEBOX_INFORMATION", SDL_MESSAGEBOX_INFORMATION);

	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "EVENT_KEYDOWN", SDL_KEYDOWN);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "EVENT_KEYUP", SDL_KEYUP);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "EVENT_MOUSEBUTTONDOWN", SDL_MOUSEBUTTONDOWN);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "EVENT_MOUSEBUTTONUP", SDL_MOUSEBUTTONUP);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "EVENT_MOUSEMOTION", SDL_MOUSEMOTION);	
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "EVENT_QUIT", SDL_QUIT);

	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "KEY_RETURN", SDLK_RETURN);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "KEY_ESCAPE", SDLK_ESCAPE);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "KEY_BACKSPACE", SDLK_BACKSPACE);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "KEY_SPACE", SDLK_SPACE);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "KEY_TAB", SDLK_TAB);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "KEY_CAPSLOCK", SDLK_CAPSLOCK);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "KEY_0", SDLK_0);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "KEY_1", SDLK_1);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "KEY_2", SDLK_2);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "KEY_3", SDLK_3);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "KEY_4", SDLK_4);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "KEY_5", SDLK_5);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "KEY_6", SDLK_6);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "KEY_7", SDLK_7);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "KEY_8", SDLK_8);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "KEY_9", SDLK_9);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "KEY_RIGHT", SDLK_RIGHT);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "KEY_LEFT", SDLK_LEFT);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "KEY_UP", SDLK_UP);
	REGISTER_NAMESPACE_NUMBER_LONG_VAR(script, engine_namespace, "KEY_DOWN", SDLK_DOWN);

	REGISTER_NAMESPACE_FUNCTION(script, engine_namespace, "printLine", "s", _WedoEngine_FeritePrintLine);
	REGISTER_NAMESPACE_FUNCTION(script, engine_namespace, "showSimpleMessageBox", "nss", _WedoEngine_FeriteShowSimpleMessageBox);
	REGISTER_NAMESPACE_FUNCTION(script, engine_namespace, "createWindow", "snn", _WedoEngine_FeriteCreateWindow);
	REGISTER_NAMESPACE_FUNCTION(script, engine_namespace, "delay", "n", _WedoEngine_FeriteDelay);
	REGISTER_NAMESPACE_FUNCTION(script, engine_namespace, "clearScreen", "", _WedoEngine_FeriteClearScreen);
	REGISTER_NAMESPACE_FUNCTION(script, engine_namespace, "updateScreen", "", _WedoEngine_FeriteUpdateScreen);
	REGISTER_NAMESPACE_FUNCTION(script, engine_namespace, "nextEvent", "", _WedoEngine_FeriteNextEvent);
	REGISTER_NAMESPACE_FUNCTION(script, engine_namespace, "currentTime", "", _WedoEngine_FeriteCurrentTime);
	REGISTER_NAMESPACE_FUNCTION(script, engine_namespace, "loadImage", "s", _WedoEngine_FeriteLoadImage);
	REGISTER_NAMESPACE_FUNCTION(script, engine_namespace, "loadTexture", "s", _WedoEngine_FeriteLoadTexture);
	REGISTER_NAMESPACE_FUNCTION(script, engine_namespace, "loadFont", "sn", _WedoEngine_FeriteLoadFont);
	REGISTER_NAMESPACE_FUNCTION(script, engine_namespace, "renderTexture", "onnnn", _WedoEngine_FeriteRenderTexture);
	REGISTER_NAMESPACE_FUNCTION(script, engine_namespace, "renderText", "osnnnnn", _WedoEngine_FeriteRenderText);
	REGISTER_NAMESPACE_FUNCTION(script, engine_namespace, "hasIntersection", "nnnnnnnn", _WedoEngine_FeriteHasIntersection);
	REGISTER_NAMESPACE_FUNCTION(script, engine_namespace, "parseINI", "s", _WedoEngine_FeriteParseINI);
	REGISTER_NAMESPACE_FUNCTION(script, engine_namespace, "writeToFile", "ss", _WedoEngine_FeriteWriteToFile);

	REGISTER_CLASS_FUNCTION(script, image_class, "destructor", "", _WedoImage_FeriteDestructor);
	REGISTER_CLASS_FUNCTION(script, image_class, "toTexture", "", _WedoImage_FeriteToTexture);
	REGISTER_CLASS_FUNCTION(script, image_class, "getColor", "nnn", _WedoImage_FeriteGetColor);
	REGISTER_CLASS_FUNCTION(script, image_class, "getPixel", "nn", _WedoImage_FeriteGetPixel);

	REGISTER_CLASS_FUNCTION(script, texture_class, "destructor", "", _WedoTexture_FeriteDestructor);

	REGISTER_CLASS_FUNCTION(script, font_class, "destructor", "", _WedoFont_FeriteDestructor);
}

void WedoEngine_Terminate() {
	_WedoEngine_TerminateSDL();
	_WedoEngine_TerminateENet();
	_WedoEngine_TerminateFerite();
}

int WedoEngine_Initialize() {
	if( _WedoEngine_InitializeSDL() == FALSE )
		return FALSE;

	if( _WedoEngine_InitializeENet() == FALSE ) {
		_WedoEngine_TerminateSDL();
		return FALSE;
	}

	if( _WedoEngine_InitializeFerite() == FALSE ) {
		_WedoEngine_TerminateSDL();
		_WedoEngine_TerminateENet();
		return FALSE;
	}

	srand(time(NULL));

	return TRUE;
}


int WedoEngine_ExecuteScript( int argc, char *argv[], char *name ) {
	FeriteScript *script = NULL;
	char *error_message = NULL;
	int success = TRUE;

	ferite_set_script_argv(argc, argv);

	script = ferite_script_compile(name);

	if( ferite_has_compile_error(script) ) {
		error_message = ferite_get_error_log(script);
		fprintf(stderr, "[ferite: compile] %s", error_message);
		success = FALSE;
	} else {
		_WedoEngine_RegisterFeriteFunctions(script);

		ferite_script_execute(script);

		if( ferite_has_runtime_error(script) ) {
			error_message = ferite_get_error_log(script);
			fprintf(stderr, "[ferite: execution] %s", error_message);
			success = FALSE;
		}
	}

	ferite_script_delete(script);

	if( error_message )
		ffree(error_message);

	return success;
}

