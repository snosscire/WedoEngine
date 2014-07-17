#ifndef WEDOFERITE_H
#define WEDOFERITE_H

#include "ferite.h"

/* namespace Engine */
FE_NATIVE_FUNCTION( _WedoEngine_FeritePrintLine );
FE_NATIVE_FUNCTION( _WedoEngine_FeriteShowSimpleMessageBox );
FE_NATIVE_FUNCTION( _WedoEngine_FeriteDelay );
FE_NATIVE_FUNCTION( _WedoEngine_FeriteCreateWindow );
FE_NATIVE_FUNCTION( _WedoEngine_FeriteClearScreen );
FE_NATIVE_FUNCTION( _WedoEngine_FeriteUpdateScreen );
FE_NATIVE_FUNCTION( _WedoEngine_FeriteNextEvent );
FE_NATIVE_FUNCTION( _WedoEngine_FeriteLoadImage );
FE_NATIVE_FUNCTION( _WedoEngine_FeriteLoadTexture );
FE_NATIVE_FUNCTION( _WedoEngine_FeriteRenderTexture );

/* class Engine.Image */
FE_NATIVE_FUNCTION( _WedoImage_FeriteDestructor );
FE_NATIVE_FUNCTION( _WedoImage_FeriteToTexture );

/* class Engine.Texture */
FE_NATIVE_FUNCTION( _WedoTexture_FeriteDestructor );

#endif /* WEDOFERITE_H */