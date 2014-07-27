#include "WedoEngine.h"

FE_NATIVE_FUNCTION( _WedoEngine_FeritePrintLine ) {
	FeriteString *str = NULL;
	ferite_get_parameters(params, 1, &str);
	printf("%s\n", str->data);
	FE_RETURN_VOID;
}

FE_NATIVE_FUNCTION( _WedoEngine_FeriteShowSimpleMessageBox ) {
	double flags = 0.0;
	FeriteString *title = NULL;
	FeriteString *message = NULL;
	ferite_get_parameters(params, 3, &flags, &title, &message);
	SDL_ShowSimpleMessageBox((Uint32)flags, title->data, message->data, _WedoEngine_GetWindow());
	FE_RETURN_VOID;
}

FE_NATIVE_FUNCTION( _WedoEngine_FeriteDelay ) {
	double ms = 0.0;
	ferite_get_parameters(params, 1, &ms);
	SDL_Delay((Uint32)ms);
	FE_RETURN_VOID;
}

FE_NATIVE_FUNCTION( _WedoEngine_FeriteCreateWindow ) {
	FeriteString *name = NULL;
	double width = 0.0;
	double height = 0.0;

	ferite_get_parameters(params, 3, &name, &width, &height);

	if( _WedoEngine_GetWindow() )
		FE_RETURN_TRUE;

	Uint32 flags = 0; /* SDL_WINDOW_SHOWN; */

	SDL_Window *window = SDL_CreateWindow(
			name->data,
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			(int)width,
			(int)height,
			flags
		);

	if( NOT window )
		FE_RETURN_FALSE;

	SDL_Renderer *renderer = SDL_CreateRenderer(
			window,
			-1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
		);

	if( NOT renderer ) {
		SDL_DestroyWindow(window);
		window = NULL;
		FE_RETURN_FALSE;
	}

	_WedoEngine_SetWindow(window);
	_WedoEngine_SetRenderer(renderer);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	FE_RETURN_TRUE;
}

FE_NATIVE_FUNCTION( _WedoEngine_FeriteClearScreen ) {
	if( _WedoEngine_GetRenderer() )
		SDL_RenderClear(_WedoEngine_GetRenderer());
	FE_RETURN_VOID;
}

FE_NATIVE_FUNCTION( _WedoEngine_FeriteUpdateScreen ) {
	if( _WedoEngine_GetRenderer() )
		SDL_RenderPresent(_WedoEngine_GetRenderer());
	FE_RETURN_VOID;
}

FE_NATIVE_FUNCTION( _WedoEngine_FeriteNextEvent ) {
	FeriteNamespaceBucket *nsb = ferite_find_namespace(script, script->mainns, "Engine", FENS_NS);
	FeriteNamespace *engine_namespace = nsb->data;
	FeriteVariable *event_variable = NULL;
	SDL_Event event;

	if( SDL_PollEvent(&event) ) {
		switch( event.type ) {
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				event_variable = _WedoEngine_CreateFeriteKeyboardEvent(script, engine_namespace, &event);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				event_variable = _WedoEngine_CreateFeriteMouseButtonEvent(script, engine_namespace, &event);
				break;
			case SDL_MOUSEMOTION:
				event_variable = _WedoEngine_CreateFeriteMouseMotionEvent(script, engine_namespace, &event);
			case SDL_QUIT:
				event_variable = _WedoEngine_CreateFeriteQuitEvent(script, engine_namespace, &event);
				break;
			default:
				event_variable = _WedoEngine_CreateFeriteUnhandledEvent(script, engine_namespace, &event);
		}
	}

	if( event_variable ) {
		MARK_VARIABLE_AS_DISPOSABLE(event_variable);
		FE_RETURN_VAR(event_variable);
	}

	FE_RETURN_NULL_OBJECT;
}

FE_NATIVE_FUNCTION( _WedoEngine_FeriteLoadImage ) {
	FeriteString *file = NULL;
	SDL_Surface *surface = NULL;

	ferite_get_parameters(params, 1, &file);

	surface = IMG_Load(file->data);

	if( surface ) {
		FeriteNamespaceBucket *nsb = ferite_find_namespace(script, script->mainns, "Engine.Image", FENS_CLS);
		FeriteVariable *image_variable = ferite_build_object(script, nsb->data);
		VAO(image_variable)->odata = surface;
		OBJECT_SET_NUMBER_LONG_VAR(script, image_variable, "width", surface->w);
		OBJECT_SET_NUMBER_LONG_VAR(script, image_variable, "height", surface->h);
		MARK_VARIABLE_AS_DISPOSABLE(image_variable);
		FE_RETURN_VAR(image_variable);
	}

	FE_RETURN_NULL_OBJECT;
}

FE_NATIVE_FUNCTION( _WedoEngine_FeriteLoadTexture ) {
	FeriteString *file = NULL;
	SDL_Surface *surface = NULL;

	ferite_get_parameters(params, 1, &file);

	surface = IMG_Load(file->data);

	if( surface ) {
		int surface_width = surface->w;
		int surface_height = surface->h;
		SDL_Texture *texture = SDL_CreateTextureFromSurface(_WedoEngine_GetRenderer(), surface);
		SDL_FreeSurface(surface);
		surface = NULL;

		if( texture ) {
			FeriteNamespaceBucket *nsb = ferite_find_namespace(script, script->mainns, "Engine.Texture", FENS_CLS);
			FeriteVariable *texture_variable = ferite_build_object(script, nsb->data);
			VAO(texture_variable)->odata = texture;
			OBJECT_SET_NUMBER_LONG_VAR(script, texture_variable, "width", surface_width);
			OBJECT_SET_NUMBER_LONG_VAR(script, texture_variable, "height", surface_height);
			MARK_VARIABLE_AS_DISPOSABLE(texture_variable);
			FE_RETURN_VAR(texture_variable);
		} else {
			printf("[SDL] Error: %s\n", SDL_GetError());
		}
	} else {
		printf("[SDL] Error: %s\n", SDL_GetError());
	}

	FE_RETURN_NULL_OBJECT;
}

FE_NATIVE_FUNCTION( _WedoEngine_FeriteRenderTexture ) {
	FeriteObject *texture_object = NULL;
	double x = 0.0;
	double y = 0.0;
	double width = 0.0;
	double height = 0.0;

	ferite_get_parameters(params, 5, &texture_object, &x, &y, &width, &height);

	if( ferite_object_is_subclass(texture_object, "Texture") ) {
		SDL_Texture *texture = texture_object->odata;
		SDL_Rect destination;
		destination.x = x;
		destination.y = y;
		destination.w = width;
		destination.h = height;
		SDL_RenderCopy(_WedoEngine_GetRenderer(), texture, NULL, &destination);
	}

	FE_RETURN_VOID;
}
