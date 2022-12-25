#pragma once
#include <exception>
#include <vector>

namespace Interfaces
{
	using namespace System::Windows::Forms;

	/// <summary>
	///	Functionality to visualize UI for Checkers
	/// </summary>
	public ref class CheckersUi
	{
		Panel^ _panel;

		int _xStart;
		int _yStart;
		int _boardSize;
		int _fieldSize;
		System::Drawing::Point _selectedFieldId = System::Drawing::Point(-1, -1);

		/// <summary>
		///	Resize event handler
		/// </summary>
		System::Void panel_resize(System::Object^ sender, System::EventArgs^ e);

		/// <summary>
		///	"Paint" event handler
		/// </summary>
		System::Void panel_paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e);

		/// <summary>
		///	"Click" event handler
		/// </summary>
		System::Void panel_click(System::Object^ sender, System::EventArgs^ e);

		/// <summary>
		///	"Mouse down" event handler
		/// </summary>
		System::Void panel_mouse_down(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e);

		/// <summary>
		///	Method for drawing
		/// </summary>
		void Draw();

		/// <summary>
		///	Method to draw the given state, which is represented as an ordered collection of 32 integer items.
		///	Each item takes value from {-2, -1, 0, 1, 2}, representing "king piece" (+/-2), "regular piece" (+/-1),
		///	"empty field" (0)
		/// </summary>
		void DrawState(const std::vector<int>& state, System::Drawing::Graphics^ gr);

		/// <summary>
		///	Returns collection of rectangles that correspond to black or white fields of the checkers board
		/// </summary>
		array<System::Drawing::Rectangle>^ GetFields(const bool black);

		/// <summary>
		///	Returns a rectangle (square) that corresponds to the selected field, if such exists,
		///	otherwise returns an empty rectangle (of zero area)
		/// </summary>
		System::Drawing::Rectangle GetSelectedField();

		/// <summary>
		///	Returns rectangle that corresponds to the field with the given id
		/// </summary>
		System::Drawing::Rectangle GetFieldRectangle(const int fieldXId, const int fieldYId);

		/// <summary>
		///	Converts the given coordinates on the panel surface into the corresponding field coordinates
		///	or (-1, -1) if there is not corresponding field
		/// </summary>
		System::Drawing::Point panel_coord_to_field_coord(const int x, const int y);
	public:
		/// <summary>
		///	Constructor
		/// </summary>
		/// <param name="panel">Panel that the interface will be drawn on</param>
		CheckersUi(Panel^ panel);
	};
}
