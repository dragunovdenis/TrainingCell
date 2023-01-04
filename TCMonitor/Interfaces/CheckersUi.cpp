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
			auto move = moves()[_current_move_id];
			state_preview.make_move(move, false, true);

			if (_inverted)
			{
				state_preview = state_preview.get_inverted();
				move.invert();
			}

			draw_state(state_preview, move, gr);
		}
		else
		{
			const auto state_to_draw = _inverted ? state().get_inverted() : state();
			draw_state(state_to_draw, Checkers::Move(), gr);
		}

		delete gr;
	}

	/// <summary>
	///	Splits the given "single value" onto x (column) and y (row) components
	/// </summary>
	Point split_field_id(const int fieldId)
	{
		return Point(fieldId % 8, fieldId / 8);
	}

	/// <summary>
	///	Returns center point of the given rectangle
	/// </summary>
	Point calc_center(Rectangle rec)
	{
		return Point( rec.X + rec.Width / 2, rec.Y + rec.Height / 2 );
	}

	void CheckersUi::draw_state(const Checkers::State& state,
		const TrainingCell::Checkers::Move& move_to_articulate, Graphics^ gr)
	{
		const auto size_correction_rate = static_cast<int>(5.0 / 50 * _fieldSize);

		for (auto item_id = 0ull; item_id < state.size(); item_id++)
		{
			const auto piece = state[item_id];
			if (piece == Checkers::Piece::Space)
				continue;

			const auto field_coords = Checkers::State::plain_id_to_piece_position(item_id);

			auto field_rectangle = GetFieldRectangle(
				static_cast<int>(field_coords.col), 
				static_cast<int>(field_coords.row));

			if (Checkers::Utils::is_alive(piece))
			{
				field_rectangle.Inflate(-size_correction_rate, - size_correction_rate);
				gr->FillEllipse(Checkers::Utils::is_allay_piece(piece) ? Brushes::WhiteSmoke : Brushes::Black, field_rectangle);

				gr->DrawEllipse(gcnew Pen(Color::BlueViolet, 3), field_rectangle);

				field_rectangle.Inflate(- 2 * size_correction_rate, - 2 * size_correction_rate);
				gr->FillEllipse(Checkers::Utils::is_king(piece) ? Brushes::Red : Brushes::Green, field_rectangle);
			} else if (Checkers::Utils::is_dead(piece))
			{
				field_rectangle.Inflate(-2 * size_correction_rate, -2 * size_correction_rate);
				gr->FillEllipse(Brushes::Gray, field_rectangle);
			} else if (Checkers::Utils::is_trace_marker(piece))
			{
				field_rectangle.Inflate(-2 * size_correction_rate, -2 * size_correction_rate);
				gr->FillEllipse(Brushes::Yellow, field_rectangle);
			}
		}

		if (move_to_articulate.is_valid())
		{
			for (const auto& sub_move : move_to_articulate.sub_moves)
			{
				const auto field_rectangle_start = GetFieldRectangle(
					static_cast<int>(sub_move.start.col),
					static_cast<int>(sub_move.start.row));
				
				const auto field_rectangle_end = GetFieldRectangle(
					static_cast<int>(sub_move.end.col),
					static_cast<int>(sub_move.end.row));

				gr->DrawLine(gcnew Pen(Color::Black, 5), calc_center(field_rectangle_start), calc_center(field_rectangle_end));
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
		state().invert();
		Draw();
	}
}
