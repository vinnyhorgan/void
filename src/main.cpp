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
#include "scriptbuilder.h"

#include "scriptstdstring.h"
#include "scriptarray.h"
#include "scriptany.h"
#include "scripthandle.h"
#include "weakref.h"
#include "scriptdictionary.h"
#include "scriptfile.h"
#include "scriptfilesystem.h"
#include "scriptmath.h"
#include "scriptmathcomplex.h"
#include "scriptgrid.h"
#include "datetime.h"
#include "scripthelper.h"

#include "api.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define WIDTH 800
#define HEIGHT 600
#define REFRESH_RATE 60

#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))

using namespace std;

enum Mode
{
    MODE_DEV,
    MODE_RUNTIME
};

struct Error
{
    string message;
    string section;
    int type;
    int line;
    int column;
    vector<string> tracelog;
};

Mode mode = MODE_DEV;
bool devRunning = false;
bool error = false;
Error errorMessage;
vector<string> consoleHistory;
string baseDir = "demo";
bool focus = true;
Vector2 virtualMouse;

asIScriptEngine *engine;
asIScriptContext *ctx;
asIScriptFunction *initFunc;
asIScriptFunction *updateFunc;
asIScriptFunction *drawFunc;
asIScriptFunction *filesdroppedFunc;
asIScriptFunction *focusFunc;
asIScriptFunction *resizeFunc;
asIScriptFunction *keypressedFunc;
asIScriptFunction *textinputFunc;

void errorHandler(string message)
{
    printf("Error: %s\n", message.c_str());

    if (ctx)
    {
        ctx->Release();
    }

    if (engine)
    {
        engine->Release();
    }

    error = true;
    errorMessage.message = message;
}

void messageCallback(const asSMessageInfo *msg, void *param)
{
    const char *type = "ERR";

    if (msg->type == asMSGTYPE_WARNING)
        type = "WARN";
    else if (msg->type == asMSGTYPE_INFORMATION)
        type = "INFO";

    printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);

    errorMessage.type = msg->type;
    errorMessage.line = msg->row;
    errorMessage.column = msg->col;
    errorMessage.section = msg->section;
    errorMessage.tracelog.push_back(msg->message);
}

