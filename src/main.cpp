#include <stdio.h>

#include "raylib.h"
#include "raymath.h"

#include "angelscript.h"
#include "scriptstdstring.h"
#include "scriptbuilder.h"

#define WIDTH 1280
#define HEIGHT 720

#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))

using namespace std;

void ASprint(string &str)
{
    printf("%s\n", str.c_str());
}

void MessageCallback(const asSMessageInfo *msg, void *param)
{
    const char *type = "ERR ";

    if( msg->type == asMSGTYPE_WARNING )
        type = "WARN";
    else if( msg->type == asMSGTYPE_INFORMATION )
        type = "INFO";

    printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
}

int configureEngine(asIScriptEngine *engine)
{
    int r;

    RegisterStdString(engine);

    r = engine->RegisterGlobalFunction("void print(string &in)", asFUNCTION(ASprint), asCALL_CDECL);
    if (r < 0)
        return r;

    return r;
}

int compileScript(asIScriptEngine *engine, string script)
{
    int r;

    CScriptBuilder builder;

    r = builder.StartNewModule(engine, "MainModule");
    if (r < 0)
        return r;

    r = builder.AddSectionFromFile(script.c_str());
    if (r < 0)
        return r;

    r = builder.BuildModule();
    if (r < 0)
        return r;

    return r;
}

int main()
{
    int r;

    string script = "demo/main.as";

    asIScriptEngine *engine = asCreateScriptEngine();
    if (engine == 0)
    {
        printf("Failed to create script engine.\n");
        return -1;
    }

    engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL);

    r = configureEngine(engine);
    if (r < 0)
    {
        printf("Error configuring engine.\n");
        engine->Release();
        return -1;
    }

    r = compileScript(engine, script);
    if (r < 0)
    {
        printf("Error compiling script.\n");
        engine->Release();
        return -1;
    }

    asIScriptContext *ctx = engine->CreateContext();
    if (ctx == 0)
    {
        printf("Failed to create the context.\n");
        engine->Release();
        return -1;
    }

    asIScriptFunction *initFunc = engine->GetModule("MainModule")->GetFunctionByDecl("void init()");
    if (initFunc == 0)
    {
        printf("The script must contain an init function!\n");
        ctx->Release();
        engine->Release();
        return -1;
    }

    r = ctx->Prepare(initFunc);
    if (r < 0)
    {
        printf("Failed to prepare the context.\n");
        ctx->Release();
        engine->Release();
        return -1;
    }

    r = ctx->Execute();
    if (r != asEXECUTION_FINISHED)
    {
        if (r == asEXECUTION_ABORTED)
            printf("The script was aborted.\n");
        else if (r == asEXECUTION_EXCEPTION)
        {
            printf("The script ended with an exception.\n");

            asIScriptFunction *func = ctx->GetExceptionFunction();
            printf("func: %s\n", func->GetDeclaration());
            printf("modl: %s\n", func->GetModuleName());
            printf("sect: %s\n", func->GetScriptSectionName());
            printf("line: %d\n", ctx->GetExceptionLineNumber());
            printf("desc: %s\n", ctx->GetExceptionString());
        }
        else
            printf("The script ended for an unexpected reason.\n");
    }

    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WIDTH, HEIGHT, "Void by Vinny Horgan");
    SetWindowMinSize(WIDTH, HEIGHT);
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    MaximizeWindow();

    RenderTexture target = LoadRenderTexture(WIDTH, HEIGHT);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

    while (!WindowShouldClose())
    {
        float scale = MIN((float)GetScreenWidth()/WIDTH, (float)GetScreenHeight()/HEIGHT);

        Vector2 mouse = GetMousePosition();
        Vector2 virtualMouse = { 0 };
        virtualMouse.x = (mouse.x - (GetScreenWidth() - (WIDTH*scale))*0.5f)/scale;
        virtualMouse.y = (mouse.y - (GetScreenHeight() - (HEIGHT*scale))*0.5f)/scale;
        virtualMouse = Vector2Clamp(virtualMouse, (Vector2){ 0, 0 }, (Vector2){ (float)WIDTH, (float)HEIGHT });

        BeginDrawing();

        ClearBackground(BLACK);

        BeginTextureMode(target);

        ClearBackground(BLACK);

        EndTextureMode();

        DrawTexturePro(target.texture, (Rectangle){ 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height },
                    (Rectangle){ (GetScreenWidth() - ((float)WIDTH*scale))*0.5f, (GetScreenHeight() - ((float)HEIGHT*scale))*0.5f,
                    (float)WIDTH*scale, (float)HEIGHT*scale }, (Vector2){ 0, 0 }, 0.0f, WHITE);

        EndDrawing();
    }

    ctx->Release();
    engine->ShutDownAndRelease();

    UnloadRenderTexture(target);

    CloseWindow();

    return 0;
}
