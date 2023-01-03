#include "CheckersUi.h"
#include <vector>
#include <algorithm>

namespace Interfaces
{
	using namespace System::Collections::Generic;
	using namespace System::Drawing;
	using namespace TrainingCell;

	CheckersUi::CheckersUi(Panel^ panel)
	{
		if (panel == nullptr)
			throw new std::exception("Invalid UI element");

		
		state() = Checkers::State::set_start_state();
		preview_state() = state();

		_panel = panel;
		_panel->Resize += gcnew System::EventHandler(this, &CheckersUi::panel_resize);
		_panel->Paint += gcnew PaintEventHandler(this, &CheckersUi::panel_paint);
		_panel->MouseDown += gcnew MouseEventHandler(this, &CheckersUi::panel_mouse_down);
		Draw();
	}

	CheckersUi::!CheckersUi()
	{
		if (_currentStatePtr != nullptr)
		{
			delete _currentStatePtr;
			_currentStatePtr = nullptr;
		}

		if (_previewStatePtr != nullptr)
		{
			delete _previewStatePtr;
			_previewStatePtr = nullptr;
		}

		dispose_moves();
	}

	CheckersUi::~CheckersUi()
	{
		throw std::exception("Not implemented");
	}

	Checkers::State& CheckersUi::state()
	{
		if (_currentStatePtr == nullptr)
			_currentStatePtr = new Checkers::State;
		return *_currentStatePtr;
	}

	Checkers::State& CheckersUi::preview_state()
	{
		if (_previewStatePtr == nullptr)
			_previewStatePtr = new Checkers::State;
		return *_previewStatePtr;
	}

	std::vector<Checkers::Move>& CheckersUi::moves()
	{
		if (_availableMoves == nullptr)
		{
			_availableMoves = new std::vector<Checkers::Move>;
			*_availableMoves = Checkers::Utils::get_moves(state());

			if (!(*_availableMoves).empty())
				_current_move_id = 0;
		}

		return *_availableMoves;
	}

	void CheckersUi::dispose_moves()
	{
		if (_availableMoves != nullptr)
		{
			delete _availableMoves;
			_availableMoves = nullptr;
		}

		_current_move_id = -1;
	}

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

		_fieldSize = _boardSize / Checkers::BoardRows;

		for (auto fieldYId = 0; fieldYId < Checkers::BoardRows; fieldYId ++)
			for (auto fieldXId = (fieldYId + (black ? 1 : 0)) % 2; fieldXId < Checkers::BoardRows; fieldXId += 2)
				result->Add(GetFieldRectangle(fieldXId, fieldYId));

		return result->ToArray();
	}

	void CheckersUi::Draw()
	{
		_boardSize = (std::min(_panel->Width, _panel->Height) / Checkers::BoardRows) * Checkers::BoardRows;
		_xStart = std::max(0, (_panel->Width - _boardSize) / 2);
		_yStart = std::max(0, (_panel->Height - _boardSize) / 2);

		auto gr = _panel->CreateGraphics();
		gr->Clear(_panel->BackColor);
		gr->FillRectangles(Brushes::SaddleBrown, GetFields(true));
		gr->FillRectangles(Brushes::BurlyWood, GetFields(false));
		gr->DrawRectangle(gcnew Pen(Brushes::Red, 5), GetSelectedField());

		if (_current_move_id >= 0)
		{
			auto state_preview = state();
			state_preview.make_move(moves()[_current_move_id], false, true);

			if (_inverted)
				state_preview = state_preview.invert();

			DrawState(state_preview, gr);
		}
		else
		{
			const auto state_to_draw = _inverted ? state().invert() : state();
			DrawState(state_to_draw, gr);
		}

		delete gr;
	}

	/// <summary>
	///	Splits the given "single value" onto x (column) and y (row) components
	/// </summary>
	Point SplitFieldId(const int fieldId)
	{
		return Point(fieldId % 8, fieldId / 8);
	}

	void CheckersUi::DrawState(const Checkers::State& state, Graphics^ gr)
	{
		for (auto itemId = 0ull; itemId < state.size(); itemId++)
		{
			const auto piece = state[itemId];
			if (piece == Checkers::Piece::Space)
				continue;

			const auto fieldCoords = Checkers::State::plain_id_to_piece_position(itemId);

			auto fieldRectangle = GetFieldRectangle(
				static_cast<int>(fieldCoords.col), 
				static_cast<int>(fieldCoords.row));

			if (Checkers::Utils::is_alive(piece))
			{
				fieldRectangle.Inflate(-5, -5);
				gr->FillEllipse(Checkers::Utils::is_allay_piece(piece) ? Brushes::WhiteSmoke : Brushes::Black, fieldRectangle);

				fieldRectangle.Inflate(-10, -10);
				gr->FillEllipse(Checkers::Utils::is_king(piece) ? Brushes::Red : Brushes::Green, fieldRectangle);
			} else if (Checkers::Utils::is_dead(piece))
			{
				fieldRectangle.Inflate(-10, -10);
				gr->FillEllipse(Brushes::Gray, fieldRectangle);
			} else if (Checkers::Utils::is_trace_marker(piece))
			{
				fieldRectangle.Inflate(-10, -10);
				gr->FillEllipse(Brushes::Yellow, fieldRectangle);
			}
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

	System::Void CheckersUi::panel_mouse_down(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {
		_selectedFieldId = panel_coord_to_field_coord(e->X, e->Y);
		Draw();
	}

	void CheckersUi::NextActionCallBack()
	{
		const auto& m = moves();

		if (_current_move_id < 0)
			return;

		_current_move_id = (_current_move_id + 1) % m.size();

		Draw();
	}

	void CheckersUi::MakeMoveCallBack()
	{
		if (_current_move_id >= 0)
			state().make_move(moves()[_current_move_id], true);

		_inverted =! _inverted;
		dispose_moves();

		state() = state().invert();

		Draw();
	}


}
