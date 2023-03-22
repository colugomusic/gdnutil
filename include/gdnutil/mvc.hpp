#pragma once

#include <array>
#include <memory>
#include <vector>

namespace gdn {

template <typename KeyType>
struct DrawInfo
{
	static_assert (std::is_enum_v<KeyType>);

	auto operator[](KeyType k) const -> bool { return values[k]; }
	auto set_all() { for(int k = 0; k < KeyType::N; k++) { values[k] = true; } };
	auto set(KeyType k) { values[k] = true; };
	
private:

	std::array<bool, KeyType::N> values;
};

template <typename ControllerType, typename ViewType>
class Controller;

template <typename Model, typename ViewType, typename ControllerType, typename DrawFlags>
class MVC;

template <typename ControllerType>
class View
{
protected:
	ControllerType* controller_{};

	template <typename ControllerType, typename ViewType>
	friend class Controller;

	template <typename Model, typename ViewType, typename ControllerType, typename DrawFlags>
	friend class MVC;
};

template <typename ControllerType, typename ViewType>
class Controller
{
public:
	Controller(ViewType* view)
		: view_{view}
	{}

	~Controller() {
		if (view_->controller_ == static_cast<ControllerType*>(this)) {
			view_->controller_ = nullptr;
		}
	}

	auto get_view() const -> ViewType* {
		return view_;
	}

protected:
	ViewType* view_{};
};

template <typename Model, typename ViewType, typename ControllerType, typename DrawFlags>
class MVC
{
public:
	static_assert (std::is_enum_v<DrawFlags>);

	using draw_info_t = DrawInfo<DrawFlags>;

	MVC() = default;
	MVC(ViewType* view, ControllerType* controller)
		: view_{view}
		, controller_{controller}
	{
		view_->controller_ = controller;
		draw_info_.set_all();
	}

	~MVC()
	{
		if (view_->controller_ == controller_) {
			view_->controller_ = nullptr;
		}
	}

	auto draw() -> void
	{
		view_->draw(model_, draw_info_);
		viewed_model_ = model_;
		draw_info_ = {};
	}

	auto set_draw_info(DrawFlags flag) -> void
	{
		draw_info_.set(flag);
		view_->update();
	}

	template <typename M>
	auto set_model(M&& model) -> void
	{
		model_ = std::move(model);
		draw_info_.set_all();
	}

	template <typename Updater>
    auto update_model(Updater&& updater) -> void
	{
		model_ = updater(std::move(model_));
		view_->update();
	}

	auto& get_model() const { return model_; }
	auto get_view() const { return view_; }

private:

	Model model_;
	Model viewed_model_{};
	ViewType* view_;
	ControllerType* controller_;
	draw_info_t draw_info_{};
};

} // gdn