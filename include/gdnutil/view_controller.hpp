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

template <typename Model, typename View, typename Controller, typename DrawFlags>
class ViewController
{
public:
	static_assert (std::is_enum_v<DrawFlags>);

	using draw_info_t = DrawInfo<DrawFlags>;

	ViewController() = default;
	ViewController(View* view, Controller* controller)
		: view_{view}
	{
		view_->set_controller(controller);
		draw_info_.set_all();
	}

	~ViewController()
	{
		view_->set_controller(nullptr);
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
	View* view_{};
	draw_info_t draw_info_{};
};

} // gdn