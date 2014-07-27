#ifndef WEDOENGINE_H
#define WEDOENGINE_H

int  WedoEngine_Initialize();
void WedoEngine_Terminate();

int  WedoEngine_ExecuteScript( int argc, char *argv[], char *name );

#endif /* WEDOENGINE_H */