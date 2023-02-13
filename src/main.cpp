#include <stdio.h>
#include <string>

#include "raylib.h"
#include "raymath.h"

#include "angelscript.h"
#include "scriptstdstring.h"
#include "scriptbuilder.h"

#include "api.h"

#define WIDTH 1280
#define HEIGHT 720

#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))

using namespace std;

void messageCallback(const asSMessageInfo *msg, void *param)
{
    const char *type = "ERR ";

    if (msg->type == asMSGTYPE_WARNING)
        type = "WARN";
    else if (msg->type == asMSGTYPE_INFORMATION)
        type = "INFO";

    printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
}

void configureEngine(asIScriptEngine *engine)
{
    RegisterStdString(engine);

    engine->RegisterGlobalFunction("void print(string &in)", asFUNCTION(print), asCALL_CDECL);
    engine->RegisterGlobalFunction("string toString(float)", asFUNCTION(toString), asCALL_CDECL);
    engine->RegisterGlobalFunction("void graphicsPrint(string &in, int, int)", asFUNCTION(graphicsPrint), asCALL_CDECL);
}

int compileScript(asIScriptEngine *engine, string script)
{
    int r;

    CScriptBuilder builder;

    r = builder.StartNewModule(engine, 0);
    if (r < 0)
    {
        printf("Failed to start new module.\n");
        return r;
    }

    r = builder.AddSectionFromFile(script.c_str());
    if (r < 0)
    {
        printf("Failed to add script file.");
        return r;
    }

    r = builder.BuildModule();
    if (r < 0)
    {
        printf("Failed to build the module.");
        return r;
    }

    return 0;
}

asIScriptFunction *getFunction(asIScriptEngine *engine, string declaration)
{
    return engine->GetModule(0)->GetFunctionByDecl(declaration.c_str());
}

int callFunction(asIScriptContext *ctx, asIScriptFunction *function, bool param, float dt)
{
    int r;

    r = ctx->Prepare(function);
    if (r < 0)
    {
        printf("Failed to prepare the context.\n");
        return r;
    }

    if (param)
    {
        ctx->SetArgFloat(0, dt);
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

        return r;
    }

    return 0;
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

    engine->SetMessageCallback(asFUNCTION(messageCallback), 0, asCALL_CDECL);

    configureEngine(engine);

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

    asIScriptFunction *initFunc = getFunction(engine, "void init()");
    if (initFunc == 0)
    {
        printf("The script must contain an init function!\n");
        ctx->Release();
        engine->Release();
        return -1;
    }

    asIScriptFunction *updateFunc = getFunction(engine, "void update(float dt)");
    if (initFunc == 0)
    {
        printf("The script must contain an update function!\n");
        ctx->Release();
        engine->Release();
        return -1;
    }

    asIScriptFunction *drawFunc = getFunction(engine, "void draw()");
    if (initFunc == 0)
    {
        printf("The script must contain a draw function!\n");
        ctx->Release();
        engine->Release();
        return -1;
    }

    r = callFunction(ctx, initFunc, false, 0);
    if (r < 0)
    {
        printf("Error calling function.\n");
        ctx->Release();
        engine->Release();
        return -1;
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

        float dt = GetFrameTime();

        r = callFunction(ctx, updateFunc, true, dt);
        if (r < 0)
        {
            printf("Error calling function.\n");
            ctx->Release();
            engine->Release();
            return -1;
        }

        BeginDrawing();

        ClearBackground(BLACK);

        BeginTextureMode(target);

        ClearBackground(BLACK);

        r = callFunction(ctx, drawFunc, false, 0);
        if (r < 0)
        {
            printf("Error calling function.\n");
            ctx->Release();
            engine->Release();
            return -1;
        }

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
