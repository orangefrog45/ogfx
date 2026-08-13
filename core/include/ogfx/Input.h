#pragma once
#include <unordered_map>
#include <lml/core.h>

struct GLFWwindow;

namespace ogfx {
	enum MouseButton {
		LEFT_BUTTON = 0,
		RIGHT_BUTTON = 1,
		SCROLL = 2,
		NONE = 3,
	};

	enum MouseAction {
		UP = 0,
		DOWN = 1,
		MOVE = 2,
		TOGGLE_VISIBILITY,
	};

	enum InputType : uint8_t {
		RELEASE = 0, // GLFW_RELEASE
		PRESS = 1, // GLFW_PRESS
		HELD = 2 // GLFW_REPEAT
	};

	enum Key : uint16_t {
		// Alphabet keys
		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,

		// Numeric keys
		Zero = 48,
		One = 49,
		Two = 50,
		Three = 51,
		Four = 52,
		Five = 53,
		Six = 54,
		Seven = 55,
		Eight = 56,
		Nine = 57,

		// Function keys
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,

		// Special keys
		Space = 32,
		Enter = 257,
		Tab = 258,
		CapsLock = 280,
		Shift = 340,
		LeftControl = 341,
		RightControl = 345,
		Alt = 342,
		Escape = 256,
		Backspace = 259,
		Delete = 261,
		ArrowUp = 265,
		ArrowDown = 264,
		ArrowLeft = 263,
		ArrowRight = 262,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,
		Insert = 260,
		Accent = 96,
	};


	enum CursorStyle {
		ARROW = 0,
		I_BEAM = 1,
		VRESIZE = 2,
		HRESIZE = 3,
		HAND = 4,
		NUM_CURSORS = 5
	};

	class Input {
		friend class Window;
        friend void GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        friend void GlfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
        friend void GlfwCursorPosCallback(GLFWwindow* window, double xpos, double ypos);
        friend void GlfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	public:
		struct ScrollState {
			bool active = false;
			lml::vec2 offset = { 0, 0 };
		};

		[[nodiscard]] ScrollState GetScrollState() const {
			return m_scroll_state;
		}

		// Not case-sensitive
		// Returns true if key has been pressed this frame or is being held down
		[[nodiscard]] bool IsKeyDown(Key key) const {
			return m_key_states.contains(key) && m_key_states.at(key) != InputType::RELEASE;
		}

		// Not case-sensitive
		// Returns true if key has been pressed this frame or is being held down
		[[nodiscard]] bool IsKeyDown(int key) const {
			Key k = static_cast<Key>(std::toupper(key));
			return m_key_states.contains(k) && m_key_states.at(k) != InputType::RELEASE;
		}

		// Not case-sensitive
		// Returns true only if key has been pressed this frame
		[[nodiscard]] bool IsKeyPressed(Key key) const {
			return m_key_states.contains(key) && m_key_states.at(key) == InputType::PRESS;
		}

		// Not case-sensitive
		// Returns true only if key has been pressed this frame
		[[nodiscard]] bool IsKeyPressed(int key) const {
			Key k = static_cast<Key>(std::toupper(key));
			return m_key_states.contains(k) && m_key_states.at(k) == InputType::PRESS;
		}

		[[nodiscard]] bool IsMouseDown(MouseButton btn) const {
			return m_mouse_states.contains(btn) && m_mouse_states.at(btn) != InputType::RELEASE;
		}

		[[nodiscard]] bool IsMouseClicked(MouseButton btn) const {
			return m_mouse_states.contains(btn) && m_mouse_states.at(btn) == InputType::PRESS;
		}

		[[nodiscard]] bool IsMouseDown(unsigned btn) const {
			MouseButton b = static_cast<MouseButton>(btn);
			return m_mouse_states.contains(b) && m_mouse_states.at(b) != InputType::RELEASE;
		}

		[[nodiscard]] bool IsMouseClicked(unsigned btn) const {
			MouseButton b = static_cast<MouseButton>(btn);
			return m_mouse_states.contains(b) && m_mouse_states.at(b) == InputType::PRESS;
		}

		[[nodiscard]] lml::ivec2 GetMousePos() const {
			return m_mouse_position;
		}

		[[nodiscard]] lml::vec2 GetMouseDelta() const {
			return lml::vec2((float)m_mouse_position.x - (float)m_last_mouse_position.x, (float)m_mouse_position.y - (float)m_last_mouse_position.y);
		}

		void Update() {
			m_scroll_state.active = false;
			m_scroll_state.offset = { 0,0 };

			m_last_mouse_position = m_mouse_position;
			// Press state only valid for one frame, then the key is considered "held"
			for (auto& [key, v] : m_key_states) {
				if (v == InputType::PRESS)
					v = InputType::HELD;
			}

			for (auto& [key, v] : m_mouse_states) {
				if (v == InputType::PRESS)
					v = InputType::HELD;
			}
		}

	private:
		lml::ivec2 m_mouse_position{ 0, 0 };
		lml::ivec2 m_last_mouse_position{ 0, 0 };

		std::unordered_map<Key, InputType> m_key_states;
		std::unordered_map<MouseButton, InputType> m_mouse_states;
		ScrollState m_scroll_state;
	};
}