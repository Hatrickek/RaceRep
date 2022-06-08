#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Assets.h"
#include <imgui.h>
#include <stdio.h>
#include "IconsFontAwesome5.h"
GLuint textures[4];
ImFont* paradigma_24;
bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
    stbi_set_flip_vertically_on_load(1);
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
    {
        printf("Couldn't find the image file! :( \n");
        return false;
    }

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;
    return true;
}
void LoadTextures()
{
    int width, height;
    LoadTextureFromFile("assets\\images\\steering1.png", &textures[0], &width, &height);
    LoadTextureFromFile("assets\\images\\steerknob2.png", &textures[1], &width, &height);
    int bgWidth, bgHeight;
    LoadTextureFromFile("assets\\images\\demobg.png", &textures[2], &bgWidth, &bgHeight);
}
void LoadFonts()
{
    ImGuiIO& io = ImGui::GetIO(); (void)io;
  
    ImFontConfig config;
    config.MergeMode = true; 
    config.PixelSnapH = true;
    static const ImWchar icons_ranges[] = { 0xe005, 0xf8ff, 0 };
    ImFont* paradigma = io.Fonts->AddFontFromFileTTF("assets\\fonts\\Roboto-Regular.ttf", 16.0f);
    ImFont* font2 = io.Fonts->AddFontFromFileTTF("assets\\fonts\\fa-solid-900.ttf", 16.0f, &config, icons_ranges);
    ImFont* defaultFont = io.Fonts->AddFontDefault();
    
    io.Fonts->Build();
}
//void LoadFont()
//{
    // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
  // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
  // - Read 'docs/FONTS.md' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
  //io.Fonts->AddFontDefault();
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
  //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
  //IM_ASSERT(font != NULL);
//}
