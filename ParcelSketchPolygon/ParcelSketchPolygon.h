#pragma once

namespace ParcelSketchPolygon {

	using namespace System;
	using namespace System::Collections::Generic;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Linq;
	using namespace System::IO;
	using namespace System::Text;
	using namespace System::Drawing;

	using namespace System::Drawing::Drawing2D;

	/// <summary>
	/// Summary for MainForm
	/// </summary>
	public ref class MainForm : public System::Windows::Forms::Form
	{
	public:
		MainForm(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

		// The grid spacing.
		const System::Int32 grid_gap = 8;

		// The "size" of an object for mouse over purposes.
	private: const System::Int32 object_radius = 3;

			 // We're over an object if the distance squared
			 // between the mouse and the object is less than this.
	private: const System::Int32 over_dist_squared = object_radius * object_radius;

			 // Each polygon is represented by a List<Point>.
	private: List<List<Point>^>^ Polygons = gcnew List<List<Point>^>();

			 // Points for the new polygon.
	private: List<Point>^ NewPolygon = nullptr;

			 // The current mouse position while drawing a new polygon.
	private: Point NewPoint;

			 // The polygon and index of the corner we are moving.
	private: List<Point>^ MovingPolygon = nullptr;


			 // The add point cursor.
	private: System::Windows::Forms::Cursor^ AddPointCursor;

			 // Create the add point cursor.
	private: void MainForm_Load(System::Object^  sender, System::EventArgs^   e)
	{
		//Reflection::Assembly^ pxAssembly = Reflection::Assembly::GetExecutingAssembly();
		////Note: add your resourcefile name here, i.e. ".MyResourceFile" as it appears in solution explorer, without it's extension
		//String^ pxResName = pxAssembly->GetName()->Name + ".add_point";
		//auto s_pxResourceManager = (gcnew Resources::ResourceManager(pxResName, pxAssembly));

		////note: this is the exact name from the resx, not the filename or name in solution explorer. By default it's name in the resx won't contain it's extension.
		//auto image = (cli::safe_cast<Drawing::Bitmap^>(s_pxResourceManager->GetObject("add_point")));

		//AddPointCursor = gcnew System::Windows::Forms::Cursor(image->GetHbitmap());

		AddPointCursor = System::Windows::Forms::Cursor::Current; // <<-- TODO: Come back to this Cursor whipping my ass.

		MakeBackgroundGrid();
	}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~MainForm()
		{
			if (components)
			{
				delete components;
			}
		}

	private: System::Windows::Forms::PictureBox^  picCanvas;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = gcnew System::ComponentModel::Container();
			this->picCanvas = (gcnew System::Windows::Forms::PictureBox());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->picCanvas))->BeginInit();
			this->SuspendLayout();
			// 
			// picCanvas
			// 
			this->picCanvas->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->picCanvas->Location = System::Drawing::Point(12, 12);
			this->picCanvas->Name = L"picCanvas";
			this->picCanvas->Size = System::Drawing::Size(260, 237);
			this->picCanvas->TabIndex = 0;
			this->picCanvas->TabStop = false;
			this->picCanvas->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &MainForm::picCanvas_Paint);
			this->picCanvas->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseDown);
			this->picCanvas->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_NotDrawing);
			this->picCanvas->Resize += gcnew System::EventHandler(this, &MainForm::picCanvas_Resize);
			// 
			// MainForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(284, 261);
			this->Controls->Add(this->picCanvas);
			this->Name = L"MainForm";
			this->Text = L"ParcelSketch - Polygon v1";
			this->Load += gcnew System::EventHandler(this, &ParcelSketchPolygon::MainForm::MainForm_Load);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->picCanvas))->EndInit();
			this->ResumeLayout(false);

		}
