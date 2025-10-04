#include "zketch.hpp"
using namespace zketch ;

RectF Client ;
Drawer Draw ;

void UpdateClient(const Window& window) noexcept {
	Client = window.GetClientBound() ;
}

class Background {
private :
	std::unique_ptr<Canvas> canvas_ ;
	bool update_ ;

	void InvokeRedraw() noexcept {
		if (update_) {
			canvas_->Create(Client.GetSize()) ;
			if (!Draw.Begin(*canvas_)) {
				return ;
			}

			Draw.End() ;
			update_ = false ;
		}
	}

public :
	Background(const Color& color = Black) noexcept : canvas_(std::make_unique<Canvas>()), update_(true) {
		canvas_->Create(Client.GetSize() - Point{10, 0}) ;
		canvas_->SetClearColor(color) ;
	}

	~Background() noexcept = default ;

	void Update() noexcept {
		update_ = true ;
	}

	void Present(Window& window) noexcept {
		InvokeRedraw() ;
		window.Present(*canvas_) ;
	}

	Canvas* GetCanvas() noexcept {
		InvokeRedraw() ;
		return canvas_.get() ;
	}
} ;

void InvokeUpdateVSlider(Canvas* canvas, const Slider& slider) noexcept {
	if (!Draw.Begin(*canvas)) {
		return ;
	}

	if (slider.IsDragging()) {
		Draw.FillRect(slider.GetRelativeThumbBound(), rgba(128, 128, 128, 1)) ;
	} else if (slider.IsHovered()) {
		Draw.FillRect(slider.GetRelativeThumbBound(), rgba(128, 128, 128, 1)) ;
	} else if (slider.IsVisible()) {
		Draw.FillRect(slider.GetRelativeThumbBound(), rgba(72, 72, 72, 1)) ;
	}
	Draw.End() ;
}

struct VSlider {
	std::unique_ptr<Slider> slider_ ;

	VSlider(const Color& color = Black) noexcept : 
	slider_(std::make_unique<Slider>(Slider::Vertical, RectF{Client.Anchor(Anchor::RightTop) - Point{10, 0}, Size{10, Client.h}}, 
	Size{10, 40})) {
		slider_->SetClearColor(color) ;
		slider_->SetDrawer(InvokeUpdateVSlider) ;
	}

	void SetPosition(const PointF& pos) noexcept {
		slider_->SetPosition(pos) ;
	}

	void Update() noexcept {
		slider_->MarkDirty() ;
		slider_->SetPosition(Client.Anchor(Anchor::RightTop) - Point{10, 0}) ;
	}

	void Present(Window& window) noexcept {
		slider_->Present(window) ;
	}

	Canvas* GetCanvas() noexcept {
		return slider_->GetCanvas() ;
	}
} ;

int main() {
	zketch_init() ;	

	Window window("Slider Demo", 800, 600) ;
	window.Show() ;

	UpdateClient(window) ;
	Background bg ;
	VSlider vslide ;

	Event e ;

	while (Application) {
		while (PollEvent(e)) {
			switch (e) {
				case EventType::Mouse :
					if (e.GetMouseState() == MouseState::Down) {
						if (e.GetMouseButton() == MouseButton::Left) {
							vslide.slider_->OnPress(e.GetMousePosition()) ;
						}
					} else if (e.GetMouseState() == MouseState::Up) {
						if (e.GetMouseButton() == MouseButton::Left) {
							vslide.slider_->OnRelease() ;
						}
					} else if (e.GetMouseState() == MouseState::None) {
						vslide.slider_->OnHover(e.GetMousePosition()) ;
						if (vslide.slider_->IsDragging()) {
							vslide.slider_->OnDrag(e.GetMousePosition()) ;
						}
					}
				break ;

				case EventType::Resize :
					UpdateClient(window) ;
					bg.Update() ;
					vslide.Update() ;
					break ;
			}
		}

		window.Present(*bg.GetCanvas(), *vslide.GetCanvas()) ;
		Sleep(16) ;
	}
}