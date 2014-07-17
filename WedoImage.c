#include "WedoFerite.h"
#include "WedoEngine.h"

FE_NATIVE_FUNCTION( _WedoImage_FeriteDestructor ) {
	FeriteObject *self = FE_CONTAINER_TO_OBJECT;
	SDL_Surface *surface = self->odata;
	if( surface ) {
		SDL_FreeSurface(surface);
		self->odata = NULL;
	}
	self = NULL;
	FE_RETURN_VOID;
}

FE_NATIVE_FUNCTION( _WedoImage_FeriteToTexture ) {
	FeriteObject *self = FE_CONTAINER_TO_OBJECT;
	SDL_Surface *surface = self->odata;

	if( surface ) {
		SDL_Texture *texture = SDL_CreateTextureFromSurface(_WedoEngine_GetRenderer(), surface);
		if( texture ) {
			FeriteNamespaceBucket *nsb = ferite_find_namespace(script, script->mainns, "Engine.Texture", FENS_CLS);
			FeriteVariable *texture_variable = ferite_build_object(script, nsb->data);
			VAO(texture_variable)->odata = texture;
			OBJECT_SET_NUMBER_LONG_VAR(script, texture_variable, "width", surface->w);
			OBJECT_SET_NUMBER_LONG_VAR(script, texture_variable, "height", surface->h);
			MARK_VARIABLE_AS_DISPOSABLE(texture_variable);
			FE_RETURN_VAR(texture_variable);
		}
	}

	FE_RETURN_NULL_OBJECT;
}