#pragma endregion




	private: System::Void picCanvas_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
	{
		// See what we're over.
		Point mouse_pt = SnapToGrid(e->Location);
		List<Point> ^hit_polygon;
		int hit_point, hit_point2;
		Point closest_point;

		if (NewPolygon != nullptr)
		{
			// We are already drawing a polygon.
			// If it's the right mouse button, finish this polygon.
			if (e->Button == System::Windows::Forms::MouseButtons::Right)
			{
				// Finish this polygon.
				if (NewPolygon->Count > 2) Polygons->Add(NewPolygon);
				NewPolygon = nullptr;

				// We no longer are drawing.
				picCanvas->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_NotDrawing);
				picCanvas->MouseMove -= gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_Drawing);
			}
			else
			{
				// Add a point to this polygon.
				if (NewPolygon[NewPolygon->Count - 1] != mouse_pt)
				{
					NewPolygon->Add(mouse_pt);
				}
			}
		}
		else if (MouseIsOverCornerPoint(mouse_pt, hit_polygon, hit_point))
		{
			// Start dragging this corner.
			picCanvas->MouseMove -= gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_NotDrawing);
			picCanvas->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_MovingCorner);
			picCanvas->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseUp_MovingCorner);

			// Remember the polygon and point number.
			MovingPolygon = hit_polygon;
			MovingPoint = hit_point;

			// Remember the offset from the mouse to the point.
			OffsetX = hit_polygon[hit_point].X - e->X;
			OffsetY = hit_polygon[hit_point].Y - e->Y;
		}
		else if (MouseIsOverEdge(mouse_pt, hit_polygon,
			hit_point, hit_point2, closest_point))
		{
			// Add a point.
			hit_polygon->Insert(hit_point + 1, closest_point);
		}
		else if (MouseIsOverPolygon(mouse_pt, hit_polygon))
		{
			// Start moving this polygon.
			picCanvas->MouseMove -= gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_NotDrawing);
			picCanvas->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_MovingPolygon);
			picCanvas->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseUp_MovingPolygon);

			// Remember the polygon.
			MovingPolygon = hit_polygon;

			// Remember the offset from the mouse to the segment's first point.
			OffsetX = hit_polygon[0].X - e->X;
			OffsetY = hit_polygon[0].Y - e->Y;
		}
		else
		{
			// Start a new polygon.
			NewPolygon = gcnew List<Point>();
			NewPoint = mouse_pt;
			NewPolygon->Add(mouse_pt);

			// Get ready to work on the new polygon.
			picCanvas->MouseMove -= gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_NotDrawing);
			picCanvas->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_Drawing);
		}

		// Redraw.
		picCanvas->Invalidate();
	}


			 // Snap to the nearest grid point.
	private: Point SnapToGrid(Point point)
	{
		//if (!chkSnapToGrid->Checked) return;
		int x = grid_gap * (System::Int32)Math::Round((System::Double)point.X / grid_gap);
		int y = grid_gap * (System::Int32)Math::Round((System::Double)point.Y / grid_gap);
		return  Point(x, y);
	}

#pragma region "Drawing"

			 // Move the next point in the new polygon.
	private: System::Void picCanvas_MouseMove_Drawing(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
	{
		// Save the new point.
		NewPoint = SnapToGrid(e->Location);

		// Redraw.
		picCanvas->Invalidate();
	}


#pragma endregion // Drawing

#pragma region "Moving End Point"

			 // The segment/point we're moving or the segment whose end point we're moving.
	private: System::Int32 MovingPoint = -1;

			 // The end point we're moving.
	private: System::Boolean MovingStartEndPoint = false;

			 // The offset from the mouse to the object being moved.
	private: System::Int32 OffsetX, OffsetY;

			 // Move the selected corner.
	private: System::Void picCanvas_MouseMove_MovingCorner(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
	{
		// Move the point to its new location.		
		MovingPolygon[MovingPoint] =
			SnapToGrid(Point(e->X + OffsetX, e->Y + OffsetY));


		// Redraw.
		picCanvas->Invalidate();
	}

			 // Finish moving the selected corner.
	private: System::Void picCanvas_MouseUp_MovingCorner(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
	{
		// Reset the event handlers.
		picCanvas->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_NotDrawing);
		picCanvas->MouseMove -= gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_MovingCorner);
		picCanvas->MouseUp -= gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseUp_MovingCorner);

		// Redraw.
		picCanvas->Invalidate();
	}

#pragma endregion // Moving End Point

#pragma region "Moving Polygon"

			 // Move the selected polygon.
	private: System::Void picCanvas_MouseMove_MovingPolygon(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
	{
		// See how far the first point will move.
		System::Int32 new_x1 = e->X + OffsetX;
		System::Int32 new_y1 = e->Y + OffsetY;


		System::Int32 dx = new_x1 - MovingPolygon[0].X;
		System::Int32 dy = new_y1 - MovingPolygon[0].Y;

		// Snap the movement to a multiple of the grid distance.
		dx = grid_gap * (int)(Math::Round((float)dx / grid_gap));
		dy = grid_gap * (int)(Math::Round((float)dy / grid_gap));

		if (dx == 0 && dy == 0) return;

		// Move the polygon.
		for (int i = 0; i < MovingPolygon->Count; i++)
		{
			MovingPolygon[i] = Point(
				MovingPolygon[i].X + dx,
				MovingPolygon[i].Y + dy);
		}

		// Redraw.
		picCanvas->Invalidate();
	}

			 // Finish moving the selected polygon.
	private: System::Void picCanvas_MouseUp_MovingPolygon(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
	{
		// Reset the event handlers.
		picCanvas->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_NotDrawing);
		picCanvas->MouseMove -= gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_MovingPolygon);
		picCanvas->MouseUp -= gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseUp_MovingPolygon);

		// Redraw.
		picCanvas->Invalidate();
	}



