#ifndef WEDOENGINE_H
#define WEDOENGINE_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "ferite.h"
#include "SDL.h"
#include "SDL_image.h"

#ifndef FALSE
	#define FALSE 0
	#define TRUE !FALSE
#endif

#define NOT !
#define AND &&
#define OR ||

#define OBJECT_SET_NUMBER_LONG_VAR( SCRIPT, VARIABLE, NAME, VALUE ) \
	ferite_object_set_var(SCRIPT, VAO(VARIABLE), NAME, ferite_create_number_long_variable(SCRIPT, NAME, VALUE, FE_STATIC));

FeriteVariable *_WedoEngine_CreateFeriteKeyboardEvent( FeriteScript *script, FeriteNamespace *namespace, SDL_Event *event );
FeriteVariable *_WedoEngine_CreateFeriteMouseButtonEvent( FeriteScript *script, FeriteNamespace *namespace, SDL_Event *event );
FeriteVariable *_WedoEngine_CreateFeriteMouseMotionEvent( FeriteScript *script, FeriteNamespace *namespace, SDL_Event *event );
FeriteVariable *_WedoEngine_CreateFeriteQuitEvent( FeriteScript *script, FeriteNamespace *namespace, SDL_Event *event );
FeriteVariable *_WedoEngine_CreateFeriteUnhandledEvent( FeriteScript *script, FeriteNamespace *namespace, SDL_Event *event );

void _WedoEngine_SetWindow( SDL_Window *window );
void _WedoEngine_SetRenderer( SDL_Renderer *renderer );

SDL_Window   *_WedoEngine_GetWindow();
SDL_Renderer *_WedoEngine_GetRenderer();

int  WedoEngine_Initialize();
void WedoEngine_Terminate();

int  WedoEngine_ExecuteScript( int argc, char *argv[], char *name );

#endif /* WEDOENGINE_H */