void configureEngine(asIScriptEngine *engine)
{
    int r;

    RegisterStdString(engine);

    RegisterScriptArray(engine, true);

    RegisterStdStringUtils(engine);

    RegisterScriptAny(engine);

    RegisterScriptHandle(engine);

    RegisterScriptWeakRef(engine);

    RegisterScriptDictionary(engine);

    RegisterScriptFile(engine);

    RegisterScriptDateTime(engine);

    RegisterScriptFileSystem(engine);

    RegisterScriptMath(engine);
    RegisterScriptMathComplex(engine);

    RegisterScriptGrid(engine);

    RegisterExceptionRoutines(engine);

    r = engine->SetDefaultNamespace("vd"); assert(r >= 0);

    r = engine->RegisterObjectType("Version", sizeof(Api::Version), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Api::Version>()); assert(r >= 0);
    r = engine->RegisterObjectProperty("Version", "int major", asOFFSET(Api::Version, major)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Version", "int minor", asOFFSET(Api::Version, minor)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Version", "int patch", asOFFSET(Api::Version, patch)); assert(r >= 0);

    r = engine->RegisterObjectType("Vector2", sizeof(Api::Vector2), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Api::Vector2>()); assert(r >= 0);
    r = engine->RegisterObjectProperty("Vector2", "float x", asOFFSET(Api::Vector2, x)); assert(r >= 0);
    r = engine->RegisterObjectProperty("Vector2", "float y", asOFFSET(Api::Vector2, y)); assert(r >= 0);

    r = engine->RegisterObjectType("Image", sizeof(Api::Image), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Api::Image>()); assert(r >= 0);

    r = engine->RegisterGlobalFunction("void log(string &in)", asFUNCTION(Api::log), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("string toString(int)", asFUNCTIONPR(Api::toString, (int), string), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("string toString(float)", asFUNCTIONPR(Api::toString, (float), string), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("string toString(bool)", asFUNCTIONPR(Api::toString, (bool), string), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Version getVersion()", asFUNCTION(Api::getVersion), asCALL_CDECL); assert(r >= 0);

    r = engine->SetDefaultNamespace("vd::graphics"); assert(r >= 0);

    r = engine->RegisterGlobalFunction("void print(string &in, int, int)", asFUNCTION(Api::Graphics::print), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("void rectangle(string &in, int, int, int, int)", asFUNCTION(Api::Graphics::rectangle), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Image newImage(string &in)", asFUNCTION(Api::Graphics::newImage), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("void drawImage(Image, int, int)", asFUNCTION(Api::Graphics::drawImage), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("void point(int, int)", asFUNCTION(Api::Graphics::point), asCALL_CDECL); assert(r >= 0);

    r = engine->SetDefaultNamespace("vd::math"); assert(r >= 0);
    r = engine->RegisterGlobalFunction("float random()", asFUNCTION(Api::Math::random), asCALL_CDECL); assert(r >= 0);

    r = engine->SetDefaultNamespace("vd::mouse"); assert(r >= 0);
    r = engine->RegisterEnum("MouseButton"); assert(r >= 0);
    r = engine->RegisterEnumValue("MouseButton", "Left", Api::Mouse::MouseButton::LeftB); assert(r >= 0);
    r = engine->RegisterEnumValue("MouseButton", "Right", Api::Mouse::MouseButton::RightB); assert(r >= 0);
    r = engine->RegisterEnumValue("MouseButton", "Middle", Api::Mouse::MouseButton::Middle); assert(r >= 0);
    r = engine->RegisterGlobalFunction("Vector2 getPosition()", asFUNCTION(Api::Mouse::getPosition), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("bool isPressed(MouseButton)", asFUNCTION(Api::Mouse::isPressed), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("bool isReleased(MouseButton)", asFUNCTION(Api::Mouse::isReleased), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("bool isDown(MouseButton)", asFUNCTION(Api::Mouse::isDown), asCALL_CDECL); assert(r >= 0);

    r = engine->SetDefaultNamespace("vd::keyboard"); assert(r >= 0);
    r = engine->RegisterEnum("Key"); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "A", Api::Keyboard::Key::A); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "B", Api::Keyboard::Key::B); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "C", Api::Keyboard::Key::C); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "D", Api::Keyboard::Key::D); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "E", Api::Keyboard::Key::E); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F", Api::Keyboard::Key::F); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "G", Api::Keyboard::Key::G); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "H", Api::Keyboard::Key::H); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "I", Api::Keyboard::Key::I); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "J", Api::Keyboard::Key::J); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "K", Api::Keyboard::Key::K); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "L", Api::Keyboard::Key::L); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "M", Api::Keyboard::Key::M); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "N", Api::Keyboard::Key::N); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "O", Api::Keyboard::Key::O); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "P", Api::Keyboard::Key::P); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Q", Api::Keyboard::Key::Q); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "R", Api::Keyboard::Key::R); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "S", Api::Keyboard::Key::S); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "T", Api::Keyboard::Key::T); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "U", Api::Keyboard::Key::U); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "V", Api::Keyboard::Key::V); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "W", Api::Keyboard::Key::W); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "X", Api::Keyboard::Key::X); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Y", Api::Keyboard::Key::Y); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Z", Api::Keyboard::Key::Z); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Space", Api::Keyboard::Key::Space); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Enter", Api::Keyboard::Key::Enter); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Escape", Api::Keyboard::Key::Escape); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Up", Api::Keyboard::Key::Up); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Down", Api::Keyboard::Key::Down); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Left", Api::Keyboard::Key::Left); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Right", Api::Keyboard::Key::Right); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Shift", Api::Keyboard::Key::Shift); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Control", Api::Keyboard::Key::Control); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Alt", Api::Keyboard::Key::Alt); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Tab", Api::Keyboard::Key::Tab); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Backspace", Api::Keyboard::Key::Backspace); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "CapsLock", Api::Keyboard::Key::CapsLock); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Zero", Api::Keyboard::Key::Zero); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "One", Api::Keyboard::Key::One); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Two", Api::Keyboard::Key::Two); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Three", Api::Keyboard::Key::Three); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Four", Api::Keyboard::Key::Four); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Five", Api::Keyboard::Key::Five); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Six", Api::Keyboard::Key::Six); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Seven", Api::Keyboard::Key::Seven); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Eight", Api::Keyboard::Key::Eight); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "Nine", Api::Keyboard::Key::Nine); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F1", Api::Keyboard::Key::F1); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F2", Api::Keyboard::Key::F2); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F3", Api::Keyboard::Key::F3); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F4", Api::Keyboard::Key::F4); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F5", Api::Keyboard::Key::F5); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F6", Api::Keyboard::Key::F6); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F7", Api::Keyboard::Key::F7); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F8", Api::Keyboard::Key::F8); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F9", Api::Keyboard::Key::F9); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F10", Api::Keyboard::Key::F10); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F11", Api::Keyboard::Key::F11); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", "F12", Api::Keyboard::Key::F12); assert(r >= 0);
    r = engine->RegisterGlobalFunction("bool isDown(Key)", asFUNCTION(Api::Keyboard::isDown), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("bool isPressed(Key)", asFUNCTION(Api::Keyboard::isPressed), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("bool isReleased(Key)", asFUNCTION(Api::Keyboard::isReleased), asCALL_CDECL); assert(r >= 0);

    r = engine->SetDefaultNamespace("vd::timer"); assert(r >= 0);
    r = engine->RegisterGlobalFunction("int getFPS()", asFUNCTION(Api::Timer::getFPS), asCALL_CDECL); assert(r >= 0);
}

int compileScript(asIScriptEngine *engine, string script)
{
    int r;

    CScriptBuilder builder;

    r = builder.StartNewModule(engine, 0);
    if (r < 0)
    {
        errorHandler("Failed to start new module.");
        return r;
    }

    r = builder.AddSectionFromFile(script.c_str());
    if (r < 0)
    {
        errorHandler("Failed to add script file.");
        return r;
    }

    r = builder.BuildModule();
    if (r < 0)
    {
        errorHandler("Failed to build the module.");
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
            errorHandler("The script was aborted.\n");
        else if (r == asEXECUTION_EXCEPTION)
        {
            asIScriptFunction *func = ctx->GetExceptionFunction();
            printf("func: %s\n", func->GetDeclaration());
            printf("modl: %s\n", func->GetModuleName());
            printf("sect: %s\n", func->GetScriptSectionName());
            printf("line: %d\n", ctx->GetExceptionLineNumber());
            printf("desc: %s\n", ctx->GetExceptionString());

            errorHandler("The script ended with an exception.\n");

            return r;
        }
        else
        {
            errorHandler("The script ended for an unexpected reason.\n");
            return r;
        }
    }

    return 0;
}

void reload()
{
    int r;

    engine = asCreateScriptEngine();
    if (engine == 0)
    {
        errorHandler("Failed to create script engine.");
        return;
    }

    engine->SetMessageCallback(asFUNCTION(messageCallback), 0, asCALL_CDECL);

    configureEngine(engine);

    r = compileScript(engine, baseDir + "/main.as");
    if (r < 0)
    {
        return;
    }

    ctx = engine->CreateContext();
    if (ctx == 0)
    {
        errorHandler("Failed to create the context.");
        return;
    }

    initFunc = getFunction(engine, "void init()");
    if (initFunc == 0)
    {
        errorHandler("The script must contain an init function!");
        return;
    }

    updateFunc = getFunction(engine, "void update(float)");
    if (updateFunc == 0)
    {
        errorHandler("The script must contain an update function!");
        return;
    }

    drawFunc = getFunction(engine, "void draw()");
    if (drawFunc == 0)
    {
        errorHandler("The script must contain a draw function!");
        return;
    }

    filesdroppedFunc = getFunction(engine, "void filesdropped(array<string>)");
    focusFunc = getFunction(engine, "void focus(bool)");
    resizeFunc = getFunction(engine, "void resize(int, int)");
    keypressedFunc = getFunction(engine, "void keypressed(int)");
    textinputFunc = getFunction(engine, "void textinput(string)");

    r = ctx->Prepare(initFunc);
    if (r < 0)
    {
        errorHandler("Failed to prepare the context.");
        return;
    }

    r = callFunction(ctx, initFunc);
    if (r < 0)
    {
        return;
    }

    error = false;
}

int main()
{
    int r;

    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Void by Vinny Horgan");
    SetWindowMinSize(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    // SetTargetFPS(REFRESH_RATE);
    SetExitKey(KEY_NULL);

    SetRandomSeed((unsigned) time(NULL));

    reload();

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
        virtualMouse.x = (mouse.x - (GetScreenWidth() - (WIDTH*scale))*0.5f)/scale;
        virtualMouse.y = (mouse.y - (GetScreenHeight() - (HEIGHT*scale))*0.5f)/scale;
        virtualMouse = Vector2Clamp(virtualMouse, (Vector2){ 0, 0 }, (Vector2){ (float)WIDTH, (float)HEIGHT });

        float dt = GetFrameTime();

        if (!error)
        {
            r = ctx->Prepare(updateFunc);
            if (r < 0)
            {
                errorHandler("Failed to prepare the context.");
            }
            else
            {
                ctx->SetArgFloat(0, dt);

                callFunction(ctx, updateFunc);
            }
        }

        if (IsFileDropped())
        {
            FilePathList files = LoadDroppedFiles();

            if (filesdroppedFunc != 0)
            {
                r = ctx->Prepare(filesdroppedFunc);
                if (r < 0)
                {
                    errorHandler("Failed to prepare the context.");
                }
                else
                {
                    CScriptArray *array = CScriptArray::Create(engine->GetTypeInfoByDecl("array<string>"), files.count);

                    for (unsigned int i = 0; i < files.count; i++)
                    {
                        string filename = files.paths[i];
                        array->SetValue(i, &filename);
                    }

                    ctx->SetArgObject(0, array);

                    callFunction(ctx, filesdroppedFunc);

                    UnloadDroppedFiles(files);
                }
            }
        }

        if (focus != IsWindowFocused())
        {
            focus = IsWindowFocused();

            if (focusFunc != 0)
            {
                r = ctx->Prepare(focusFunc);
                if (r < 0)
                {
                    errorHandler("Failed to prepare the context.");
                }
                else
                {
                    ctx->SetArgByte(0, focus);

                    callFunction(ctx, focusFunc);
                }
            }
        }

        if (IsWindowResized())
        {
            if (resizeFunc != 0)
            {
                r = ctx->Prepare(resizeFunc);
                if (r < 0)
                {
                    errorHandler("Failed to prepare the context.");
                }
                else
                {
                    ctx->SetArgDWord(0, GetScreenWidth());
                    ctx->SetArgDWord(1, GetScreenHeight());

                    callFunction(ctx, resizeFunc);
                }
            }
        }

        int key = GetKeyPressed();

        if (key != KEY_NULL)
        {
            if (keypressedFunc != 0)
            {
                r = ctx->Prepare(keypressedFunc);
                if (r < 0)
                {
                    errorHandler("Failed to prepare the context.");
                }
                else
                {
                    ctx->SetArgDWord(0, key);

                    callFunction(ctx, keypressedFunc);
                }
            }
        }

        int charPressed = GetCharPressed();

        if (charPressed != 0)
        {
            if (textinputFunc != 0)
            {
                r = ctx->Prepare(textinputFunc);
                if (r < 0)
                {
                    errorHandler("Failed to prepare the context.");
                }
                else
                {
                    string text = string(1, charPressed);

                    ctx->SetArgObject(0, &text);

                    callFunction(ctx, textinputFunc);
                }
            }
        }


        if (mode == MODE_DEV && IsKeyPressed(KEY_ESCAPE))
        {
            mode = MODE_RUNTIME;
            devRunning = true;
        }
        else if (devRunning && IsKeyPressed(KEY_ESCAPE))
        {
            mode = MODE_DEV;
            devRunning = false;
        }

        BeginDrawing();

        ClearBackground(BLACK);

        BeginTextureMode(target);

        ClearBackground(BLACK);

        if (!error)
        {
            r = ctx->Prepare(drawFunc);
            if (r < 0)
            {
                errorHandler("Failed to prepare the context.\n");
            }
            else
                callFunction(ctx, drawFunc);
        }
        else
        {
            ClearBackground(SKYBLUE);
            DrawText(errorMessage.message.c_str(), 10, 10, 20, WHITE);

            const char *type = "ERR";

            if (errorMessage.type == asMSGTYPE_WARNING)
                type = "WARN";
            else if (errorMessage.type == asMSGTYPE_INFORMATION)
                type = "INFO";

            DrawText(TextFormat("%s (%d, %d) : %s", errorMessage.section.c_str(), errorMessage.line, errorMessage.column, type), 10, 40, 20, WHITE);

            int y = 0;
            for (auto &i : errorMessage.tracelog)
            {
                DrawText(i.c_str(), 10, 70 + y, 20, WHITE);
                y += 30;
            }
        }

        EndTextureMode();

        if (mode == MODE_RUNTIME)
        {
            DrawTexturePro(target.texture, (Rectangle){ 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height },
                        (Rectangle){ (GetScreenWidth() - ((float)WIDTH*scale))*0.5f, (GetScreenHeight() - ((float)HEIGHT*scale))*0.5f,
                        (float)WIDTH*scale, (float)HEIGHT*scale }, (Vector2){ 0, 0 }, 0.0f, WHITE);
        }

        rlImGuiBegin();

        if (mode == MODE_DEV)
        {
            ImGui::DockSpaceOverViewport();

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
                        reload();
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
        }

        rlImGuiEnd();

        EndDrawing();
    }

    if (!error)
    {
        ctx->Release();
        engine->ShutDownAndRelease();
    }

    rlImGuiShutdown();

    UnloadRenderTexture(target);

    CloseWindow();

    return 0;
}