#pragma endregion // Moving End Point

			 // See if we're over a polygon or corner point.
	private: void picCanvas_MouseMove_NotDrawing(System::Object^  sender, System::Windows::Forms::MouseEventArgs^ e)
	{
		auto new_cursor = Cursors::Cross;

		// See what we're over.
		Point mouse_pt = SnapToGrid(e->Location);
		List<Point> ^hit_polygon;
		int hit_point, hit_point2;
		Point closest_point;

		if (MouseIsOverCornerPoint(mouse_pt, hit_polygon, hit_point))
		{
			new_cursor = Cursors::Arrow;
		}
		else if (MouseIsOverEdge(mouse_pt, hit_polygon,
			hit_point, hit_point2, closest_point))
		{
			new_cursor = AddPointCursor;
		}
		else if (MouseIsOverPolygon(mouse_pt, hit_polygon))
		{
			new_cursor = Cursors::Hand;
		}

		// Set the new cursor.
		if (picCanvas->Cursor != new_cursor)
		{
			picCanvas->Cursor = new_cursor;
		}
	}


			 // See if the mouse is over a corner point.
	private: System::Boolean MouseIsOverCornerPoint
			 (Point mouse_pt,
				 [Runtime::InteropServices::Out] List<Point>^% hit_polygon,
				 [Runtime::InteropServices::Out] int %hit_pt)
	{
		// See if we're over a corner point.
		for each(List<Point> ^polygon in Polygons)
		{
			// See if we're over one of the polygon's corner points.
			for (int i = 0; i < polygon->Count; i++)
			{
				// See if we're over this point.
				if (FindDistanceToPointSquared(polygon[i], mouse_pt) < over_dist_squared)
				{
					// We're over this point.
					hit_polygon = polygon;
					hit_pt = i;
					return true;
				}
			}
		}

		hit_polygon = nullptr;
		hit_pt = -1;
		return false;
	}

			 // See if the mouse is over a polygon's edge.
	private: System::Boolean MouseIsOverEdge
			 (Point mouse_pt,
				 [Runtime::InteropServices::Out] List<Point>^% hit_polygon,
				 [Runtime::InteropServices::Out] int %hit_pt1,
				 [Runtime::InteropServices::Out] int %hit_pt2,
				 [Runtime::InteropServices::Out] Point %closest_point)
	{
		// Examine each polygon.
		// Examine them in reverse order to check the ones on top first.
		for (int pgon = Polygons->Count - 1; pgon >= 0; pgon--)
		{
			List<Point> ^polygon = Polygons[pgon];

			// See if we're over one of the polygon's segments.
			for (int p1 = 0; p1 < polygon->Count; p1++)
			{
				// Get the index of the polygon's next point.
				int p2 = (p1 + 1) % polygon->Count;

				// See if we're over the segment between these points.
				PointF closest;
				if (FindDistanceToSegmentSquared(mouse_pt,
					polygon[p1], polygon[p2], closest) < over_dist_squared)
				{
					// We're over this segment.
					hit_polygon = polygon;
					hit_pt1 = p1;
					hit_pt2 = p2;
					closest_point = Point::Round(closest);
					return true;
				}
			}
		}

		hit_polygon = nullptr;
		hit_pt1 = -1;
		hit_pt2 = -1;
		closest_point = Point(0, 0);
		return false;
	}

			 // See if the mouse is over a polygon's body.
	private: System::Boolean MouseIsOverPolygon(Point mouse_pt, [Runtime::InteropServices::Out] List<Point>^% hit_polygon)
	{
		// Examine each polygon.
		// Examine them in reverse order to check the ones on top first.
		for (int i = Polygons->Count - 1; i >= 0; i--)
		{
			// Make a GraphicsPath representing the polygon.
			GraphicsPath ^path = gcnew GraphicsPath();
			path->AddPolygon(Polygons[i]->ToArray());

			// See if the point is inside the GraphicsPath.
			if (path->IsVisible(mouse_pt))
			{
				hit_polygon = Polygons[i];
				return true;
			}
		}

		hit_polygon = nullptr;
		return false;
	}

