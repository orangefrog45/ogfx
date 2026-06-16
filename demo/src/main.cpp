#include "../headers/ComputeDemoLayer.h"
#include "ogfx/Engine.h"

int main() {
    ogfx::Window window{1920, 1080};
    ogfx::Engine engine{};
    ComputeDemoLayer& demo_layer = engine.layers.AddLayer<ComputeDemoLayer>(window);

    ogfx::Engine::InitData init_data{};
    init_data.p_window = &window;
    init_data.imgui_rd_func = [&] {
        ogfx::ImGuiRenderData rd;
        rd.p_output_image = &demo_layer.GetRenderImage();
        rd.output_image_layout = vk::ImageLayout::eGeneral;

        return rd;
    };

    engine.Init(init_data, "Demo");
    engine.MainLoop();
    engine.Cleanup();

    return 0;
}