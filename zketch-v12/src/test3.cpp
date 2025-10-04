#include "zketch.hpp"
using namespace zketch ;

Rect Client ;

void UpdateClient(const Window& window) noexcept {
	Client = window.GetClientBound() ;
}

class Painter {
protected :
	static inline std::unique_ptr<Canvas> canvas_ ;
	static inline std::unique_ptr<Drawer> drawer_ ;
	static inline bool update_ = false ;

	Painter() noexcept {
		if (!canvas_) {
			canvas_ = std::make_unique<Canvas>() ;
		}
		canvas_->Create(Client.GetSize()) ;
		if (!drawer_) {
			drawer_ = std::make_unique<Drawer>() ;
		}
	}
} ;

class Background : public Painter {
private :
	Rect rect_ ;
	Color color_ ;

protected :
	Background(const Color& color) noexcept : color_(color), Painter() {}

public :
	void InvokeUpdate() noexcept {
		if (update_) {
			if (drawer_->Begin(*canvas_)) {
				drawer_->Clear(color_) ;
			}
		}
	}

	void SetColor(const Color& color) noexcept {
		color_ = color ;
		update_ = true ;
	}

	void SetSize(const Size& size) noexcept {
		canvas_->Create(size) ;
		update_ = true ;
	}
} ;

class Dot : public Painter {
private :
	Color color_ ;
	RectF rect_ ;

protected : 
	Dot(const RectF& rect) noexcept : Painter(), rect_(rect) {}

public :
	void InvokeUpdate() noexcept {
		if (update_) {
			drawer_->FillCircle(rect_.Anchor(Anchor::Center), rect_.w / 2, color_) ;
			drawer_->DrawRect(0, White, 10) ;
			drawer_->End() ;
			update_ = false ;
		}
	}

	void SetColor(const Color& color) noexcept {
		color_ = color ;
		update_ = true ;
	}

	void SetPosition(const PointF& pos) noexcept {
		rect_.SetPos(pos) ;
		update_ = true ;
	}

	void Translate(const PointF& value) noexcept {
		rect_.SetPos(rect_.GetPos() + value) ;
		update_ = true ;
	}

	const RectF& GetRect() const noexcept {
		return rect_ ;
	}
} ;

class Game : public Background, public Dot {
public : 
	Game(const Color& bg, const RectF& dot) noexcept : Background(bg), Dot(dot) {}

	void Present(Window& window) noexcept {
		Background::InvokeUpdate() ;
		Dot::InvokeUpdate() ;
		window.Present(*canvas_) ;
	}

	void Update() noexcept {
		update_ = true ;
	}
} ;

int main() {
	zketch_init() ;

	Window window("Draw Demo", 900, 800) ;
	window.Show() ;

	UpdateClient(window) ;
	Game g(Black, {PointF{}, SizeF{50, 50}}) ;
	g.Dot::SetColor(Red) ;

	Event e ;
	InputSystem in ;
	bool is_dragging = false ;

	while(Application) {
		while (PollEvent(e)) {
			switch (e) {
				case EventType::Key :
					if (e.GetKeyState() == KeyState::Down) {
						if (e.GetKeyCode() == KeyCode::ArrowUp || e.GetKeyCode() == KeyCode::W) {
							in.SetKeyUp(e.GetKeyCode()) ;
							g.Dot::Translate({0, -10}) ;
						}
						if (e.GetKeyCode() == KeyCode::ArrowDown || e.GetKeyCode() == KeyCode::S) {
							g.Dot::Translate({0, 10}) ;
						} 
						if (e.GetKeyCode() == KeyCode::ArrowLeft || e.GetKeyCode() == KeyCode::A) {
							g.Dot::Translate({-10, 0}) ;
						}
						if (e.GetKeyCode() == KeyCode::ArrowRight || e.GetKeyCode() == KeyCode::D) {
							g.Dot::Translate({10, 0}) ;
						}
					}
					break ;

				case EventType::Mouse :
					if (e.GetMouseState() == MouseState::Down && e.GetMouseButton() == MouseButton::Left) {
						if (g.GetRect().Contain(e.GetMousePosition())) {
							is_dragging = true ;
						}
					} else if (e.GetMouseState() == MouseState::None) {
						if (is_dragging) {
							g.Dot::SetPosition(e.GetMousePosition() - (g.GetRect().GetSize() / 2)) ;
						}
					} else if (e.GetMouseState() == MouseState::Up) {
						if (is_dragging) {
							is_dragging = false ;
						}
					}

				case EventType::Resize :
					UpdateClient(window) ;
					g.SetSize(Client.GetSize()) ;
					g.Update() ;
					break ;

				default : break ;
			}
		}

		g.Present(window) ;
		Sleep(16) ;
	}
}