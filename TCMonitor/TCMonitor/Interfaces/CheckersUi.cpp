#include "CheckersUi.h"
#include <vector>
#include <algorithm>

namespace Interfaces
{
	using namespace System::Collections::Generic;
	using namespace System::Drawing;

	CheckersUi::CheckersUi(Panel^ panel)
	{
		if (panel == nullptr)
			throw new std::exception("Invalid UI element");

		_panel = panel;
		_panel->Resize += gcnew System::EventHandler(this, &CheckersUi::panel_resize);
		_panel->Click += gcnew System::EventHandler(this, &CheckersUi::panel_click);
		_panel->Paint += gcnew PaintEventHandler(this, &CheckersUi::panel_paint);
		_panel->MouseDown += gcnew MouseEventHandler(this, &CheckersUi::panel_mouse_down);
		Draw();
	}

	/// <summary>
	///	Number of fields in the vertical and horizontal dimensions
	/// </summary>
	constexpr int FieldsDim = 8;

	/// <summary>
	///	Returns rectangle that corresponds to the field with the given id
	/// </summary>
	Rectangle CheckersUi::GetFieldRectangle(const int fieldXId, const int fieldYId)
	{
		return Rectangle(_xStart + fieldXId * _fieldSize, _yStart + fieldYId * _fieldSize, _fieldSize, _fieldSize);
	}

	array<Rectangle>^ CheckersUi::GetFields(const bool black)
	{
		auto result = gcnew List<Rectangle>();

		_fieldSize = _boardSize / FieldsDim;

		for (auto fieldYId = 0; fieldYId < FieldsDim; fieldYId ++)
			for (auto fieldXId = (fieldYId + (black ? 1 : 0)) % 2; fieldXId < FieldsDim; fieldXId += 2)
				result->Add(GetFieldRectangle(fieldXId, fieldYId));

		return result->ToArray();
	}

	void CheckersUi::Draw()
	{
		_boardSize = (std::min(_panel->Width, _panel->Height) / FieldsDim) * FieldsDim;
		_xStart = std::max(0, (_panel->Width - _boardSize) / 2);
		_yStart = std::max(0, (_panel->Height - _boardSize) / 2);

		auto gr = _panel->CreateGraphics();
		gr->Clear(_panel->BackColor);
		gr->FillRectangles(Brushes::Brown, GetFields(true));
		gr->FillRectangles(Brushes::BurlyWood, GetFields(false));
		gr->DrawRectangle(gcnew Pen(Brushes::Red, 5), GetSelectedField());

		DrawState({ 1, 1, 1, 1, 1, 1, 1, 1,
					 0, 0, 0, 0, 0, 0, 0, 0,
					 0, 0, 0, 0, 0, 0, 0, 0,
					 -1, -1, -1, -1, -1, -1, -1, -1,
			}, gr);

		delete gr;
	}

	/// <summary>
	///	Splits the given "single value" onto x (column) and y (row) components
	/// </summary>
	Point SplitFieldId(const int fieldId)
	{
		return Point(fieldId % 8, fieldId / 8);
	}

	void CheckersUi::DrawState(const std::vector<int>& state, Graphics^ gr)
	{
		if (state.size() != 32)
			throw std::exception("Invalid status");

		for (auto itemId = 0ull; itemId < state.size(); itemId++)
		{
			if (state[itemId] == 0)
				continue;

			const auto fieldId = 2 * itemId + 1;
			auto fieldCoords = SplitFieldId(fieldId);
			if (fieldCoords.Y % 2 != 0)
				fieldCoords.X -= 1;

			auto fieldRectangle = GetFieldRectangle(fieldCoords.X, fieldCoords.Y);

			fieldRectangle.Inflate(-5, -5);
			gr->FillEllipse(state[itemId] > 0 ? Brushes::WhiteSmoke : Brushes::Black, fieldRectangle);

			fieldRectangle.Inflate(-10, -10);
			gr->FillEllipse(std::abs(state[itemId]) > 1 ? Brushes::IndianRed : Brushes::Green, fieldRectangle);
		}
	}

	Point CheckersUi::panel_coord_to_field_coord(const int x, const int y)
	{
		if (x < _xStart || x > _xStart + _boardSize || y < _yStart || y > _yStart + _boardSize)
			return Point(-1, -1);

		return Point((x - _xStart) / _fieldSize, (y - _yStart) / _fieldSize);
	}

	Rectangle CheckersUi::GetSelectedField()
	{
		if (_selectedFieldId.X < 0 || _selectedFieldId.Y < 0)
			return Rectangle(0, 0, 0, 0);//Empty rectangle

		return GetFieldRectangle(_selectedFieldId.X, _selectedFieldId.Y);
	}

	System::Void CheckersUi::panel_resize(System::Object^ sender, System::EventArgs^ e) {
		Draw();
	}

	System::Void CheckersUi::panel_paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e) {
		Draw();
	}

	System::Void CheckersUi::panel_click(System::Object^ sender, System::EventArgs^ e)
	{
	}

	System::Void CheckersUi::panel_mouse_down(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {
		_selectedFieldId = panel_coord_to_field_coord(e->X, e->Y);
		Draw();
	}


}
