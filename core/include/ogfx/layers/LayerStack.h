#pragma once

#include <memory>
#include <vector>

namespace ogfx {
    struct Layer {
        virtual ~Layer() = default;

        virtual void Init() {}
        virtual void Update() {}
        virtual void Render() {}
        virtual void RenderGUI() {}
        virtual void Present() {}
        virtual void Shutdown() {}
    };

    class LayerStack {
    public:
        template<typename LayerType, typename ...Args>
        LayerType& AddLayer(Args&&... args) {
            return *static_cast<LayerType*>(m_layers.emplace_back(std::make_unique<LayerType>(std::forward<Args>(args)...)).get());
        }

        void Init() {
            for (auto& layer : m_layers) {
                layer->Init();
            }
        }

        void Update() {
            for (auto& layer : m_layers) {
                layer->Update();
            }
        }

        void Render() {
            for (auto& layer : m_layers) {
                layer->Render();
            }
        }

        void RenderGUI() {
            for (auto& layer : m_layers) {
                layer->RenderGUI();
            }
        }

        void Present() {
            for (auto& layer : m_layers) {
                layer->Present();
            }
        }

        void Shutdown() {
            for (auto& layer : m_layers) {
                layer->Shutdown();
            }

            m_layers.clear();
        }

    private:
        std::vector<std::unique_ptr<Layer>> m_layers;
    };
}