#pragma region DistanceFunctions
			 // Calculate the distance squared between two points.
	private: System::Int32 FindDistanceToPointSquared(Point pt1, Point pt2)
	{
		int dx = pt1.X - pt2.X;
		int dy = pt1.Y - pt2.Y;
		return dx * dx + dy * dy;
	}

			 // Calculate the distance squared between
			 // point pt and the segment p1 --> p2.
	private: System::Double FindDistanceToSegmentSquared(Point pt, Point p1, Point p2, [Runtime::InteropServices::Out] PointF %closest)
	{
		float dx = p2.X - p1.X;
		float dy = p2.Y - p1.Y;
		if ((dx == 0) && (dy == 0))
		{
			// It's a point not a line segment.
			closest = p1;
			dx = pt.X - p1.X;
			dy = pt.Y - p1.Y;
			return dx * dx + dy * dy;
		}

		// Calculate the t that minimizes the distance.
		float t = ((pt.X - p1.X) * dx + (pt.Y - p1.Y) * dy) / (dx * dx + dy * dy);

		// See if this represents one of the segment's
		// end points or a point in the middle.
		if (t < 0)
		{
			closest = PointF(p1.X, p1.Y);
			dx = pt.X - p1.X;
			dy = pt.Y - p1.Y;
		}
		else if (t > 1)
		{
			closest = PointF(p2.X, p2.Y);
			dx = pt.X - p2.X;
			dy = pt.Y - p2.Y;
		}
		else
		{
			closest = PointF(p1.X + t * dx, p1.Y + t * dy);
			dx = pt.X - closest.X;
			dy = pt.Y - closest.Y;
		}

		return dx * dx + dy * dy;
	}

#pragma endregion DistanceFunctions


			 // Draw the lines.
	private: System::Void picCanvas_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e)
	{
		e->Graphics->SmoothingMode = SmoothingMode::AntiAlias;


		// Draw the old polygons.
		for each(List<Point> ^polygon in Polygons)
		{
			// Draw the polygon.
			e->Graphics->FillPolygon(Brushes::White, polygon->ToArray());
			e->Graphics->DrawPolygon(Pens::Blue, polygon->ToArray());

			// Draw the corners.
			for each(Point corner in polygon)
			{
				Rectangle rect = Rectangle(
					corner.X - object_radius, corner.Y - object_radius,
					2 * object_radius + 1, 2 * object_radius + 1);
				e->Graphics->FillEllipse(Brushes::White, rect);
				e->Graphics->DrawEllipse(Pens::Black, rect);
			}
		}

		// Draw the new polygon.
		if (NewPolygon != nullptr)
		{
			// Draw the new polygon.
			if (NewPolygon->Count > 1)
			{
				e->Graphics->DrawLines(Pens::Green, NewPolygon->ToArray());
			}

			// Draw the newest edge.
			if (NewPolygon->Count > 0)
			{
				Pen ^dashed_pen = gcnew Pen(Color::Green);

				dashed_pen->DashPattern = gcnew array<float>{ 3, 3 };
				e->Graphics->DrawLine(dashed_pen,
					NewPolygon[NewPolygon->Count - 1],
					NewPoint);

			}
		}
	}

			 // Give the PictureBox a grid background.
	private: System::Void picCanvas_Resize(System::Object^  sender, System::EventArgs^  e)
	{
		this->MakeBackgroundGrid();
	}

	private: System::Void chkSnapToGrid_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
	{
		this->MakeBackgroundGrid();
	}


	private: System::Void MakeBackgroundGrid()
	{

		Bitmap^ bm = gcnew Bitmap(
			picCanvas->ClientSize.Width,
			picCanvas->ClientSize.Height);
		for (int x = 0; x < picCanvas->ClientSize.Width; x += grid_gap)
		{
			for (int y = 0; y < picCanvas->ClientSize.Height; y += grid_gap)
			{
				bm->SetPixel(x, y, Color::Black);
			}
		}

		picCanvas->BackgroundImage = bm;

	}



	private: System::Void  ShowMyImage(String^ fileToDisplay, int xSize, int ySize)
	{
		Bitmap^ MyImage;
		// Sets up an image object to be displayed.
		if (MyImage != nullptr)
		{
			delete MyImage;
		}


		// Stretches the image to fit the pictureBox.
		this->picCanvas->SizeMode = PictureBoxSizeMode::StretchImage;
		MyImage = gcnew Bitmap(fileToDisplay);
		this->picCanvas->ClientSize = System::Drawing::Size(xSize, ySize);
		this->picCanvas->Image = dynamic_cast<Image^>(MyImage);
	}

	private: System::Void  ShowMyImage(Stream^ fileToDisplay, int xSize, int ySize)
	{
		Bitmap^ MyImage;
		// Sets up an image object to be displayed.
		if (MyImage != nullptr)
		{
			delete MyImage;
		}


		// Stretches the image to fit the pictureBox.
		this->picCanvas->SizeMode = PictureBoxSizeMode::StretchImage;
		MyImage = gcnew Bitmap(fileToDisplay);
		this->picCanvas->ClientSize = System::Drawing::Size(xSize, ySize);
		this->picCanvas->Image = dynamic_cast<Image^>(MyImage);
	}





	};
}
