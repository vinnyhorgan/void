#include <cstdio>
#include <string>
#include <cassert>
#include <fstream>
#include <vector>

#include "raylib.h"
#include "raymath.h"

#include "imgui.h"
#include "rlImGui.h"
#include "TextEditor.h"

#include "angelscript.h"
#include "scriptstdstring.h"
#include "scriptbuilder.h"

#include "api.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define WIDTH 800
#define HEIGHT 600
#define REFRESH_RATE 60

#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))

using namespace std;

vector<string> consoleHistory;
string baseDir = "demo";
asIScriptEngine *engine;
asIScriptContext *ctx;
asIScriptFunction *initFunc;
asIScriptFunction *updateFunc;
asIScriptFunction *drawFunc;

void messageCallback(const asSMessageInfo *msg, void *param)
{
    const char *type = "ERR";

    if (msg->type == asMSGTYPE_WARNING)
        type = "WARN";
    else if (msg->type == asMSGTYPE_INFORMATION)
        type = "INFO";

    printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
}

void configureEngine(asIScriptEngine *engine)
{
    int r;

    RegisterStdString(engine);

    r = engine->SetDefaultNamespace("vd"); assert(r >= 0);
    r = engine->RegisterGlobalFunction("void log(string &in)", asFUNCTION(Api::log), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("string toString(int)", asFUNCTIONPR(Api::toString, (int), string), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("string toString(float)", asFUNCTIONPR(Api::toString, (float), string), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("string toString(bool)", asFUNCTIONPR(Api::toString, (bool), string), asCALL_CDECL); assert(r >= 0);

    r = engine->RegisterObjectType("Version", 0, asOBJ_REF); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("Version", asBEHAVE_FACTORY, "Version@ f()", asFUNCTION(Api::Version::factory), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("Version", asBEHAVE_ADDREF, "void f()", asMETHOD(Api::Version, addRef), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("Version", asBEHAVE_RELEASE, "void f()", asMETHOD(Api::Version, release), asCALL_THISCALL); assert(r >= 0);
    r = engine->RegisterObjectMethod("Version", "Version &opAssign(Version&in)", asFUNCTION(Api::Version::assignment), asCALL_CDECL_OBJLAST); assert(r >= 0);
    r = engine->RegisterObjectProperty("Version", "int major", asOFFSET(Api::Version, major)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Version", "int minor", asOFFSET(Api::Version, minor)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Version", "int patch", asOFFSET(Api::Version, patch)); assert(r >= 0);

    r = engine->RegisterGlobalFunction("Version@ getVersion()", asFUNCTION(Api::getVersion), asCALL_CDECL); assert(r >= 0);

    r = engine->SetDefaultNamespace("vd::graphics"); assert(r >= 0);
    r = engine->RegisterGlobalFunction("void print(string &in, int, int)", asFUNCTION(Api::print), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("void rectangle(string &in, int, int, int, int)", asFUNCTION(Api::rectangle), asCALL_CDECL); assert(r >= 0);

    r = engine->SetDefaultNamespace("vd::math"); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float random()", asFUNCTION(Api::random), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float sqrt(float)", asFUNCTION(Api::sqrt), asCALL_CDECL); assert(r >= 0);

    r = engine->SetDefaultNamespace("vd::keyboard"); assert(r >= 0);
    r = engine->RegisterEnum("Key"); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "A", Api::Key::A); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "B", Api::Key::B); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "C", Api::Key::C); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "D", Api::Key::D); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "E", Api::Key::E); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F", Api::Key::F); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "G", Api::Key::G); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "H", Api::Key::H); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "I", Api::Key::I); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "J", Api::Key::J); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "K", Api::Key::K); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "L", Api::Key::L); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "M", Api::Key::M); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "N", Api::Key::N); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "O", Api::Key::O); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "P", Api::Key::P); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Q", Api::Key::Q); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "R", Api::Key::R); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "S", Api::Key::S); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "T", Api::Key::T); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "U", Api::Key::U); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "V", Api::Key::V); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "W", Api::Key::W); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "X", Api::Key::X); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Y", Api::Key::Y); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Z", Api::Key::Z); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Space", Api::Key::Space); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Enter", Api::Key::Enter); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Escape", Api::Key::Escape); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Up", Api::Key::Up); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Down", Api::Key::Down); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Left", Api::Key::Left); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Right", Api::Key::Right); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Shift", Api::Key::Shift); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Control", Api::Key::Control); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Alt", Api::Key::Alt); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Tab", Api::Key::Tab); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Backspace", Api::Key::Backspace); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "CapsLock", Api::Key::CapsLock); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Zero", Api::Key::Zero); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "One", Api::Key::One); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Two", Api::Key::Two); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Three", Api::Key::Three); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Four", Api::Key::Four); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Five", Api::Key::Five); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Six", Api::Key::Six); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Seven", Api::Key::Seven); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Eight", Api::Key::Eight); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Nine", Api::Key::Nine); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F1", Api::Key::F1); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F2", Api::Key::F2); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F3", Api::Key::F3); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F4", Api::Key::F4); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F5", Api::Key::F5); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F6", Api::Key::F6); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F7", Api::Key::F7); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F8", Api::Key::F8); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F9", Api::Key::F9); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F10", Api::Key::F10); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F11", Api::Key::F11); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F12", Api::Key::F12); assert(r >= 0);
    r = engine->RegisterGlobalFunction("bool isDown(int)", asFUNCTION(Api::isDown), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("bool isPressed(int)", asFUNCTION(Api::isPressed), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("bool isReleased(int)", asFUNCTION(Api::isReleased), asCALL_CDECL); assert(r >= 0);
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

int callFunction(asIScriptContext *ctx, asIScriptFunction *function)
{
    int r;

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

int reload()
{
    int r;

    engine = asCreateScriptEngine();
    if (engine == 0)
    {
        printf("Failed to create script engine.\n");
        return -1;
    }

    engine->SetMessageCallback(asFUNCTION(messageCallback), 0, asCALL_CDECL);

    configureEngine(engine);

    r = compileScript(engine, baseDir + "/main.as");
    if (r < 0)
    {
        engine->Release();
        return -1;
    }

    ctx = engine->CreateContext();
    if (ctx == 0)
    {
        printf("Failed to create the context.\n");
        engine->Release();
        return -1;
    }

    initFunc = getFunction(engine, "void init()");
    if (initFunc == 0)
    {
        printf("The script must contain an init function!\n");
        ctx->Release();
        engine->Release();
        return -1;
    }

    updateFunc = getFunction(engine, "void update(float dt)");
    if (initFunc == 0)
    {
        printf("The script must contain an update function!\n");
        ctx->Release();
        engine->Release();
        return -1;
    }

    drawFunc = getFunction(engine, "void draw()");
    if (initFunc == 0)
    {
        printf("The script must contain a draw function!\n");
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

    r = callFunction(ctx, initFunc);
    if (r < 0)
    {
        ctx->Release();
        engine->Release();
        return -1;
    }

    return 0;
}

int main()
{
    int r;

    r = reload();
    if (r < 0)
        return -1;

    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Void by Vinny Horgan");
    SetWindowMinSize(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    SetTargetFPS(REFRESH_RATE);
    SetExitKey(KEY_NULL);

    RenderTexture target = LoadRenderTexture(WIDTH, HEIGHT);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

    rlImGuiSetup(true);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    TextEditor editor;
    auto lang = TextEditor::LanguageDefinition::AngelScript();
    editor.SetLanguageDefinition(lang);

    ifstream t(baseDir + "/main.as");
    stringstream buffer;
    buffer << t.rdbuf();

    editor.SetText(buffer.str());

    while (!WindowShouldClose())
    {
        float scale = MIN((float)GetScreenWidth()/WIDTH, (float)GetScreenHeight()/HEIGHT);

        Vector2 mouse = GetMousePosition();
        Vector2 virtualMouse = { 0 };
        virtualMouse.x = (mouse.x - (GetScreenWidth() - (WIDTH*scale))*0.5f)/scale;
        virtualMouse.y = (mouse.y - (GetScreenHeight() - (HEIGHT*scale))*0.5f)/scale;
        virtualMouse = Vector2Clamp(virtualMouse, (Vector2){ 0, 0 }, (Vector2){ (float)WIDTH, (float)HEIGHT });

        float dt = GetFrameTime();

        r = ctx->Prepare(updateFunc);
        if (r < 0)
        {
            printf("Failed to prepare the context.\n");
            ctx->Release();
            engine->Release();
            return -1;
        }

        ctx->SetArgFloat(0, dt);

        r = callFunction(ctx, updateFunc);
        if (r < 0)
        {
            ctx->Release();
            engine->Release();
            return -1;
        }

        BeginDrawing();

        ClearBackground(BLACK);

        BeginTextureMode(target);

        ClearBackground(BLACK);

        r = ctx->Prepare(drawFunc);
        if (r < 0)
        {
            printf("Failed to prepare the context.\n");
            ctx->Release();
            engine->Release();
            return -1;
        }

        r = callFunction(ctx, drawFunc);
        if (r < 0)
        {
            ctx->Release();
            engine->Release();
            return -1;
        }

        EndTextureMode();

/*
        DrawTexturePro(target.texture, (Rectangle){ 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height },
                    (Rectangle){ (GetScreenWidth() - ((float)WIDTH*scale))*0.5f, (GetScreenHeight() - ((float)HEIGHT*scale))*0.5f,
                    (float)WIDTH*scale, (float)HEIGHT*scale }, (Vector2){ 0, 0 }, 0.0f, WHITE);
*/

        rlImGuiBegin();

        ImGui::DockSpaceOverViewport();

/*
        bool open = true;
        ImGui::ShowDemoWindow(&open);
*/

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_None);

        rlImGuiImageRect(&target.texture, target.texture.width, target.texture.height, (Rectangle){ 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height});
        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::Begin("Console", nullptr, ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Options"))
            {
                if (ImGui::MenuItem("Clear"))
                {
                    consoleHistory.clear();
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        for (auto& line : consoleHistory)
        {
            ImGui::TextUnformatted(line.c_str());
        }

        ImGui::End();

        auto cpos = editor.GetCursorPosition();
        ImGui::Begin("Text Editor", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
        ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Run"))
                {
                    r = reload();
                    if (r < 0)
                        return -1;
                }

                if (ImGui::MenuItem("Save"))
                {
                    string textToSave = editor.GetText();

                    ofstream out(baseDir + "/main.as");
                    out << textToSave;
                    out.close();
                }

                if (ImGui::MenuItem("Quit", "Alt-F4"))
                    break;

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit"))
            {
                bool ro = editor.IsReadOnly();

                if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
                    editor.SetReadOnly(ro);

                ImGui::Separator();

                if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && editor.CanUndo()))
                    editor.Undo();

                if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))
                    editor.Redo();

                ImGui::Separator();

                if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
                    editor.Copy();

                if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))
                    editor.Cut();

                if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))
                    editor.Delete();

                if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
                    editor.Paste();

                ImGui::Separator();

                if (ImGui::MenuItem("Select all", nullptr, nullptr))
                    editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::MenuItem("Dark palette"))
                    editor.SetPalette(TextEditor::GetDarkPalette());

                if (ImGui::MenuItem("Light palette"))
                    editor.SetPalette(TextEditor::GetLightPalette());

                if (ImGui::MenuItem("Retro blue palette"))
                    editor.SetPalette(TextEditor::GetRetroBluePalette());

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
            editor.IsOverwrite() ? "Ovr" : "Ins",
            editor.CanUndo() ? "*" : " ",
            editor.GetLanguageDefinition().mName.c_str(), baseDir.c_str());

        editor.Render("TextEditor");
        ImGui::End();

        rlImGuiEnd();

        EndDrawing();
    }

    ctx->Release();
    engine->ShutDownAndRelease();

    rlImGuiShutdown();

    UnloadRenderTexture(target);

    CloseWindow();

    return 0;
}
