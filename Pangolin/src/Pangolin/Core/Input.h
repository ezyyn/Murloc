#pragma once

namespace PG {
	class Input {
	public:
		struct MousePosition {
			float X;
			float Y;
		};

		static bool IsKeyPressed(int keycode);
		static bool IsKeyReleased(int keycode);

		static bool IsMouseButtonPressed(int button);

		static float GetMouseX();
		static float GetMouseY();

		static MousePosition GetMousePosition();
	private:
	};
}